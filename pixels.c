/*  Pixel and tile functions for xcftools
 *
 * This file was written by Henning Makholm <henning@makholm.net>
 * It is hereby in the public domain.
 * 
 * In jurisdictions that do not recognise grants of copyright to the
 * public domain: I, the author and (presumably, in those jurisdictions)
 * copyright holder, hereby permit anyone to distribute and use this code,
 * in source code or binary form, with or without modifications. This
 * permission is world-wide and irrevocable.
 *
 * Of course, I will not be liable for any errors or shortcomings in the
 * code, since I give it away without asking any compenstations.
 *
 * If you use or distribute this code, I would appreciate receiving
 * credit for writing it, in whichever way you find proper and customary.
 */

#define DEBUG
#include "xcftools.h"
#include "pixels.h"
#include <assert.h>
#include <string.h>

rgba colormap[256] ;
unsigned colormapLength=0 ;

int
degrayPixel(rgba pixel)
{
  if( ((pixel >> RED_SHIFT) & 255) == ((pixel >> GREEN_SHIFT) & 255) &&
      ((pixel >> RED_SHIFT) & 255) == ((pixel >> BLUE_SHIFT) & 255) )
    return (pixel >> RED_SHIFT) & 255 ;
  return -1 ;
}

/* ****************************************************************** */

typedef const struct _convertParams {
  int bpp ;
  int shift[4] ;
  uint32_t base_pixel ;
  const rgba *lookup ;
} convertParams ;
#define RGB_SHIFT RED_SHIFT, GREEN_SHIFT, BLUE_SHIFT
#define OPAQUE (255 << ALPHA_SHIFT)
static convertParams convertRGB       = { 3, {RGB_SHIFT}, OPAQUE, 0 };
static convertParams convertRGBA      = { 4, {RGB_SHIFT, ALPHA_SHIFT}, 0,0 };
static convertParams convertGRAY      = { 1, {-1}, OPAQUE, graytable };
static convertParams convertGRAYA     = { 2, {-1,ALPHA_SHIFT}, 0, graytable };
static convertParams convertINDEXED   = { 1, {-1}, OPAQUE, colormap };
static convertParams convertINDEXEDA  = { 2, {-1,ALPHA_SHIFT}, 0, colormap };

static convertParams convertColormap  = { 3, {RGB_SHIFT}, 0, 0 };
static convertParams convertChannel   = { 1, {ALPHA_SHIFT}, 0, 0 };

/* ****************************************************************** */

static inline xcfptr_t
tileDirectoryOneLevel(struct tileDimensions *dim,xcfptr_t ptr)
{
  if( ptr == 0 )
    return 0 ;
  if( xcfL(ptr  ) != dim->c.r - dim->c.l ||
      xcfL(ptr+4) != dim->c.b - dim->c.t )
    FatalBadXCF("Drawable size mismatch at %" PRIXPTR, ptr);
  return ptr += 8 ;
}

static void
initTileDirectory(struct tileDimensions *dim,struct xcfTiles *tiles,
                  const char *type)
{
  xcfptr_t ptr ;
  xcfptr_t data ;

  ptr = tiles->hierarchy ;
  tiles->hierarchy = 0 ;
  if( (ptr = tileDirectoryOneLevel(dim,ptr)) == 0 ) return ;
  if( tiles->params == &convertChannel ) {
    /* A layer mask is a channel.
     * Skip a name and a property list.
     */
    xcfString(ptr,&ptr);
    while( xcfNextprop(&ptr,&data) != PROP_END )
      ;
    ptr = xcfOffset(ptr,4*4);
    if( (ptr = tileDirectoryOneLevel(dim,ptr)) == 0 ) return ;
  }
  /* The XCF format has a dummy "hierarchy" level which was
   * once meant to mean something, but never happened. It contains
   * the bpp value and a list of "level" pointers; but only the
   * first level actually contains data.
   */
  if( xcfL(ptr) != tiles->params->bpp )
    FatalBadXCF("%"PRIu32" bytes per pixel for %s drawable",xcfL(ptr),type);
  ptr = xcfOffset(ptr+4,3*4) ;
  if( (ptr = tileDirectoryOneLevel(dim,ptr)) == 0 ) return ;

  xcfCheckspace(ptr,dim->ntiles*4+4,"Tile directory at %" PRIXPTR,ptr);
  if( xcfL(ptr + dim->ntiles*4) != 0 )
    FatalBadXCF("Wrong sized tile directory at %" PRIXPTR,ptr);
# if defined(WORDS_BIGENDIAN)
  if( (xcfPsz == sizeof(xcfptr_t))
# if ! defined(CAN_DO_UNALIGNED_WORDS)
      && ((ptr&(sizeof(xcfptr_t)-1)) == 0)
# endif
      )
    tiles->tileptrs = (xcfptr_t*)(xcf_file + ptr);
  else
# endif
    {
      unsigned i ;
      tiles->tileptrs = xcfmalloc(dim->ntiles * sizeof(xcfptr_t)) ;
      for( i = 0 ; i < dim->ntiles ; i++ )
        tiles->tileptrs[i] = xcfP(ptr+i*xcfPsz);
    }
}

