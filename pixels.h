/* Pixel and tile functions for xcftools
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

#ifndef PIXELS_H
#define PIXELS_H

#include "xcftools.h"

/* MACROS FOR INTERNAL PIXEL ORDERING HERE */
/*=========================================*/
/* In principle the internal representation of pixels may change.
 * - this was supposed to allow an optimization where a layer could
 * be represented as a pointer into the mmapped xcf file, if
 * alignment, bpp, and endianness agreed (the point was that the
 * pixel representation had to agree with the endianness).
 *
 * However, it turns out that the current Gimp _always_ saves images
 * with RLE encoding of tiles, so such an effort would be in vain.
 *
 * Just for modularity, nevertheless try to isolate knowledge of
 * the RGBA-to-machine-word packing in this section of the
 * header file. Define new macros if necessary.
 *
 * Given that we don't have to agree with the uncompressed
 * RLE format, we choose to have the alpha in the _least_
 * significant byte on all archs - it is tested and used more
 * often than the visible channels.
 */
typedef uint32_t rgba ;

#define ALPHA_SHIFT 0
#define RED_SHIFT 8
#define GREEN_SHIFT 16
#define BLUE_SHIFT 24

#define ALPHA(rgba) ((uint8_t)(rgba))
#define FULLALPHA(rgba) ((uint8_t)(rgba) == 255)
#define NULLALPHA(rgba) ((uint8_t)(rgba) == 0)
#define NEWALPHA(rgb,a) (((rgba)(rgb) & 0xFFFFFF00) + (a))

extern const uint8_t scaletable[256][256] ;
extern const rgba graytable[256] ;
extern rgba colormap[256] ;
extern unsigned colormapLength ;
void initLayer(struct xcfLayer *);
void initColormap();

int degrayPixel(rgba); /* returns -1 for non-gray pixels */

/* ******************************************************* */

#define TILEXn(dim,tx) \
    ((tx)==(dim).tilesx ? (dim).c.r : (dim).c.l + ((tx)*TILE_WIDTH))
#define TILEYn(dim,ty) \
    ((ty)==(dim).tilesy ? (dim).c.b : (dim).c.t + ((ty)*TILE_HEIGHT))

#if __i386__
/* This is probably the only common architecture where small constants
 * are more efficient for byte operations.
 */
typedef int8_t summary_t ;
typedef short int refcount_t ;
#else
typedef int summary_t ;
typedef int refcount_t ;
#endif

#define TILESUMMARY_UPTODATE 8
#define TILESUMMARY_ALLNULL 4
#define TILESUMMARY_ALLFULL 2
#define TILESUMMARY_CRISP   1 /* everyting either null or full */
struct Tile {
  refcount_t refcount ;
  summary_t summary ; /* a combination of TIMESUMMARY_FOO constatns */
  unsigned count ;
  rgba pixels[TILE_WIDTH * TILE_HEIGHT];
};
/* Actually, the Tile structures that get allocated many not have
 * room for that many pixels. We subtract the space for those we don't
 * use - which is Not Legal C, but ought to be portable.
 *  OTOH, one can also use a static struct Tile for temporary storage.
 */


#define assertTileCompatibility(t1,t2) assert((t1)->count==(t2)->count)

struct Tile *newTile(struct rect);
struct Tile *forkTile(struct Tile*);
void freeTile(struct Tile*);
#define invalidateSummary(tile,mask) \
  do{ assert((tile)->refcount==1); (tile)->summary &= mask; } while(0)
summary_t __ATTRIBUTE__((pure)) tileSummary(struct Tile *);

void fillTile(struct Tile*,rgba);

/* applyMask() destructively changes tile,
 * applyMask() gets ownership of mask
 */
void applyMask(struct Tile *tile, struct Tile *mask);

struct Tile *getLayerTile(struct xcfLayer *,const struct rect *);

#endif /* FLATTEN_H */
