/* Flattning functions for xcftools
 *
 * Copyright (C) 2006  Henning Makholm
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xcftools.h"
#include "flatten.h"
#include "pixels.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static rgba __ATTRIBUTE__((noinline,const))
composite_one(rgba bot,rgba top)
{
  unsigned tfrac, alpha ;

  tfrac = ALPHA(top) ;
  alpha = 255 ;
  if( !FULLALPHA(bot) ) {
    alpha = 255 ^ scaletable[255-ALPHA(bot)][255-ALPHA(top)] ;
    /* This peculiar combination of ^ and - makes the GCC code
     * generator for i386 particularly happy.
     */
    tfrac = (256*ALPHA(top) - 1) / alpha ;
    /* Tfrac is the fraction of the coposited pixel's covered area
     * that comes from the top pixel.
     * For mathematical accuracy we ought to scale by 255 and
     * subtract alpha/2, but this is faster, and never misses the
     * true value by more than one 1/255. This effect is completely
     * overshadowed by the linear interpolation in the first place.
     * (I.e. gamma is ignored when combining intensities).
     *   [In any case, complete fairness is not possible: if the
     *    bottom pixel had alpha=170 and the top has alpha=102,
     *    each should contribute equally to the color of the
     *    resulting alpha=204 pixel, which is not possible in general]
     * Subtracting one helps the topfrac never be 256, which would
     * be bad.
     * On the other hand it means that we would get tfrac=-1 if the
     * top pixel is completely transparent, and we get a division
     * by zero if _both_ pixels are fully transparent. These cases
     * must be handled by all callers.
     */
  }
  return (alpha << ALPHA_SHIFT)
    + ((uint32_t)scaletable[  tfrac  ][255&(top>>RED_SHIFT  )] << RED_SHIFT   )
    + ((uint32_t)scaletable[  tfrac  ][255&(top>>GREEN_SHIFT)] << GREEN_SHIFT )
    + ((uint32_t)scaletable[  tfrac  ][255&(top>>BLUE_SHIFT )] << BLUE_SHIFT  )
    + ((uint32_t)scaletable[255^tfrac][255&(bot>>RED_SHIFT  )] << RED_SHIFT   )
    + ((uint32_t)scaletable[255^tfrac][255&(bot>>GREEN_SHIFT)] << GREEN_SHIFT )
    + ((uint32_t)scaletable[255^tfrac][255&(bot>>BLUE_SHIFT )] << BLUE_SHIFT  )
    ;
}

/* merge_normal() takes ownership of bot.
 * merge_normal() will share ownership of top.
 * Return: may be shared.
 */
static struct Tile * __ATTRIBUTE__((noinline))
merge_normal(struct Tile *bot, struct Tile *top)
{
  unsigned i ;
  assertTileCompatibility(bot,top);

  /* See if there is an easy winner */
  if( (bot->summary & TILESUMMARY_ALLNULL) ||
      (top->summary & TILESUMMARY_ALLFULL) ) {
    freeTile(bot);
    return top ;
  }
  if( top->summary & TILESUMMARY_ALLNULL ) {
    freeTile(top);
    return bot ;
  }
  
  /* Try hard to make top win */
  for( i=0; ; i++ ) {
    if( i == top->count ) {
      freeTile(bot);
      return top ;
    }
    if( !(NULLALPHA(bot->pixels[i]) || FULLALPHA(top->pixels[i])) )
      break ;
  }

  /* Otherwise bot wins, but is forever changed ... */
  if( (top->summary & TILESUMMARY_ALLNULL) == 0 ) {
    unsigned i ;
    invalidateSummary(bot,0);
    for( i=0 ; i < top->count ; i++ ) {
      if( !NULLALPHA(top->pixels[i]) ) {
        if( FULLALPHA(top->pixels[i]) || NULLALPHA(bot->pixels[i]) )
          bot->pixels[i] = top->pixels[i] ;
        else
          bot->pixels[i] = composite_one(bot->pixels[i],top->pixels[i]);
      }
    }
  }
  freeTile(top);
  return bot ;
}

#define exotic_combinator static inline unsigned __ATTRIBUTE__((const))