void
initLayer(struct xcfLayer *layer) {
  if( layer->dim.ntiles == 0 ||
      (layer->pixels.hierarchy == 0 && layer->mask.hierarchy == 0) )
    return ;
  switch(layer->type) {
#define DEF(X) case GIMP_##X##_IMAGE: layer->pixels.params = &convert##X; break
    DEF(RGB);
    DEF(RGBA);
    DEF(GRAY);
    DEF(GRAYA);
    DEF(INDEXED);
    DEF(INDEXEDA);
  default:
    FatalUnsupportedXCF(_("Layer type %s"),_(showGimpImageType(layer->type)));
  }
  initTileDirectory(&layer->dim,&layer->pixels,
                    _(showGimpImageType(layer->type)));
  layer->mask.params = &convertChannel ;
  initTileDirectory(&layer->dim,&layer->mask,"layer mask");
}
static void copyStraightPixels(rgba *dest,unsigned npixels,
                               xcfptr_t ptr,convertParams *params);
void
initColormap(void) {
  uint32_t ncolors ;
  if( XCF.colormapptr == 0 ) {
    colormapLength = 0 ;
    return ;
  }
  ncolors = xcfL(XCF.colormapptr) ;
  if( ncolors > 256 )
    FatalUnsupportedXCF(_("Color map has more than 256 entries"));
  copyStraightPixels(colormap,ncolors,XCF.colormapptr+4,&convertColormap);
  colormapLength = ncolors ;
#ifdef xDEBUG
  {
    unsigned j ;
    fprintf(stderr,"Colormap decoding OK\n");
    for( j = 0 ; j < ncolors ; j++ ) {
      if( j % 8 == 0 ) fprintf(stderr,"\n");
      fprintf(stderr," %08x",colormap[j]);
    }
    fprintf(stderr,"\n");
  }
#endif
}

/* ****************************************************************** */

struct Tile *
newTile(struct rect r)
{
  unsigned npixels = (unsigned)(r.b-r.t) * (unsigned)(r.r-r.l) ;
  struct Tile *data
    = xcfmalloc(sizeof(struct Tile) -
                sizeof(rgba)*(TILE_HEIGHT*TILE_WIDTH - npixels)) ;
  data->count = npixels ;
  data->refcount = 1 ;
  data->summary = 0 ;
  return data ;
}

struct Tile *
forkTile(struct Tile* tile)
{
  if( ++tile->refcount <= 0 )
    FatalUnsupportedXCF(_("Unbelievably many layers?\n"
                          "More likely to be a bug in %s"),progname);
  return tile ;
}

void
freeTile(struct Tile* tile)
{
  if( --tile->refcount == 0 )
    xcffree(tile) ;
}

summary_t
tileSummary(struct Tile *tile)
{
  unsigned i ;
  summary_t summary ;
  if( (tile->summary & TILESUMMARY_UPTODATE) != 0 )
    return tile->summary ;
  summary = TILESUMMARY_ALLNULL + TILESUMMARY_ALLFULL + TILESUMMARY_CRISP ;
  for( i=0; summary && i<tile->count; i++ ) {
    if( FULLALPHA(tile->pixels[i]) )
      summary &= ~TILESUMMARY_ALLNULL ;
    else if( NULLALPHA(tile->pixels[i]) )
      summary &= ~TILESUMMARY_ALLFULL ;
    else
      summary = 0 ;
  }
  summary += TILESUMMARY_UPTODATE ;
  tile->summary = summary ;
  return summary ;
}
  
void
fillTile(struct Tile *tile,rgba data)
{
  unsigned i ;
  for( i = 0 ; i < tile->count ; i++ )
    tile->pixels[i] = data ;
  if( FULLALPHA(data) )
    tile->summary = TILESUMMARY_UPTODATE+TILESUMMARY_ALLFULL+TILESUMMARY_CRISP;
  else if (NULLALPHA(data) )
    tile->summary = TILESUMMARY_UPTODATE+TILESUMMARY_ALLNULL+TILESUMMARY_CRISP;
  else
    tile->summary = TILESUMMARY_UPTODATE ;
}

/* ****************************************************************** */

