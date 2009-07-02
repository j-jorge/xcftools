/* Convert xcf files to png
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

#include "xcftools.h"
#include "flatten.h"
#include "palette.h"
#include <png.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#if HAVE_GETOPT_H
#include <getopt.h>
#else
#include <unistd.h>
#endif
#ifndef HAVE_GETOPT_LONG
#define getopt_long(argc,argv,optstring,l1,l2) getopt(argc,argv,optstring)
#endif

#include "xcf2png.oi"

static void
usage(FILE *where)
{
  fprintf(where,_("Usage: %s [options] filename.xcf[.gz] [layers]\n"),
          progname) ;
  fprintf(where,_("Options:\n"));
  opt_usage(where);
  if( where == stderr ) {
    exit(1);
  }
}

static struct FlattenSpec flatspec ;

static FILE *outfile = NULL ;
static png_structp libpng = NULL ;
static png_infop libpng2 = NULL ;

static void
my_error_callback(png_structp png_ptr, png_const_charp errormsg)
{
  FatalUnexpected(_("Libpng error '%s'"),errormsg);
}

  
static void
init_output(void)
{
  int bit_depth ;
  int color_type ;
  int invert_mono = 0 ;
  png_colorp pngpalette = NULL ;
  png_bytep ptrans = NULL ;
  
  outfile = openout(flatspec.output_filename);
  libpng = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                   png_voidp_NULL,
                                   my_error_callback,
                                   png_error_ptr_NULL);
  if( !libpng )
    FatalUnexpected(_("Couldn't initialize libpng library"));
  
  libpng2 = png_create_info_struct(libpng);
  if( !libpng2 )
    FatalUnexpected("Couldn't create PNG info structure");

  png_init_io(libpng,outfile);
  
  bit_depth = 8;
  switch( flatspec.out_color_mode ) {
  case COLOR_GRAY:
    if( flatspec.default_pixel == PERHAPS_ALPHA_CHANNEL ||
        flatspec.default_pixel == FORCE_ALPHA_CHANNEL )
      color_type = PNG_COLOR_TYPE_GRAY_ALPHA ;
    else
      color_type = PNG_COLOR_TYPE_GRAY ;
    break ;
  case COLOR_RGB:
    if( flatspec.default_pixel == PERHAPS_ALPHA_CHANNEL ||
        flatspec.default_pixel == FORCE_ALPHA_CHANNEL )
      color_type = PNG_COLOR_TYPE_RGB_ALPHA ;
    else
      color_type = PNG_COLOR_TYPE_RGB ;
    break ;
  case COLOR_INDEXED:
    if( paletteSize == 2 &&
        palette[0] == NEWALPHA(0,255) &&
        palette[1] == NEWALPHA(-1,255) ) {
      color_type = PNG_COLOR_TYPE_GRAY ;
      bit_depth = 1 ;
    } else if( paletteSize == 2 &&
               palette[0] == NEWALPHA(-1,255) &&
               palette[1] == NEWALPHA(0,255) ) {
      color_type = PNG_COLOR_TYPE_GRAY ;
      bit_depth = 1 ;
      invert_mono = 1 ;
    } else {
      unsigned i ;
      int need_trans = flatspec.default_pixel == FORCE_ALPHA_CHANNEL ;
      color_type = PNG_COLOR_TYPE_PALETTE ;
      pngpalette = xcfmalloc(paletteSize*sizeof(png_color)) ;
      ptrans = xcfmalloc(paletteSize);
      for(i = 0; i<paletteSize; i++ ) {
        pngpalette[i].red = 255 & (palette[i] >> RED_SHIFT);
        pngpalette[i].green = 255 & (palette[i] >> GREEN_SHIFT);
        pngpalette[i].blue = 255 & (palette[i] >> BLUE_SHIFT);
        if( (ptrans[i] = ALPHA(palette[i])) != 255 )
          need_trans = 1 ;
      }
      if( !need_trans ) {
        xcffree(ptrans);
        ptrans = NULL ;
      }
      if( paletteSize <= 2 )
        bit_depth = 1 ;
      else if( paletteSize <= 4 )
        bit_depth = 2 ;
      else if( paletteSize <= 16 )
        bit_depth = 4 ;
      else
        bit_depth = 8;
    }
    break ;
  default:
    FatalUnexpected("This can't happen (unknown out_color_mode)");
  }

  if( verboseFlag ) {
    fprintf(stderr,"Writing PNG: %s%s%s%s, %d bits",
            color_type & PNG_COLOR_MASK_COLOR ? _("color") : _("grayscale"),
            color_type & PNG_COLOR_MASK_PALETTE ? _("+palette") : "",
            color_type & PNG_COLOR_MASK_ALPHA ? _("+alpha") : "",
            ptrans || NULLALPHA(flatspec.default_pixel)
            ? _("+transparency") : "",
            bit_depth);
    if( pngpalette )
      fprintf(stderr,_(" (%d colors)"),paletteSize);
    fprintf(stderr,"\n");
  }
  
  png_set_IHDR(libpng,libpng2,flatspec.dim.width,flatspec.dim.height,
               bit_depth, color_type,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  if( invert_mono )
    png_set_invert_mono(libpng);
  
  if( pngpalette )
    png_set_PLTE(libpng,libpng2,pngpalette,paletteSize);
  if( ptrans )
    png_set_tRNS(libpng,libpng2,ptrans,paletteSize,NULL);
  else if ( !pngpalette &&
            NULLALPHA(flatspec.default_pixel) ) {
    static png_color_16 trans ;
    trans.gray =
      trans.red = 255 & (flatspec.default_pixel >> RED_SHIFT) ;
    trans.green = 255 & (flatspec.default_pixel >> GREEN_SHIFT) ;
    trans.blue = 255 & (flatspec.default_pixel >> BLUE_SHIFT) ;
    png_set_tRNS(libpng,libpng2,NULL,0,&trans);
  }

  /* png_set_text here */

  png_write_info(libpng,libpng2);

  if( bit_depth < 8 )
    png_set_packing(libpng);

  switch( color_type ) {
  case PNG_COLOR_TYPE_RGB:
  case PNG_COLOR_TYPE_RGBA:
#if (BLUE_SHIFT < RED_SHIFT) == !defined(WORDS_BIGENDIAN)
    png_set_bgr(libpng);
#endif
    if( color_type == PNG_COLOR_TYPE_RGB )
#if (ALPHA_SHIFT < RED_SHIFT) == !defined(WORDS_BIGENDIAN)
      png_set_filler(libpng,0,PNG_FILLER_BEFORE);
    else
      png_set_swap_alpha(libpng);
#else
    png_set_filler(libpng,0,PNG_FILLER_AFTER);
#endif
    break ;
  case PNG_COLOR_TYPE_GRAY:
    png_set_filler(libpng,0,PNG_FILLER_AFTER);
    break ;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
  case PNG_COLOR_TYPE_PALETTE:
    break ;
  default:
    FatalUnexpected("This can't happen (unexpected png color_type)");
  }
}