exotic_combinator
ucombine_ADDITION(uint8_t bot,uint8_t top)
{
  return bot+top > 255 ? 255 : bot+top ;
}

exotic_combinator
ucombine_SUBTRACT(uint8_t bot,uint8_t top)
{
  return top>bot ? 0 : bot-top ;
}

exotic_combinator
ucombine_LIGHTEN_ONLY(uint8_t bot,uint8_t top)
{
  return top > bot ? top : bot ;
}

exotic_combinator
ucombine_DARKEN_ONLY(uint8_t bot,uint8_t top)
{
  return top < bot ? top : bot ;
}

exotic_combinator
ucombine_DIFFERENCE(uint8_t bot,uint8_t top)
{
  return top > bot ? top-bot : bot-top ;
}

exotic_combinator
ucombine_MULTIPLY(uint8_t bot,uint8_t top)
{
  return scaletable[bot][top] ;
}

exotic_combinator
ucombine_DIVIDE(uint8_t bot,uint8_t top)
{
  int result = (int)bot*256 / (1+top) ;
  return result >= 256 ? 255 : result ;
}

exotic_combinator
ucombine_SCREEN(uint8_t bot,uint8_t top)
{
  /* An inverted version of "multiply" */
  return 255 ^ scaletable[255-bot][255-top] ;
}

exotic_combinator
ucombine_OVERLAY(uint8_t bot,uint8_t top)
{
  return scaletable[bot][bot] +
    2*scaletable[top][scaletable[bot][255-bot]] ;
  /* This strange formula is equivalent to
   *   (1-top)*(bot^2) + top*(1-(1-top)^2)
   * that is, the top value is used to interpolate between
   * the self-multiply and the self-screen of the bottom.
   */
  /* Note: This is exactly what the "Soft light" effect also
   * does, though with different code in the Gimp.
   */
}

exotic_combinator
ucombine_DODGE(uint8_t bot,uint8_t top)
{
  return ucombine_DIVIDE(bot,255-top);
}

exotic_combinator
ucombine_BURN(uint8_t bot,uint8_t top)
{
  return 255 - ucombine_DIVIDE(255-bot,top);
}

exotic_combinator
ucombine_HARDLIGHT(uint8_t bot,uint8_t top)
{
  if( top >= 128 )
    return 255 ^ scaletable[255-bot][2*(255-top)] ;
  else
    return scaletable[bot][2*top];
  /* The code that implements "hardlight" in Gimp 2.2.10 has some
   * rounding errors, but this is undoubtedly what is meant.
   */
}

exotic_combinator
ucombine_GRAIN_EXTRACT(uint8_t bot,uint8_t top)
{
  int temp = (int)bot - (int)top + 128 ;
  return temp < 0 ? 0 : temp >= 256 ? 255 : temp ;
}

exotic_combinator
ucombine_GRAIN_MERGE(uint8_t bot,uint8_t top)
{
  int temp = (int)bot + (int)top - 128 ;
  return temp < 0 ? 0 : temp >= 256 ? 255 : temp ;
}



/* merge_exotic() destructively updates bot.
 * merge_exotoc() reads but does not free top.
 */
