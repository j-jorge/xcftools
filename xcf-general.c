/* Generic functions for reading XCF files
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
#include <string.h>
#include <errno.h>
#ifdef HAVE_ICONV
# include <iconv.h>
#elif !defined(ICONV_CONST)
# define ICONV_CONST const
#endif

uint8_t *xcf_file = 0 ;
size_t xcf_length ;
int use_utf8 = 0 ;

uint32_t
xcfOffset(uint32_t addr,int spaceafter)
{
  uint32_t apparent ;
  xcfCheckspace(addr,4,"(xcfOffset)");
  apparent = xcfL(addr);
  xcfCheckspace(apparent,spaceafter,
                "Too large offset (%" PRIX32 ") at position %" PRIX32,
                apparent,addr);
  return apparent ;
}

int
xcfNextprop(uint32_t *master,uint32_t *body)
{
  uint32_t ptr, length, total, minlength ;
  PropType type ;
  ptr = *master ;
  xcfCheckspace(ptr,8,"(property header)");
  type = xcfL(ptr);
  length = xcfL(ptr+4);
  *body = ptr+8 ;
  *master = ptr+8+length ;
  total = 8 + length + (type != PROP_END ? 8 : 0) ;
  if( total < length ) /* Check overwrap */
    FatalBadXCF("Overlong property at %" PRIX32, ptr);
  xcfCheckspace(ptr,total,"Overlong property at %" PRIX32,ptr) ;

  switch(type) {
  case PROP_COLORMAP:    minlength = 4+3*xcfL(ptr+8); break;
  case PROP_COMPRESSION: minlength = 1; break;
  case PROP_OPACITY:     minlength = 4; break;
  case PROP_APPLY_MASK:  minlength = 4; break;
  case PROP_OFFSETS:     minlength = 8; break;
  case PROP_MODE:        minlength = 4; break;
  default:               minlength = 0; break;
  }
  if( length < minlength )
    FatalBadXCF("Short %s property at %" PRIX32 " (%" PRIu32 "<%" PRIu32 ")",
                showPropType(type),ptr,length,minlength);
  return type ;
}

const char*
xcfString(uint32_t ptr,uint32_t *after)
{
  uint32_t length ;
  unsigned i ;
  ICONV_CONST char *utf8master ;
  
  xcfCheckspace(ptr,4,"(string length)");
  length = xcfL(ptr) ;
  ptr += 4 ;
  xcfCheckspace(ptr,length,"(string)");
  utf8master = (ICONV_CONST char*)(xcf_file+ptr) ;
  if( after ) *after = ptr + length ;
  if( length == 0 || utf8master[length-1] != 0 )
    FatalBadXCF("String at %" PRIX32 " not zero-terminated",ptr-4);
  length-- ;

  if( use_utf8 ) return utf8master ;

  /* We assume that the local character set includes ASCII...
   * Check if conversion is needed at all
   */
  for( i=0 ; ; i++ ) {
    if( i == length )
      return utf8master ; /* Only ASCII after all */
    if( utf8master[i] == 0 )
      FatalBadXCF("String at %" PRIX32 " has embedded zeroes",ptr-4);
    if( (int8_t) utf8master[i] < 0 )
      break ;
  }
#ifdef HAVE_ICONV
  {
    size_t targetsize = length+1 ;
    int sloppy_translation = 0 ;
    iconv_t cd = iconv_open("//TRANSLIT","UTF-8");
    if( cd == (iconv_t) -1 ) {
      cd = iconv_open("","UTF-8");
      sloppy_translation = 1 ;
    }
    if( cd == (iconv_t) -1 )
      iconv_close(cd) ; /* Give up; perhaps iconv doesn't know UTF-8 */
    else
      while(1) {
        char *buffer = xcfmalloc(targetsize) ;
        ICONV_CONST char *inbuf = utf8master ;
        char *outbuf = buffer ;
        size_t incount = length ;
        size_t outcount = targetsize ;
        while(1) { /* Loop for systems without //ICONV support */
          size_t result = iconv(cd,&inbuf,&incount,&outbuf,&outcount) ;
          if( result == (size_t)-1 && errno == EILSEQ &&
              sloppy_translation && outcount > 0 ) {
            *outbuf++ = '?' ;
            outcount-- ;
            while( (int8_t)*inbuf < 0 ) inbuf++, incount-- ;
            continue ;
          }
          if( result != (size_t)-1 ) {
            if( outcount == 0 )
              errno = E2BIG ;
            else {
              *outbuf = 0 ;
              iconv_close(cd) ;
              return buffer ;
            }
          }
          break ;
        }
        if( errno == EILSEQ || errno == EINVAL )
          FatalBadXCF("Bad UTF-8 encoding '%s' at %" PRIXPTR,
                      inbuf,(uintptr_t)((inbuf-utf8master)+ptr));
        if( errno == E2BIG ) {
          targetsize += 1+incount ;
          xcffree(buffer) ;
          continue ;
        }
        FatalUnexpected("!iconv on layer name at %"PRIX32,ptr);
      }
  }
#endif
  {
    static int warned = 0 ;
    if( !warned ) {
      fprintf(stderr,_("Warning: one or more layer names could not be\n"
                       "         translated to the local character set.\n"));
      warned = 1 ;
    }
  }
  return utf8master ;
}