static void
copyStraightPixels(rgba *dest,unsigned npixels,
                   xcfptr_t ptr,convertParams *params)
{
  unsigned bpp = params->bpp;
  const rgba *lookup = params->lookup;
  rgba base_pixel = params->base_pixel ;
  uint8_t *bp = xcf_file + ptr ;
  xcfCheckspace(ptr,bpp*npixels,
                "pixel array (%u x %d bpp) at %"PRIXPTR,npixels,bpp,ptr);
  while( npixels-- ) {
    rgba pixel = base_pixel ;
    unsigned i ;
    for( i = 0 ; i < bpp ; i++ ) {
      if( params->shift[i] < 0 ) {
        pixel += lookup[*bp++] ;
      } else {
        pixel += *bp++ << params->shift[i] ;
      }
    }
    *dest++ = pixel ;
  }
}

static inline void
copyRLEpixels(rgba *dest,unsigned npixels,xcfptr_t ptr,convertParams *params)
{
  unsigned i,j ;
  rgba base_pixel = params->base_pixel ;

#ifdef xDEBUG
  fprintf(stderr,"RLE stream at %" PRIXPTR ", want %u x %u pixels, base %x\n",
          ptr,params->bpp,npixels,base_pixel);
#endif

  
  /* This algorithm depends on the indexed byte always being the first one */
  if( params->shift[0] < -1 )
    base_pixel = 0 ;
  for( j = npixels ; j-- ; )
    dest[j] = base_pixel ;

  for( i = 0 ; i < params->bpp ; i++ ) {
    int shift = params->shift[i] ;
    if( shift < 0 )
      shift = 0 ;
    for( j = 0 ; j < npixels ; ) {
      int countspec ;
      unsigned count ;
      xcfCheckspace(ptr,2,"RLE data stream");
      countspec = (int8_t) xcf_file[ptr++] ;
      count = countspec >= 0 ? countspec+1 : -countspec ;
      if( count == 128 ) {
        xcfCheckspace(ptr,3,"RLE long count");
        count = xcf_file[ptr++] << 8 ;
        count += xcf_file[ptr++] ;
      }
      if( j + count > npixels )
        FatalBadXCF("Overlong RLE run at %"PRIXPTR" (plane %u, %u left)",
                    ptr,i,npixels-j);
      if( countspec >= 0 ) {
        rgba data = (uint32_t) xcf_file[ptr++] << shift ;
        while( count-- )
          dest[j++] += data ;
      } else {
        while( count-- )
          dest[j++] += (uint32_t) xcf_file[ptr++] << shift ;
      }
    }
    if( i == 0 && params->shift[0] < 0 ) {
      const rgba *lookup = params->lookup ;
      base_pixel = params->base_pixel ;
      for( j = npixels ; j-- ; ) {
        dest[j] = lookup[dest[j]-base_pixel] + base_pixel ;
      }
    }
  }
#ifdef xDEBUG
  fprintf(stderr,"RLE decoding OK at %"PRIXPTR"\n",ptr);
  /*
  for( j = 0 ; j < npixels ; j++ ) {
    if( j % 8 == 0 ) fprintf(stderr,"\n");
    fprintf(stderr," %8x",dest[j]);
  }
  fprintf(stderr,"\n");
  */
#endif
}

static inline void
copyTilePixels(struct Tile *dest, xcfptr_t ptr,convertParams *params)
{
  if( FULLALPHA(params->base_pixel) )
    dest->summary = TILESUMMARY_UPTODATE+TILESUMMARY_ALLFULL+TILESUMMARY_CRISP;
  else
    dest->summary = 0 ;
  switch( XCF.compression ) {
  case COMPRESS_NONE:
    copyStraightPixels(dest->pixels,dest->count,ptr,params);
    break ;
  case COMPRESS_RLE:
    copyRLEpixels(dest->pixels,dest->count,ptr,params);
    break ;
  default:
    FatalUnsupportedXCF(_("%s compression"),
                        _(showXcfCompressionType(XCF.compression)));
  }
}

static struct Tile *
getMaskOrLayerTile(struct tileDimensions *dim, struct xcfTiles *tiles,
                   struct rect want)
{
  struct Tile *tile = newTile(want);

  assert( want.l < want.r && want.t < want.b );
  if( tiles->tileptrs == 0 ) {
    fillTile(tile,0);
    return tile ;
  }
  
#ifdef xDEBUG
  fprintf(stderr,"getMaskOrLayer: (%d-%d),(%d-%d)\n",left,right,top,bottom);
#endif
  
  if( isSubrect(want,dim->c) &&
      (want.l - dim->c.l) % TILE_WIDTH == 0 &&
      (want.t - dim->c.t) % TILE_HEIGHT == 0 ) {
    int tx = TILE_NUM(want.l - dim->c.l);
    int ty = TILE_NUM(want.t - dim->c.t);
    if( want.r == TILEXn(*dim,tx+1) && want.b == TILEYn(*dim,ty+1) ) {
      /* The common case? An entire single tile from the layer */
      copyTilePixels(tile,tiles->tileptrs[tx + ty*dim->tilesx],tiles->params);
      return tile ;
    }
  }

