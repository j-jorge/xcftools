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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef FLATTEN_H
#define FLATTEN_H

#include "xcftools.h"
#include "pixels.h"

#define PERHAPS_ALPHA_CHANNEL (NEWALPHA(0,1))
#define FORCE_ALPHA_CHANNEL (NEWALPHA(0,2))
#define CHECKERED_BACKGROUND (NEWALPHA(0,200))
struct FlattenSpec {
  struct tileDimensions dim ;
  rgba default_pixel ;
  int numLayers ;
  struct xcfLayer *layers ;

  const char * transmap_filename ;
  const char * output_filename ;
  enum out_color_mode {
    COLOR_BY_FILENAME,
    COLOR_BY_CONTENTS,
    COLOR_INDEXED,
    COLOR_RGB,
    COLOR_GRAY,
    COLOR_MONO
  } out_color_mode ;
  enum { ALLOW_PARTIAL_TRANSPARENCY,
         DISSOLVE_PARTIAL_TRANSPARENCY,
         FORBID_PARTIAL_TRANSPARENCY,
         PARTIAL_TRANSPARENCY_IMPOSSIBLE
  } partial_transparency_mode ;
  int process_in_memory ;
  int gimpish_indexed ;
};

/* From flatspec.c */

void init_flatspec(struct FlattenSpec *);

void add_layer_request(struct FlattenSpec *,const char *name);
struct xcfLayer *lastlayerspec(struct FlattenSpec *,const char *option);

typedef enum out_color_mode (*guesser) (struct FlattenSpec *,rgba **);

/* Call this after processing options, and after opening the XCF file */
void complete_flatspec(struct FlattenSpec *,guesser);
void analyse_colormode(struct FlattenSpec *,rgba **allPixels,guesser);

/* From flatten.c */

typedef void (*lineCallback)(unsigned num,rgba *pixels);
void flattenIncrementally(struct FlattenSpec *,lineCallback);
rgba **flattenAll(struct FlattenSpec*);
void shipoutWithCallback(struct FlattenSpec *,rgba **pixels,lineCallback);

#endif /* FLATTEN_H */