static void __ATTRIBUTE__((noinline))
merge_exotic(struct Tile *bot, const struct Tile *top,
             GimpLayerModeEffects mode)
{
  unsigned i ;
  assertTileCompatibility(bot,top);
  if( (bot->summary & TILESUMMARY_ALLNULL) != 0 ) return ;
  if( (top->summary & TILESUMMARY_ALLNULL) != 0 ) return ;
  assert( bot->refcount == 1 );
  /* The transparency status of bot never changes */

  for( i=0; i < top->count ; i++ ) {
    uint32_t red, green, blue ;
    if( NULLALPHA(bot->pixels[i]) || NULLALPHA(top->pixels[i]) )
      continue ;
#define UNIFORM(mode) case GIMP_ ## mode ## _MODE: \
      red   = ucombine_ ## mode (bot->pixels[i]>>RED_SHIFT  ,  \
                                 top->pixels[i]>>RED_SHIFT  ); \
      green = ucombine_ ## mode (bot->pixels[i]>>GREEN_SHIFT,  \
                                 top->pixels[i]>>GREEN_SHIFT); \
      blue  = ucombine_ ## mode (bot->pixels[i]>>BLUE_SHIFT ,  \
                                 top->pixels[i]>>BLUE_SHIFT ); \
      break ;
    switch( mode ) {
    case GIMP_NORMAL_MODE:
    case GIMP_DISSOLVE_MODE:
      FatalUnexpected("Normal and Dissolve mode can't happen here!");
      UNIFORM(ADDITION);
      UNIFORM(SUBTRACT);
      UNIFORM(LIGHTEN_ONLY);
      UNIFORM(DARKEN_ONLY);
      UNIFORM(DIFFERENCE);
      UNIFORM(MULTIPLY);
      UNIFORM(DIVIDE);
      UNIFORM(SCREEN);
    case GIMP_SOFTLIGHT_MODE: /* A synonym for "overlay"! */
      UNIFORM(OVERLAY);
      UNIFORM(DODGE);
      UNIFORM(BURN);
      UNIFORM(HARDLIGHT);
      UNIFORM(GRAIN_EXTRACT);
      UNIFORM(GRAIN_MERGE);
    default:
      FatalUnsupportedXCF(_("'%s' layer mode"),showGimpLayerModeEffects(mode));
    }
    if( FULLALPHA(bot->pixels[i] & top->pixels[i]) )
      bot->pixels[i] = (bot->pixels[i] & (255 << ALPHA_SHIFT)) +
        (red << RED_SHIFT) +
        (green << GREEN_SHIFT) +
        (blue << BLUE_SHIFT) ;
    else {
      rgba bp = bot->pixels[i] ;
      /* In a sane world, the alpha of the top pixel would simply be
       * used to interpolate linearly between the bottom pixel's base
       * color and the effect-computed color.
       * But no! What the Gimp actually does is empirically
       * described by the following (which borrows code from
       * composite_one() that makes no theoretical sense here):
       */
      unsigned tfrac = ALPHA(top->pixels[i]) ;
      if( !FULLALPHA(bp) ) {
        unsigned pseudotop = (tfrac < ALPHA(bp) ? tfrac : ALPHA(bp));
        unsigned alpha = 255 ^ scaletable[255-ALPHA(bp)][255-pseudotop] ;
        tfrac = (256*pseudotop - 1) / alpha ;
      }
      bot->pixels[i] = (bp & (255 << ALPHA_SHIFT)) +
        ((rgba)scaletable[  tfrac  ][  red                ] << RED_SHIFT  ) +
        ((rgba)scaletable[  tfrac  ][  green              ] << GREEN_SHIFT) +
        ((rgba)scaletable[  tfrac  ][  blue               ] << BLUE_SHIFT ) +
        ((rgba)scaletable[255^tfrac][255&(bp>>RED_SHIFT  )] << RED_SHIFT  ) +
        ((rgba)scaletable[255^tfrac][255&(bp>>GREEN_SHIFT)] << GREEN_SHIFT) +
        ((rgba)scaletable[255^tfrac][255&(bp>>BLUE_SHIFT )] << BLUE_SHIFT ) ;
    }
  }
  return ;
}      
     
static void
dissolveTile(struct Tile *tile)
{
  unsigned i ;
  summary_t summary ;
  assert( tile->refcount == 1 );
  if( (tile->summary & TILESUMMARY_CRISP) )
    return ;
  summary = TILESUMMARY_UPTODATE + TILESUMMARY_ALLNULL
    + TILESUMMARY_ALLFULL + TILESUMMARY_CRISP ;
  for( i = 0 ; i < tile->count ; i++ ) {
    if( FULLALPHA(tile->pixels[i]) )
      summary &= ~TILESUMMARY_ALLNULL ;
    else if ( NULLALPHA(tile->pixels[i]) )
      summary &= ~TILESUMMARY_ALLFULL ;
    else if( ALPHA(tile->pixels[i]) > rand() % 0xFF ) {
      tile->pixels[i] |= 255 << ALPHA_SHIFT ;
      summary &= ~TILESUMMARY_ALLNULL ;
    } else {
      tile->pixels[i] = 0 ;
      summary &= ~TILESUMMARY_ALLFULL ;
    }
  }
  tile->summary = summary ;
}
     
