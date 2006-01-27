/* This program is written by Henning Makholm, and is in the
 * public domain.
 */

#define TEST_IMAGE_WIDTH 50
#define TEST_IMAGE_HEIGHT 50

#include <math.h>
#include <stdlib.h>

void
makepixel(int x,int y,int *r,int *g,int *b,int *a) {
  double yy = 2*(double)y/(TEST_IMAGE_HEIGHT-1) - 1 ;
  double xx = 2*(double)x/(TEST_IMAGE_WIDTH-1) - 1 ;
  double rad = sqrt(xx*xx+yy*yy) ;
  unsigned t  = x + abs((y - TEST_IMAGE_HEIGHT/2)) ;
  t = t / 10 + ((600 + y - TEST_IMAGE_HEIGHT/2) / 10)*77 ;
  if( rad < 0.9 )
    *a = 255 ;
  else if( rad < 1 )
    *a = 190 ;
  else if( rad < 1.2 )
    *a = 73 ;
  else
    *a = 0 ;

  t *= 3847822 ;
  t ^= 29938132 ;
  t %= 2093847 ;
  *r = 120 * ((t >> 3) % 3) + 3 ;
  *g = 120 * ((t >> 7) % 3) + 3 ;
  *b = 120 * ((t >> 10) % 3) + 3 ;
}

#include "mkbase.i"
