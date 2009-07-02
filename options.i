/* Option processing for xcftools -*- C -*-
 *
 * This file was written by Henning Makholm <henning@makholm.net>
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
printf(OPTI_TARGET " - " PACKAGE_STRING "\n");
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
        .BR "\-Z zcat" .
        Default if the filename ends with
        .BR gz .
        ));
unzipper = "zcat" ;
break ;

OPTION('Z',--unpack,(command) use 'command' to decompress input,
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
        error message and exit status 101;
        use
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
        or as an X11 color name
        (which will only work if a color name database can be found
         in one of a number of standard locations).
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
    const char *filenames[] =  { "/etc/X11/rgb.txt",
                                 "/usr/lib/X11/rgb.txt",
                                 "/usr/share/X11/rgb.txt",
                                 NULL };
    const char **fnp ;
    r = (unsigned)-1 ;
    int any = 0 ;
    for( fnp = filenames; r == (unsigned)-1 && fnp && *fnp; fnp++ ) {
      FILE *colortable = fopen(*fnp,"rt");
      if( colortable ) {
        any = 1 ;
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
        fclose(colortable) ;
      }
    }
    if( !any ) {
      fprintf(stderr,_("Could not find X11 color database\n"));
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
       (Force the output to use RGB color space even if it there are
        more compact alternatives.
#ifdef XCF2PNM
        This will be selected automatically if the output file''s name
        ends with
        .BR .ppm .
#endif
       ));
flatspec.out_color_mode = COLOR_RGB ;
break ;

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
        based on image contents.
        This is the default if the filename is not recognized, and
        when writing to stdout.
        ));
flatspec.out_color_mode = COLOR_BY_CONTENTS ;
break ;
#endif

OPTION('T',--truecolor,treat indexed images as RGB for flattening,
       (Use standard RGB compositing for flattening indexed layers.
        Without this option,
        .B \*p
        will mimic the Gimp''s current strategy of rounding each
        alpha value to either full transparency or full opacity,
        and interpret all layer modes as
        .BR Normal .
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
        is also given, this tests whether there there is partial
        transparency before applying the background color.
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
    if( flatspec.window_mode == AUTOCROP ) flatspec.window_mode = USE_CANVAS ;
    flatspec.window_mode |= MANUAL_CROP ;
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
    if( flatspec.window_mode == AUTOCROP ) flatspec.window_mode = USE_CANVAS ;
    flatspec.window_mode |= MANUAL_OFFSET ;
    flatspec.dim.c.l = x ;
    flatspec.dim.c.t = y ;
  } else
    FatalGeneric(20,_("-O option must have an argument of the form x,y"));
  break ;
}

OPTION('C',--autocrop,autocrop to visible layer boundaries,
       (Set the converted part of the image such that it just include
        the boundaries of the visible (or selected) layers.
        This may make it either smaller or larger than the canvas,
        depending on the position and size of the visible layers.
        (Note that the
        .I contents
        of the layers is not taken into account when autocropping).
        .IP
        In the absence of options that specify otherwise, the converted
        image will cover the entire XCF canvas.
        ));
flatspec.window_mode = AUTOCROP ;
break ;

#ifndef XCFVIEW
OPTIONGROUP(1il,Layer-selection options);
#endif

OPTION(300,--mode,(mode) set layer mode,
       (Set the layer mode (e.g.,
        .B Normal
        or
        .BR Multiply ).
        ));
{
  GimpLayerModeEffects m ;
  #ifdef ENABLE_NLS
  for( m = 0; m < GimpLayerModeEffects_LAST; m++ )
    if( strcmp(optarg,_(showGimpLayerModeEffects(m))) == 0 )
      goto found_localized ;
  #endif
    
  for( m = 0; strcmp(optarg,showGimpLayerModeEffects(m)) != 0 ; m++ ) {
    if( m > GimpLayerModeEffects_LAST )
      FatalGeneric(20,_("Layer mode '%s' is unknown"),optarg);
  }
 found_localized:
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
    FatalGeneric(20,_("The argument to --percent is not a percentage"));
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