static void
roundAlpha(struct Tile *tile)
{
  unsigned i ;
  summary_t summary ;
  assert( tile->refcount == 1 );
  if( (tile->summary & TILESUMMARY_CRISP) )
    return ;
  summary = TILESUMMARY_UPTODATE + TILESUMMARY_ALLNULL
    + TILESUMMARY_ALLFULL + TILESUMMARY_CRISP ;
  for( i = 0 ; i < tile->count ; i++ ) {
    if( ALPHA(tile->pixels[i]) >= 128 ) {
      tile->pixels[i] |= 255 << ALPHA_SHIFT ;
      summary &= ~TILESUMMARY_ALLNULL ;
    } else {
      tile->pixels[i] = 0 ;
      summary &= ~TILESUMMARY_ALLFULL ;
    }
  }
  tile->summary = summary ;
}

/* flattenTopdown() shares ownership of top.
 * The return value may be a shared tile.
 */
static struct Tile *
flattenTopdown(struct FlattenSpec *spec, struct Tile *top,
               unsigned nlayers, const struct rect *where)
{
  struct Tile *tile;

  while( nlayers-- ) {
    if( tileSummary(top) & TILESUMMARY_ALLFULL )
      return top ;
    if( !spec->layers[nlayers].isVisible )
      continue ;
    
    tile = getLayerTile(&spec->layers[nlayers],where);
    
    if( tile->summary & TILESUMMARY_ALLNULL )
      continue ; /* Simulate a tail call */

    switch( spec->layers[nlayers].mode ) {
    case GIMP_NORMAL_NOPARTIAL_MODE:
      roundAlpha(tile) ;
      /* fall through */
      if(0) {
      case GIMP_DISSOLVE_MODE:
        dissolveTile(tile);
        /* fall through */
      }
    case GIMP_NORMAL_MODE:
      top = merge_normal(tile,top);
      break ;
    default:
      {
        struct Tile *below, *above ;
        unsigned i ;
        if( !(top->summary & TILESUMMARY_ALLNULL) ) {
          rgba tile_or = 0 ;
          invalidateSummary(tile,0);
          for( i=0; i<top->count; i++ )
            if( FULLALPHA(top->pixels[i]) )
              tile->pixels[i] = 0 ;
            else
              tile_or |= tile->pixels[i] ;
          /* If the tile only has pixels that will be covered by 'top' anyway,
           * forget it anyway.
           */
          if( ALPHA(tile_or) == 0 ) {
            freeTile(tile);
            break ; /* from the switch, which will continue the while */
          }
        }
        /* Create a dummy top for the layers below this */
        if( top->summary & TILESUMMARY_CRISP ) {
          above = forkTile(top);
        } else {
          summary_t summary = TILESUMMARY_ALLNULL ;
          above = newTile(*where);
          for( i=0; i<top->count; i++ )
            if( FULLALPHA(top->pixels[i]) ) {
              above->pixels[i] = -1 ;
              summary = 0 ;
            } else
              above->pixels[i] = 0 ;
          above->summary = TILESUMMARY_UPTODATE + TILESUMMARY_CRISP + summary;
        }
        below = flattenTopdown(spec, above, nlayers, where);
        if( below->refcount > 1 ) {
          assert( below == top );
          /* This can only happen if 'below' is a copy of 'top'
           * THROUGH 'above', which in turn means that none of all
           * this is visible after all. So just free it and return 'top'.
           */
          freeTile(below);
          return top ;
        }
        merge_exotic(below,tile,spec->layers[nlayers].mode);
        freeTile(tile);
        top = merge_normal(below,top);
        return top ;
      }
    }
  }
  return top ;
}

