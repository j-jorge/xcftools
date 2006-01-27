/* -*- C -*-
 * This program is written by Henning Makholm, and is in the
 * public domain.
 */

#include <png.h>
#include <stdio.h>
#include <stdlib.h>

static void
error(png_structp png_ptr, png_const_charp errormsg)
{
  fprintf(stderr,"PNG error: %s\n",errormsg);
  exit(1);
}


int
main(void)
{
  png_structp libpng = NULL ;
  png_infop libpng2 = NULL ;
  unsigned char row[TEST_IMAGE_WIDTH*4] ;
  unsigned x,y ;
  int r,g,b,a ;

  libpng = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                   png_voidp_NULL,
                                   error,
                                   png_error_ptr_NULL);
  if( !libpng )
    error(libpng,"Couldn't initialize libpng library");
  
  libpng2 = png_create_info_struct(libpng);
  if( !libpng2 )
    error(libpng,"Couldn't create PNG info structure");

  png_init_io(libpng,stdout);
    
  png_set_IHDR(libpng,libpng2,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,
               8, PNG_COLOR_TYPE_RGB_ALPHA,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(libpng,libpng2);
               
  for( y=0; y<TEST_IMAGE_HEIGHT; y++ ) {
    for( x=0; x<TEST_IMAGE_WIDTH; x++ ) {
      makepixel(x,y,&r,&g,&b,&a);
      row[x*4+0] = r ;
      row[x*4+1] = g ;
      row[x*4+2] = b ;
      row[x*4+3] = a ;
    }
    png_write_row(libpng,row);
  }
  png_write_end(libpng,libpng2);
  return 0 ;  
}