static void
raw_callback(unsigned num, rgba *pixels) {
  if( libpng == NULL ) {
    init_output() ;
  }
  png_write_row(libpng,(png_bytep)pixels);
  xcffree(pixels);
}

static void
graying_callback(unsigned num, rgba *pixels) {
  png_bytep fillptr = (uint8_t *)pixels ;
  unsigned i ;
  for( i = 0 ; i < num ; i++ ) {
    rgba pixel = pixels[i] ;
    int g = degrayPixel(pixel) ;
    if( g == -1 )
      FatalGeneric(103,
                   _("Grayscale output selected, but colored pixel(s) found"));
    *fillptr++ = g ;
    *fillptr++ = ALPHA(pixel) ;
  }
  raw_callback(num,pixels);
}

static void
optimistic_palette_callback(unsigned num,rgba *pixels) {
  unsigned prev_size = paletteSize ;
  if( !palettify_row(pixels,num)  || paletteSize != prev_size )
    FatalUnexpected("Oops! Somehow the precomputed palette does not suffice "
                    "after all...");
  raw_callback(num,pixels);
}

static enum out_color_mode
guessIndexed(struct FlattenSpec *spec,rgba *allPixels[])
{
  if( allPixels == NULL ) {
    if (spec->gimpish_indexed && colormapLength ) {
      unsigned i ;
      init_palette_hash();
      for( i=0; i<colormapLength; i++ )
        lookup_or_intern(NEWALPHA(colormap[i],255));
      if( lookup_or_intern( FULLALPHA(spec->default_pixel) ?
                            spec->default_pixel : 0 ) >= 0 )
        return COLOR_INDEXED ;
    }
  } else {
    init_palette_hash() ;
    if( palettify_rows(allPixels,spec->dim.width,spec->dim.height) ) {
      /* Might grayscale sometimes be preferred? No, that is what
       * -g is for! */
      return COLOR_INDEXED ;
    }
  }
  return COLOR_BY_CONTENTS ;
}

static lineCallback
selectCallback(void)
{
  switch( flatspec.out_color_mode ) {
  default:
  case COLOR_RGB: return &raw_callback ;
  case COLOR_GRAY: return &graying_callback ;
  case COLOR_INDEXED:
    if( flatspec.process_in_memory )
      return &raw_callback ;
    else
      return &optimistic_palette_callback ;
  }
}

