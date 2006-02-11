/* Palette-manipulation functions functions for xcftools
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

#ifndef PALETTE_H
#define PALETTE_H

#include "xcftools.h"
#include "pixels.h"

#define MAX_PALETTE 256
extern rgba palette[MAX_PALETTE] ;
extern unsigned paletteSize ;

typedef uint8_t index_t ;

void init_palette_hash(void);

/* lookup_or_intern() returns a negative number if there is no room
 * for the color in the palette.
 */
int lookup_or_intern(rgba color);

/* palettify_row will convert a row of 'rgba' values into a packed row
 * of 'uint8_t' indces. If it succeeds without running out of colormap
 * entries, it returns nonzero. On the other hand if it does run out
 * of colormap entries it returns zero _and_ undoes the conversions
 * already done, so that the row is still a full row of 'rgba' values
 * afterwards.
 */
int palettify_row(rgba *row,unsigned ncols);

/* palettify_rows is like palettify_rows, but works on several
 * rows at a time.
 */
int palettify_rows(rgba *rows[],unsigned ncols,unsigned nlines);

#endif /* PALETTE_H */