  /* OK, we must construct the wanted tile as a jigsaw */
  {
    unsigned width = want.r-want.l ;
    rgba *pixvert = tile->pixels ;
    rgba *pixhoriz ;
    int y, ty, l0, l1 ;
    int x, tx, c0, c1 ;
    unsigned lstart, lnum ;
    unsigned cstart, cnum ;
    
    if( !isSubrect(want,dim->c) ) {
      if( want.l < dim->c.l ) pixvert += (dim->c.l - want.l),
                                want.l = dim->c.l ;
      if( want.r > dim->c.r ) want.r = dim->c.r ;
      if( want.t < dim->c.t ) pixvert += (dim->c.t - want.t) * width,
                                want.t = dim->c.t ;
      if( want.b > dim->c.b ) want.b = dim->c.b ;
      fillTile(tile,0);
    } else {
      tile->summary = -1 ; /* I.e. whatever the jigsaw pieces say */
    }

#ifdef xDEBUG
    fprintf(stderr,"jig0 (%d-%d),(%d-%d)\n",left,right,top,bottom);
#endif

    for( y=want.t, ty=TILE_NUM(want.t-dim->c.t), l0=TILEYn(*dim,ty);
         y<want.b;
         pixvert += lnum*width, ty++, y=l0=l1 ) {
      l1 = TILEYn(*dim,ty+1) ;
      lstart = y - l0 ;
      lnum = (l1 > want.b ? want.b : l1) - y ;
      
      pixhoriz = pixvert ;
      for( x=want.l, tx=TILE_NUM(want.l-dim->c.l), c0=TILEXn(*dim,tx);
           x<want.r;
           pixhoriz += cnum, tx++, x=c0=c1 ) {
        c1 = TILEXn(*dim,tx+1);
        cstart = x - c0 ;
        cnum = (c1 > want.r ? want.r : c1) - x ;

        {
          static struct Tile tmptile ;
          unsigned dwidth = c1-c0 ;
          unsigned i, j ;
          tmptile.count = (c1-c0)*(l1-l0) ;
#ifdef xDEBUG
          fprintf(stderr,"jig ty=%u(%u-%u-%u)(%u+%u) tx=%u(%u-%u-%u)(%u+%u)\n",
                  ty,l0,y,l1,lstart,lnum,
                  tx,c0,x,c1,cstart,cnum);
#endif
          copyTilePixels(&tmptile,
                         tiles->tileptrs[tx+ty*dim->tilesx],tiles->params);
          for(i=0; i<lnum; i++)
            for(j=0; j<cnum; j++)
              pixhoriz[i*width+j]
                = tmptile.pixels[(i+lstart)*dwidth+(j+cstart)];
          tile->summary &= tmptile.summary ;
        }
      }
    }
  }
  return tile ;
}

void
applyMask(struct Tile *tile, struct Tile *mask)
{
  unsigned i ;
  assertTileCompatibility(tile,mask);
  assert( tile->count == mask->count );
  INIT_SCALETABLE_IF(1);
  invalidateSummary(tile,0);
  for( i=0; i < tile->count ;i++ )
    tile->pixels[i] = NEWALPHA(tile->pixels[i],
                               scaletable[mask->pixels[i]>>ALPHA_SHIFT]
                                         [ALPHA(tile->pixels[i])]);
  freeTile(mask);
}

struct Tile *
getLayerTile(struct xcfLayer *layer,const struct rect *where)
{
  struct Tile *data ;

#ifdef xDEBUG
  fprintf(stderr,"getLayerTile(%s): (%d-%d),(%d-%d)\n",
          layer->name,where->l,where->r,where->t,where->b);
#endif

  if( disjointRects(*where,layer->dim.c) ||
      layer->opacity == 0 ) {
    data = newTile(*where);
    fillTile(data,0);
    return data ;
  }
  
  data = getMaskOrLayerTile(&layer->dim,&layer->pixels,*where);
  if( (data->summary & TILESUMMARY_ALLNULL) != 0 )
    return data ;
  if( layer->hasMask ) {
    struct Tile *mask = getMaskOrLayerTile(&layer->dim,&layer->mask,*where);
    applyMask(data,mask);
  }
  if( layer->opacity < 255 ) {
    const uint8_t *ourtable ;
    int i ;
    invalidateSummary(data,~(TILESUMMARY_CRISP | TILESUMMARY_ALLFULL));
    INIT_SCALETABLE_IF(1);
    ourtable = scaletable[layer->opacity] ;
    for( i=0; i < data->count; i++ )
      data->pixels[i]
        = NEWALPHA(data->pixels[i],ourtable[ALPHA(data->pixels[i])]) ;
  }
  return data ;
}
  
