/* Run-time scaletable computation for Xcftools
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "pixels.h"
#ifndef PRECOMPUTED_SCALETABLE

uint8_t scaletable[256][256] ;
int ok_scaletable = 0 ;

void
mk_scaletable(void)
{
  unsigned p, q, r ;
  if( ok_scaletable ) return ;
  for( p = 0 ; p < 128 ; p++ )
    for( q = 0 ; q <= p ; q++  ) {
      r = (p*q+127)/255 ;
      scaletable[p][q] = scaletable[q][p] = r ;
      scaletable[255-p][q] = scaletable[q][255-p] = q-r ;
      scaletable[p][255-q] = scaletable[255-q][p] = p-r ;
      scaletable[255-p][255-q] = scaletable[255-q][255-p] = (255-q)-(p-r) ;
    }
  ok_scaletable = 1 ;
}

#endif
    
