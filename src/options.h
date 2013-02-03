/* 
 * This file was written by Julien Jorge <julien.jorge@gamned.org>
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
#ifndef OPTIONS_H
#define OPTIONS_H

#include "flatten.h"
#include "process.h"

#include <getopt.h>

/*----------------------------------------------------------------------------*/
#define short_options_prefix "-"

/*----------------------------------------------------------------------------*/
#define option_help_value 'h'
#define short_option_help "h"
#define option_help { "help", 0, NULL, option_help_value }

/*----------------------------------------------------------------------------*/
#define option_version_value 'V'
#define short_option_version "V"
#define option_version { "version", 0, NULL, option_version_value }

/*----------------------------------------------------------------------------*/
#define option_verbose_value 'v'
#define short_option_verbose "v"
#define option_verbose { "verbose", 0, NULL, option_verbose_value }

/*----------------------------------------------------------------------------*/
#define option_bzip_value 'j'
#define short_option_bzip "j"
#define option_bzip { "bzip", 0, NULL, option_bzip_value }

/*----------------------------------------------------------------------------*/
#define option_gzip_value 'z'
#define short_option_gzip "z"
#define option_gzip { "gzip", 0, NULL, option_gzip_value }

/*----------------------------------------------------------------------------*/
#define option_unpack_value 'Z'
#define short_option_unpack "Z:"
#define option_unpack { "unpack", 1, NULL, option_unpack_value }

/*----------------------------------------------------------------------------*/
#define option_output_value 'o'
#define short_option_output "o:"
#define option_output { "output", 1, NULL, option_output_value }

/*----------------------------------------------------------------------------*/
#define option_background_value 'b'
#define short_option_background "b:"
#define option_background { "background", 1, NULL, option_background_value }

/*----------------------------------------------------------------------------*/
#define option_force_alpha_value 'A'
#define short_option_force_alpha "A"
#define option_force_alpha { "force-alpha", 0, NULL, option_force_alpha_value }

/*----------------------------------------------------------------------------*/
#define option_color_value 'c'
#define short_option_color "c"
#define option_color { "color", 0, NULL, option_color_value }
#define option_colour { "colour", 0, NULL, option_color_value }

/*----------------------------------------------------------------------------*/
#define option_gray_value 'g'
#define short_option_gray "g"
#define option_gray { "gray", 0, NULL, option_gray_value }
#define option_grey { "grey", 0, NULL, option_gray_value }

/*----------------------------------------------------------------------------*/
#define option_truecolor_value 'T'
#define short_option_truecolor "T"
#define option_truecolor { "truecolor", 0, NULL, option_truecolor_value }

/*----------------------------------------------------------------------------*/
#define option_for_gif_value 'G'
#define short_option_for_gif "G"
#define option_for_gif { "for-gif", 0, NULL, option_for_gif_value }

/*----------------------------------------------------------------------------*/
#define option_dissolve_value 'D'
#define short_option_dissolve "D"
#define option_dissolve { "dissolve", 0, NULL, option_dissolve_value }

/*----------------------------------------------------------------------------*/
#define option_full_image_value 'f'
#define short_option_full_image "f"
#define option_full_image { "full-image", 0, NULL, option_full_image_value }

/*----------------------------------------------------------------------------*/
#define option_size_value 'S'
#define short_option_size "S:"
#define option_size { "size", 1, NULL, option_size_value }

/*----------------------------------------------------------------------------*/
#define option_offset_value 'O'
#define short_option_offset "O:"
#define option_offset { "offset", 1, NULL, option_offset_value }

/*----------------------------------------------------------------------------*/
#define option_autocrop_value 'C'
#define short_option_autocrop "C"
#define option_autocrop { "autocrop", 0, NULL, option_autocrop_value }

/*----------------------------------------------------------------------------*/
#define option_mode_value 300
#define option_mode { "mode", 1, NULL, option_mode_value }

/*----------------------------------------------------------------------------*/
#define option_percent_value 301
#define option_percent { "percent", 1, NULL, option_percent_value }

/*----------------------------------------------------------------------------*/
#define option_opacity_value 302
#define option_opacity { "opacity", 1, NULL, option_opacity_value }

/*----------------------------------------------------------------------------*/
#define option_mask_value 303
#define option_mask { "mask", 0, NULL, option_mask_value }

/*----------------------------------------------------------------------------*/
#define option_nomask_value 304
#define option_nomask { "nomask", 0, NULL, option_nomask_value }

/*----------------------------------------------------------------------------*/
#define option_utf8_value 'u'
#define short_option_utf8 "u"
#define option_utf8 { "utf8", 0, NULL, option_utf8_value }

int option_parse
( int argc, char** argv, const char* short_options,
  const struct option* long_options,
  struct ProcessControl* p, struct FlattenSpec* flatspec );

#endif /* OPTIONS_H */