/* findUnusedColor() will prefer to find a gray pixel */
static rgba
findUnusedColor(rgba *pixels[],unsigned width,unsigned height)
{
  size_t freqtab[256] ;
  unsigned x,y ;
  unsigned i,j ;
  rgba sofar ;
  
  for( i=0; i<256; i++ )
    freqtab[i] = 0 ;
  for( y=0; y<height; y++ )
    for( x=0; x<width; x++ )
      if( pixels[y][x] )
        freqtab[255 & (pixels[y][x] >> RED_SHIFT)] ++ ;
  j = 0 ;
  for( i=0; i<256; i++ ) {
    if( freqtab[i] == 0 ) {
      return ((rgba)i << RED_SHIFT) +
        ((rgba)i << GREEN_SHIFT) +
        ((rgba)i << BLUE_SHIFT) +
        ((rgba)255 << ALPHA_SHIFT) ;
    }
    if( freqtab[i] < freqtab[j] ) {
      j = i ;
    }
  }
  sofar = ((rgba)255<<ALPHA_SHIFT) + ((rgba)j << RED_SHIFT) ;

  for( i=0; i<256; i++ )
    freqtab[i] = 0 ;
  for( y=0; y<height; y++ )
    for( x=0; x<width; x++ )
      if( ((((rgba)255 << ALPHA_SHIFT) + ((rgba)255 << RED_SHIFT))
           & pixels[y][x]) == sofar )
        freqtab[255 & (pixels[y][x] >> GREEN_SHIFT)] ++ ;
  j = 0 ;
  for( i=0; i<256; i++ ) {
    if( freqtab[i] == 0 ) {
      return sofar + ((rgba)i << GREEN_SHIFT);
    }
    if( freqtab[i] < freqtab[j] ) {
      j = i ;
    }
  }
  sofar += (rgba)j << GREEN_SHIFT ;

  for( i=0; i<256; i++ )
    freqtab[i] = 0 ;
  for( y=0; y<height; y++ )
    for( x=0; x<width; x++ )
      if( ((((rgba)255 << ALPHA_SHIFT) +
           ((rgba)255 << RED_SHIFT) +
            ((rgba)255 << GREEN_SHIFT))
           & pixels[y][x]) == sofar )
        freqtab[255 & (pixels[y][x] >> BLUE_SHIFT)] ++ ;
  for( i=0; i<256; i++ ) {
    if( freqtab[i] == 0 ) {
      return sofar + ((rgba)i << BLUE_SHIFT);
    }
  }

  return 0 ;
}

int
main(int argc,char **argv)
{
  int option ;
  const char *unzipper = NULL ;
  const char *infile = NULL ;

  setlocale(LC_ALL,"");
  progname = argv[0] ;
  nls_init();

  if( argc <= 1 ) gpl_blurb() ;
  
  init_flatspec(&flatspec) ;
  while( (option=getopt_long(argc,argv,"-"OPTSTRING,longopts,NULL)) >= 0 )
    switch(option) {
      #define OPTION(char,long,desc,man) case char:
      #include "options.i"
    case 1:
      if( infile ) 
        add_layer_request(&flatspec,optarg);
      else
        infile = optarg ;
      break ;
    case '?':
      usage(stderr);
    default:
      FatalUnexpected("Getopt(_long) unexpectedly returned '%c'",option);
    }
  if( infile == NULL ) {
    usage(stderr);
  }
  
  read_or_mmap_xcf(infile,unzipper);
  getBasicXcfInfo() ;
  initColormap();
 
  complete_flatspec(&flatspec,guessIndexed);
  if( flatspec.process_in_memory ) {
    rgba **allPixels = flattenAll(&flatspec);
     
    analyse_colormode(&flatspec,allPixels,guessIndexed);

    /* See if we can do alpha compaction.
     */
    if( flatspec.partial_transparency_mode != ALLOW_PARTIAL_TRANSPARENCY &&
        !FULLALPHA(flatspec.default_pixel) &&
        flatspec.out_color_mode != COLOR_INDEXED ) {
      rgba unused = findUnusedColor(allPixels,
                                    flatspec.dim.width,
                                    flatspec.dim.height);
      if( unused && (flatspec.out_color_mode == COLOR_RGB ||
                     degrayPixel(unused) >= 0) ) {
        unsigned x,y ;
        unused = NEWALPHA(unused,0) ;
        for( y=0; y<flatspec.dim.height; y++)
          for( x=0; x<flatspec.dim.width; x++)
            if( allPixels[y][x] == 0 )
              allPixels[y][x] = unused ;
        flatspec.default_pixel = unused ;
      }
    }
    shipoutWithCallback(&flatspec,allPixels,selectCallback());
  } else {
    flattenIncrementally(&flatspec,selectCallback());
  }
  if( libpng ) {
    png_write_end(libpng,libpng2);
    png_destroy_write_struct(&libpng,&libpng2);
  }
  if( outfile ) {
    closeout(outfile,flatspec.output_filename);
  }
  return 0 ;
}
