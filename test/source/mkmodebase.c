/* This program is written by Henning Makholm, and is in the
 * public domain.
 */

#define TEST_IMAGE_WIDTH 64
#define TEST_IMAGE_HEIGHT 64

void
makepixel(int x,int y,int *r,int *g,int *b,int *a) {
  if( x < 3 || x >= 61 ) {
    *a = 0 ;
    return ;
  }
  if( y < 3 || y >= 61 ||
      x < 6 || x >= 58 ) {
    *a = 255 ;
    *r=*g=*b= 255*(x>=32) ;
    return ;
  }
  x -= 6 ;
  if( x <= 17 ) {
    *a = x*15 ;
    *r = 255 ;
    *g = 0 ;
    *b = 17*7 ;
    return ;
  }
  *a = 255 ;
  x -= 17 ;
  if( x <= 17 ) {
    *r = 255 ;
    *g = x*15 ;
    *b = (17-x)*7 ;
    return ;
  }
  x -= 17 ;
  if( x <= 17 ) {
    *r = (17-x)*15 ;
    *g = (17-x)*15 ;
    *b = (x-1)*15 ;
    return ;
  }
  *r=255 ;
  *g=*b=0 ;
}

#include "mkbase.i"