static void
addBackground(struct FlattenSpec *spec, struct Tile *tile)
{
  unsigned i ;

  if( tileSummary(tile) & TILESUMMARY_ALLFULL )
    return ;

  switch( spec->partial_transparency_mode ) {
  case FORBID_PARTIAL_TRANSPARENCY:
    if( !(tileSummary(tile) & TILESUMMARY_CRISP) )
      FatalGeneric(102,_("Flattened image has partially transparent pixels"));
    break ;
  case DISSOLVE_PARTIAL_TRANSPARENCY:
    dissolveTile(tile);
    break ;
  case ALLOW_PARTIAL_TRANSPARENCY:
  case PARTIAL_TRANSPARENCY_IMPOSSIBLE:
    break ;
  }

  if( !FULLALPHA(spec->default_pixel) )  return ;
  if( tileSummary(tile) & TILESUMMARY_ALLNULL ) {
    fillTile(tile,spec->default_pixel);
  } else {
    for( i=0; i<tile->count; i++ )
      if( NULLALPHA(tile->pixels[i]) )
        tile->pixels[i] = spec->default_pixel ;
      else if( FULLALPHA(tile->pixels[i]) )
        ;
      else
        tile->pixels[i] = composite_one(spec->default_pixel,tile->pixels[i]);
    
    tile->summary = TILESUMMARY_UPTODATE +
      TILESUMMARY_ALLFULL + TILESUMMARY_CRISP ;
  }
}

void
flattenIncrementally(struct FlattenSpec *spec,lineCallback callback)
{
  rgba *rows[TILE_HEIGHT] ;
  unsigned i, y, nrows, ncols ;
  struct rect where ;
  struct Tile *tile ;
  static struct Tile toptile ;

  toptile.count = TILE_HEIGHT * TILE_WIDTH ;
  fillTile(&toptile,0);

  for( where.t = spec->dim.c.t; where.t < spec->dim.c.b; where.t=where.b ) {
    where.b = (where.t+TILE_HEIGHT) - where.t % TILE_HEIGHT ;
    if( where.b > spec->dim.c.b ) where.b = spec->dim.c.b ;
    nrows = where.b - where.t ;
    for( y = 0; y < nrows ; y++ )
      rows[y] = xcfmalloc(4*(spec->dim.c.r-spec->dim.c.l));

    for( where.l = spec->dim.c.l; where.l < spec->dim.c.r; where.l=where.r ) {
      where.r = (where.l+TILE_WIDTH) - where.l % TILE_WIDTH ;
      if( where.r > spec->dim.c.r ) where.r = spec->dim.c.r ;
      ncols = where.r - where.l ;

      toptile.count = ncols * nrows ;
      toptile.refcount = 2 ; /* For bug checking */
      assert( toptile.summary == TILESUMMARY_UPTODATE +
              TILESUMMARY_ALLNULL + TILESUMMARY_CRISP );
      tile = flattenTopdown(spec,&toptile,spec->numLayers,&where) ;
      toptile.refcount-- ; /* addBackground may change destructively */
      addBackground(spec,tile);

      for( i = 0 ; i < tile->count ; i++ )
        if( NULLALPHA(tile->pixels[i]) )
          tile->pixels[i] = 0 ;
      for( y = 0 ; y < nrows ; y++ )
        memcpy(rows[y] + (where.l - spec->dim.c.l),
               tile->pixels + y * ncols, ncols*4);
      
      if( tile == &toptile ) {
        fillTile(&toptile,0);
      } else {
        freeTile(tile);
      }
    }
    for( y = 0 ; y < nrows ; y++ )
      callback(spec->dim.width,rows[y]);
  }
}

static rgba **collectPointer ;

static void
collector(unsigned num,rgba *row)
{
  *collectPointer++ = row ;
}

rgba **
flattenAll(struct FlattenSpec *spec)
{
  rgba **rows = xcfmalloc(spec->dim.height * sizeof(rgba*));
  if( verboseFlag )
    fprintf(stderr,_("Flattening image ..."));
  collectPointer = rows ;
  flattenIncrementally(spec,collector);
  if( verboseFlag )
    fprintf(stderr,"\n");
  return rows ;
}

void
shipoutWithCallback(struct FlattenSpec *spec, rgba **pixels,
                    lineCallback callback)
{
  unsigned i ;
  for( i = 0; i < spec->dim.height; i++ ) {
    callback(spec->dim.width,pixels[i]);
  }
  xcffree(pixels);
}