/* ****************************************************************** */

void
computeDimensions(struct tileDimensions *d)
{
  d->c.r = d->c.l + d->width ;
  d->c.b = d->c.t + d->height ;
  d->tilesx = (d->width+TILE_WIDTH-1)/TILE_WIDTH ;
  d->tilesy = (d->height+TILE_HEIGHT-1)/TILE_HEIGHT ;
  d->ntiles = d->tilesx * d->tilesy ;
}

struct xcfImage XCF ;

void
getBasicXcfInfo(void)
{
  uint32_t ptr, data, layerfile ;
  PropType type ;
  int i ;
  
  xcfCheckspace(0,14+7*4,"(very short)");
  if( strcmp((char*)xcf_file,"gimp xcf file") == 0 )
    XCF.version = 0 ;
  else if( xcf_file[13] == 0 &&
          sscanf((char*)xcf_file,"gimp xcf v%d",&XCF.version) == 1 )
    ;
  else
    FatalBadXCF(_("Not an XCF file at all (magic not recognized)"));

  if( XCF.version < 0 || XCF.version > 2 ) {
    fprintf(stderr,
            _("Warning: XCF version %d not supported (trying anyway...)\n"),
            XCF.version);
  }
  
  XCF.compression = COMPRESS_NONE ;
  XCF.colormapptr = 0 ;
  
  ptr = 14 ;
  XCF.width    = xcfL(ptr); ptr += 4 ;
  XCF.height   = xcfL(ptr); ptr += 4 ;
  XCF.type     = xcfL(ptr); ptr += 4 ;
  while( (type = xcfNextprop(&ptr,&data)) != PROP_END ) {
    switch(type) {
    case PROP_COLORMAP:
      XCF.colormapptr = data ;
      break ;
    case PROP_COMPRESSION:
      XCF.compression = xcf_file[data] ;
      break ;
    default:
      /* Ignore unknown properties */
      break ;
    }
  }

  layerfile = ptr ;
  for( XCF.numLayers = 0 ; xcfOffset(ptr,8*4) ; XCF.numLayers++, ptr+=4  )
    ;
  XCF.layers = xcfmalloc(XCF.numLayers * sizeof(struct xcfLayer)) ;
  for( i = 0 ; i < XCF.numLayers ; i++ ) {
    struct xcfLayer *L = XCF.layers + i ;
    ptr = xcfL(layerfile+4*(XCF.numLayers-1-i)) ;
    L->mode = GIMP_NORMAL_MODE ;
    L->opacity = 255 ;
    L->isVisible = 1 ;
    L->hasMask = 0 ;
    L->dim.width = xcfL(ptr); ptr+=4 ;
    L->dim.height = xcfL(ptr); ptr+=4 ;
    L->type = xcfL(ptr); ptr+=4 ;
    L->name = xcfString(ptr,&ptr);
    L->propptr = ptr ;
    while( (type = xcfNextprop(&ptr,&data)) != PROP_END ) {
      switch(type) {
      case PROP_OPACITY:
        L->opacity = xcfL(data);
        if( L->opacity > 255 )
          L->opacity = 255 ;
        break ;
      case PROP_VISIBLE:
        L->isVisible = xcfL(data) != 0 ;
        break ;
      case PROP_APPLY_MASK:
        L->hasMask = xcfL(data) != 0 ;
        break ;
      case PROP_OFFSETS:
        L->dim.c.l = (int32_t)(xcfL(data  )) ;
        L->dim.c.t = (int32_t)(xcfL(data+4)) ;
        break ;
      case PROP_MODE:
        L->mode = xcfL(data);
        break ;
      default:
        /* Ignore unknown properties */
        break ;
      }
    }
    xcfCheckspace(ptr,8,"(end of layer %s)",L->name);
    L->pixels.tileptrs = 0 ;
    L->pixels.hierarchy = xcfOffset(ptr  ,4*4);
    L->mask.tileptrs = 0 ;
    L->mask.hierarchy   = xcfOffset(ptr+4,4*4);

    computeDimensions(&L->dim);
  }
}

