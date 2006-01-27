/* Option processing for xcftools -*- C -*-
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

OPTIONGROUP(1i,General options);

OPTION('h',--help,show this message,
       (Print an option summery to standard output and exit with a
        return code of 0.
        ));
usage(stdout);
exit(0);

OPTION('V',--version,show version,
       (Print the version numer of
        .B xcftools
        to standard output and exit with a return code of 0.
        ));
printf(PACKAGE_STRING "\n");
exit(0);

OPTION('v',--verbose,show progress messages,
       (Print progress messages about the conversion to standard error.
        ));
verboseFlag = 1 ;
break ;

OPTION('j',--bzip,input is bzip2 compressed,
       (Equivalent to
        .BR "\-Z bzcat" .
        Default if the filename ends with
        .BR bz2 .
        ));
unzipper = "bzcat" ;
break ;
     
OPTION('z',--gzip,input is gzip compressed,
       (Equivalent to
        .BR "\-Z gzcat" .
        Default if the filename ends with
        .BR gz .
        ));
unzipper = "gzcat" ;
break ;

OPTION('Z',--unpack,(pgm) use 'pgm' to decompress input,
       (Specify a command that the input file is filtered through
        before being interpreted as an XCF file. The command is invoked as
        .I command filename
        and must produce output to its standard output.
        Note that it is not possible to specify arguments as part of
        .IR command .
        An uncompressor is selected automatically if the filename ends
        with
        .B gz
        or
        .BR bz2 ;
        to suppress this, use
        .B \-Z cat
        (which is implemented without actually starting a
         .BR cat (1)
         process).
        ));
unzipper = optarg ;
break ;

#ifdef XCF2FOO

OPTION('o',--output,(filename) name output file,
       (Write the converted picture to
        .I filename
        instead of to standard output.
        ));
flatspec.output_filename = optarg ;
break ;

#ifdef XCF2PNM
OPTION('a',--alpha,(filename) write transparency map,
       (Output a transparency mask for the flattened image to
        .I filename
        as a
        .BR pgm (5)
        file, in addition to the ordinary output.
        If the flattened image is completely opaque, this will produce an
        error message and exit status 101
        (the alpha file may or may not be written); use
        .B \-A
        to suppress this.
        ));
flatspec.transmap_filename = optarg ;
break ;
#endif

OPTION('b',--background,(color) select background color,
       (Use this color for transparent pixels in the image.
        The color can be given as
        .B #rrggbb
        or
        .B #rgb
        hexadecimal values,
        or as an X11 color name that will be looked up in
        .BR /usr/lib/X11/rgb.txt .
        ));
{
  unsigned r,g,b ;
  unsigned long hex ;
  int met = 0 ;
  if( *optarg == '#' )
    sscanf(optarg+1,"%lx%n",&hex,&met);
  if( met == 3 && strlen(optarg) == 4 ) {
    r = ((hex >> 8) & 0xF) * 0x11 ;
    g = ((hex >> 4) & 0xF) * 0x11 ;
    b = ((hex >> 0) & 0xF) * 0x11 ;
  } else if( met == 6 && strlen(optarg) == 7 ) {
    r = ((hex >> 16) & 0xFF) ;
    g = ((hex >> 8) & 0xFF) ;
    b = ((hex >> 0) & 0xFF) ;
  } else if( strcasecmp(optarg,"black") == 0 )
    r = g = b = 0 ;
  else if( strcasecmp(optarg,"white") == 0 )
    r = g = b = 255 ;
  else {
    const char filename[] = "/usr/lib/X11/rgb.txt" ;
    FILE *colortable = fopen(filename,"rt");
    if( colortable ) {
      int clen ;
      char colorbuf[80] ;
      do {
        if( !fgets(colorbuf,sizeof colorbuf,colortable) ) {
          r = (unsigned)-1 ;
          break ;
        }
        clen = strlen(colorbuf);
        while( clen && isspace(colorbuf[clen-1]) )
          clen-- ;
        colorbuf[clen] = '\0' ;
        clen = 0 ;
        sscanf(colorbuf," %u %u %u %n",&r,&g,&b,&clen);
      } while( clen == 0 || strcasecmp(colorbuf+clen,optarg) != 0 );
    } else {
      fprintf(stderr,_("Could not open color database file %s\n"),filename);
    }
  }
  if( r == (unsigned)-1 )
    FatalGeneric(20,_("Unknown background color '%s'"),optarg);
  flatspec.default_pixel = ((rgba)255 << ALPHA_SHIFT)
    + ((rgba)r << RED_SHIFT)
    + ((rgba)g << GREEN_SHIFT)
    + ((rgba)b << BLUE_SHIFT);
  break ;
}

OPTION('A',--force-alpha,force alpha channel in output,
       (Invent a trivial alpha channel even if the flattened image is
        completely opaque.
        ));
flatspec.default_pixel = FORCE_ALPHA_CHANNEL ;
break ;

OPTION('c',--color --colour,select color output,
       (Force the output to use RGB color space even if it could be
        represented.
#ifdef XCF2PNM
        This will be selected automatically if the output file''s name
        ends with
        .BR .ppm .
#endif
       ));
flatspec.out_color_mode = COLOR_RGB ;
break ;

#if 0
OPTION('i',--indexed,select indexed output,
       (Force the output to used indexed color
        .I with the palette of the XCF file.
        If colors outside of the XCF file''s palette are produced
        during flattening, exit with status XXX.
        Beware that in PNG (unlike XCF) palette entries include
        transparency information, so if the flattened image contains
        any partial transparency, an error with status XXX will
        result. (On the other hand,
        .B xcf2png
        will artificially add a palette entry for fully transparent
        pixels or the the background color specified by
        .BR \-b ,
        if there is room in the palette).
        ));
flatspec.out_color_mode = COLOR_INDEXED ;
break ;
#endif

OPTION('g',--gray --grey,select grayscale output,
       (Force the output to be a grayscale image even if it may be monochrome.
        If any colored pixels are encountered, exit with status 103.
        This will be selected automatically if the output file''s name
        ends with
        .BR .pgm .
        ));
flatspec.out_color_mode = COLOR_GRAY ;
break ;

#ifdef XCF2PNM
OPTION('m',--mono,select monochrome output,
       (Force the output to be a monochrome image.
        If any colors except black and white are encountered, exit with
        status 103.
        This will be selected automatically if the output file''s name
        ends with
        .BR .pbm .
        ));
flatspec.out_color_mode = COLOR_MONO ;
break ;
#endif

#ifdef XCF2PNM
OPTION('n',--pnm,select -c/-g/-m by image contents,
       (Suppress the automatic choice of
        .BR \-c ,
        .BR \-g ,
        or
        .BR \-m
        based on output filename, and instead select the output format
        based on image contents. This is the default if the filename
        is not recognized, and when writing to stdout.
        ));
flatspec.out_color_mode = COLOR_BY_CONTENTS ;
break ;
#endif

OPTION('T',--truecolor,treat indexed images as RGB for flattening,
       (Use standard RGB compositing for flattening indexed layers.
        Without this option
        .B THISPROGRAM
        will mimic the Gimp''s current strategy of rounding each
        alpha value to either full transparency or full opacity,
        and interpret all layer modes as
        .B Normal .
        ));
flatspec.gimpish_indexed = 0 ;
break ;

OPTION('G',--for-gif,disallow partial transparency,
       (Assert that the flattened image will have no partial transparency
        (allowing a more compact representation of the alpha output).
        Exit with status 102 if the flattened image has any partial
        transparency.
        If
        .B \-b
        is also given, exit with status 102 if there is partial transparency
        before applying the background color.
        ));
flatspec.partial_transparency_mode = FORBID_PARTIAL_TRANSPARENCY ;
break ;

OPTION('D',--dissolve,dissolve partial transparency,
       (Do a "dissolve" step to eliminate partial transparency after
        flattening.
        If
        .B \-b
        is also given, this happens before the background color is applied.
        ));
flatspec.partial_transparency_mode = DISSOLVE_PARTIAL_TRANSPARENCY ;
break ;

OPTION('f',--full-image,flatten to memory; then analyse,
       (First flatten the entire image to a memory buffer before writing
        output. Then analyse the image to decide on the details of the
        output format (e.g., whether a grayscale output is sufficient).
        Without this option, the program flattens only a singe row of "tiles"
        (height 64) at a time.
        ));
flatspec.process_in_memory = 1 ;
break ;

OPTION('S',--size,(w"x"h) crop image while converting,
       (Crop the converted image to width \fIw\fP and height \fIh\fP.
        ));
{
  unsigned w,h ;
  int n = 0 ;
  sscanf(optarg,"%ux%u%n",&w,&h,&n) ;
  if( n && n == strlen(optarg) ) {
    flatspec.dim.width = w ;
    flatspec.dim.height = h ;
  } else
    FatalGeneric(20,_("-S option must have an argument of the form wxh"));
  break ;
}

OPTION('O',--offset,(x","y) translate converted part of image,
       (Offset the converted part of the image from the top-left corner
        of the XCF canvas. Usually used with
        .BR \-S .
        ));
{
  int x,y ;
  int n = 0 ;
  sscanf(optarg,"%d,%d%n",&x,&y,&n) ;
  if( n && n == strlen(optarg) ) {
    flatspec.dim.c.l = x ;
    flatspec.dim.c.t = y ;
  } else
    FatalGeneric(20,_("-O option must have an argument of the form x,y"));
  break ;
}

OPTIONGROUP(1il,Layer-selection options);

OPTION(300,--mode,(mode) set layer mode,
       (Set the layer mode (e.g.,
        .B Normal
        or
        .BR Multiply ).
        ));
{
  GimpLayerModeEffects m ;
  for( m = 0; strcmp(optarg,showGimpLayerModeEffects(m)) != 0 ; m++ ) {
    if( m > GimpLayerModeEffects_LAST )
      FatalGeneric(20,_("Layer mode '%s' is unknown"),optarg);
  }
  lastlayerspec(&flatspec,"--mode")->mode = m ;
  break ;
}

OPTION(301,--percent,(n) set opacity in percent,
       (Set the opacity on a scale from 0 to 100
        (as in the Gimp user interface).
        ));
{
  unsigned pct ;
  int n ;
  sscanf(optarg,"%u%n",&pct,&n) ;
  if( n != strlen(optarg) || pct > 100 )
    FatalGeneric(20,_("The argument to --pct is not a percentage"));
  lastlayerspec(&flatspec,"--percent")->opacity = pct * 255 / 100 ;
  break ;
}

OPTION(302,--opacity,(n) set opacity in 1/255 units,
       (Set the opacity on a scale from 0 to 255 (as used internally)
        ));
{
  unsigned alpha ;
  int n ;
  sscanf(optarg,"%u%n",&alpha,&n) ;
  if( n != strlen(optarg) || alpha > 255 )
    FatalGeneric(20,_("The argument to --opacity is not a number "
                     "between 0 and 255"));
  lastlayerspec(&flatspec,"--percent")->opacity = alpha ;
  break ;
}

OPTION(303,--mask,enable layer mask,
       (Enable the layer mask.
        ));
lastlayerspec(&flatspec,"--mask")->hasMask = 1 ;
break ;

OPTION(304,--nomask,disable layer mask,
       (Disable the layer mask.
        ));
lastlayerspec(&flatspec,"--nomask")->hasMask = 0 ;
break ;

#endif /* XCF2FOO */

OPTIONGROUP(1i,)

#if HAVE_ICONV
OPTION('u',--utf8,use UTF-8 for layer names,
       (Use the raw UTF-8 representation from the XCF file to compare
        and display layer names.
        Ordinarily, layer names will be converted to the character set
        of the current locale.
        ));
use_utf8 = 1 ;
break ;
#endif
