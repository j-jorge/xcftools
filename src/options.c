/* Option processing for xcftools -*- C -*-
 *
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
#include "options.h"

#include "version.h"

#include <string.h>

/*----------------------------------------------------------------------------*/
void option_show_version( const char* app_name )
{
  printf( "%s - %s\n", app_name, PACKAGE_STRING );
} // option_show_version()

/*----------------------------------------------------------------------------*/
void option_set_verbose( struct ProcessControl* p )
{
  p->verboseFlag = 1;
} // option_set_verbose()

/*----------------------------------------------------------------------------*/
void option_set_bzip( struct ProcessControl* p )
{
  p->unzipper = "bzcat";
} // option_set_bzip()

/*----------------------------------------------------------------------------*/
void option_set_gzip( struct ProcessControl* p )
{
  p->unzipper = "zcat";
} // option_set_gzip()
     
/*----------------------------------------------------------------------------*/
void option_set_unpack( struct ProcessControl* p, char* command )
{
  p->unzipper = command;
} // option_set_unpack()

/*----------------------------------------------------------------------------*/
void option_set_path_separator( struct ProcessControl* p, char* separator )
{
  p->pathSeparator = separator;
} // option_set_path_separator()

/*----------------------------------------------------------------------------*/
void option_set_utf8( struct ProcessControl* p )
{
  p->use_utf8 = 1;
} // option_set_utf8()

/*----------------------------------------------------------------------------*/
void option_set_output( struct FlattenSpec* flatspec, char* filename )
{
  flatspec->output_filename = filename;
} // option_set_output()

/*----------------------------------------------------------------------------*/
void option_set_alpha( struct FlattenSpec* flatspec, char* filename )
{
  flatspec->transmap_filename = filename;
} // option_set_alpha()

/*----------------------------------------------------------------------------*/
void option_set_background( struct FlattenSpec* flatspec, char* color )
{
  unsigned r,g,b ;
  unsigned long hex ;
  int met = 0 ;
  if( color[0] == '#' )
    sscanf(color+1,"%lx%n",&hex,&met);

  if( (met == 3) && (strlen(color) == 4) ) {
    r = ((hex >> 8) & 0xF) * 0x11 ;
    g = ((hex >> 4) & 0xF) * 0x11 ;
    b = ((hex >> 0) & 0xF) * 0x11 ;
  } else if( met == 6 && strlen(color) == 7 ) {
    r = ((hex >> 16) & 0xFF) ;
    g = ((hex >> 8) & 0xFF) ;
    b = ((hex >> 0) & 0xFF) ;
  } else if( strcasecmp(color,"black") == 0 )
    r = g = b = 0 ;
  else if( strcasecmp(color,"white") == 0 )
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
        } while( clen == 0 || strcasecmp(colorbuf+clen,color) != 0 );
        fclose(colortable) ;
      }
    }
    if( !any ) {
      fprintf( stderr, gettext("Could not find X11 color database\n") );
    }
  }

  if( r == (unsigned)-1 )
    FatalGeneric
      ( 20, gettext("Unknown background color '%s'"), color );

  flatspec->default_pixel = ((rgba)255 << ALPHA_SHIFT)
    + ((rgba)r << RED_SHIFT)
    + ((rgba)g << GREEN_SHIFT)
    + ((rgba)b << BLUE_SHIFT);
} // option_set_background()

/*----------------------------------------------------------------------------*/
void option_set_checkered_background
( struct FlattenSpec* flatspec )
{
  flatspec->default_pixel = CHECKERED_BACKGROUND;
} // option_set_checkered_background()

/*----------------------------------------------------------------------------*/
void option_set_force_alpha( struct FlattenSpec* flatspec )
{
  flatspec->default_pixel = FORCE_ALPHA_CHANNEL ;
} // option_set_force_alpha()

/*----------------------------------------------------------------------------*/
void option_set_color( struct FlattenSpec* flatspec )
{
  flatspec->out_color_mode = COLOR_RGB ;
} // option_set_color()

/*----------------------------------------------------------------------------*/
void option_set_gray( struct FlattenSpec* flatspec )
{
  flatspec->out_color_mode = COLOR_GRAY ;
} // option_set_gray()

/*----------------------------------------------------------------------------*/
void option_set_mono( struct FlattenSpec* flatspec )
{
  flatspec->out_color_mode = COLOR_MONO ;
} // option_set_mono()

/*----------------------------------------------------------------------------*/
void option_set_pnm( struct FlattenSpec* flatspec )
{
  flatspec->out_color_mode = COLOR_BY_CONTENTS ;
} // option_set_pnm()

/*----------------------------------------------------------------------------*/
void option_set_truecolor( struct FlattenSpec* flatspec )
{
  flatspec->gimpish_indexed = 0 ;
} // option_set_truecolor()

/*----------------------------------------------------------------------------*/
void option_set_for_gif( struct FlattenSpec* flatspec )
{
  flatspec->partial_transparency_mode = FORBID_PARTIAL_TRANSPARENCY ;
} // option_set_for_gif()

/*----------------------------------------------------------------------------*/
void option_set_dissolve( struct FlattenSpec* flatspec )
{
  flatspec->partial_transparency_mode = DISSOLVE_PARTIAL_TRANSPARENCY ;
} // option_set_dissolve()

/*----------------------------------------------------------------------------*/
void option_set_full_image( struct FlattenSpec* flatspec )
{
  flatspec->process_in_memory = 1 ;
} // option_set_full_image()

/*----------------------------------------------------------------------------*/
void option_set_size( struct FlattenSpec* flatspec, char* size )
{
  unsigned w, h;
  int n = 0 ;

  sscanf( size,"%ux%u%n", &w, &h, &n );

  if( ( n != 0 ) && ( n == strlen(size) ) )
    {
      if( flatspec->window_mode == AUTOCROP )
        flatspec->window_mode = USE_CANVAS ;

      flatspec->window_mode |= MANUAL_CROP ;
      flatspec->dim.width = w ;
      flatspec->dim.height = h ;
    }
  else
    FatalGeneric( 20, gettext("The size must have the form WxH") );
} // option_set_size()

/*----------------------------------------------------------------------------*/
void option_set_offset( struct FlattenSpec* flatspec, char* offset )
{
  int x, y;
  int n = 0;

  sscanf( offset, "%d,%d%n", &x, &y, &n );

  if( ( n != 0 ) && ( n == strlen(offset) ) )
    {
      if( flatspec->window_mode == AUTOCROP )
        flatspec->window_mode = USE_CANVAS ;

      flatspec->window_mode |= MANUAL_OFFSET ;
      flatspec->dim.c.l = x ;
      flatspec->dim.c.t = y ;
    }
  else
    FatalGeneric( 20, gettext("The offset must have the form x,y") );
} // option_set_offset()

/*----------------------------------------------------------------------------*/
void option_set_autocrop( struct FlattenSpec* flatspec )
{
  flatspec->window_mode = AUTOCROP ;
} // option_set_autocrop()

/*----------------------------------------------------------------------------*/
void option_set_mode( struct FlattenSpec* flatspec, char* mode )
{
  GimpLayerModeEffects m ;
  int found = 1;

  if( flatspec->numLayers == 0 )
    FatalGeneric
      ( 20,
        gettext("There is no layer to which the mode can be applied") );

  for( m = 0; (found == 0) && (m < GimpLayerModeEffects_LAST); m++ )
    if( strcmp( mode, gettext(showGimpLayerModeEffects(m)) ) == 0 )
      found = 1;
    
  for( m = 0; (found == 0) && (m < GimpLayerModeEffects_LAST); m++ )
    if( strcmp( mode, showGimpLayerModeEffects(m) ) == 0 )
      found = 1;

  if ( found )
    lastlayerspec( flatspec )->mode = m;
  else
    FatalGeneric( 20, gettext("Layer mode '%s' is unknown"), mode );
} // option_set_mode()

/*----------------------------------------------------------------------------*/
void option_set_percent( struct FlattenSpec* flatspec, char* percent )
{
  unsigned pct ;
  int n ;

  if( flatspec->numLayers == 0 )
    FatalGeneric
      ( 20,
        gettext("There is no layer to which the percent can be applied") );

  sscanf( percent, "%u%n", &pct, &n );

  if( ( n != strlen(percent) ) || ( pct > 100 ) )
    FatalGeneric( 20, gettext("The percent value is not a percentage") );
  
  lastlayerspec( flatspec )->opacity = pct * 255 / 100;
} // option_set_percent()

/*----------------------------------------------------------------------------*/
void option_set_opacity( struct FlattenSpec* flatspec, char* opacity )
{
  unsigned alpha ;
  int n ;

  if( flatspec->numLayers == 0 )
    FatalGeneric
      ( 20,
        gettext("There is no layer to which the opacity can be applied") );

  sscanf( opacity, "%u%n", &alpha, &n );

  if( ( n != strlen(opacity) ) || ( alpha > 255 ) )
    FatalGeneric
      ( 20, gettext("The opacity is not a number between 0 and 255") );

  lastlayerspec( flatspec )->opacity = alpha ;
} // option_set_opacity()

/*----------------------------------------------------------------------------*/
void option_set_mask( struct FlattenSpec* flatspec )
{
  if( flatspec->numLayers == 0 )
    FatalGeneric
      ( 20,
        gettext("There is no layer on which the mask can be enabled") );

  lastlayerspec( flatspec )->hasMask = 1;
} // option_set_mask()

/*----------------------------------------------------------------------------*/
void option_set_nomask( struct FlattenSpec* flatspec )
{
  if( flatspec->numLayers == 0 )
    FatalGeneric
      ( 20,
        gettext("There is no layer on which the mask can be disabled") );

  lastlayerspec( flatspec )->hasMask = 0;
} // option_set_nomask()

/*----------------------------------------------------------------------------*/
int process_option
( int option, char* argval,
  struct ProcessControl* p, struct FlattenSpec* flatspec )
{
  switch(option)
    {
    case option_help_value:
    case '?':
      return -1;
    case option_version_value:
      return 1;
    case option_verbose_value:
      option_set_verbose( p );
      break;
    case option_bzip_value:
      option_set_bzip( p );
      break;
    case option_gzip_value:
      option_set_gzip( p );
      break;
    case option_unpack_value:
      option_set_unpack( p, argval );
      break;
    case option_output_value:
      option_set_output( flatspec, argval );
      break;
    case option_path_separator_value:
      option_set_path_separator( p, argval );
      break;
    case option_alpha_value:
      option_set_alpha( flatspec, argval );
      break;
    case option_background_value:
      option_set_background( flatspec, argval );
      break;
    case option_checkered_background_value:
      option_set_checkered_background( flatspec );
      break;
    case option_force_alpha_value:
      option_set_force_alpha( flatspec );
      break;
    case option_color_value:
      option_set_color( flatspec );
      break;
    case option_gray_value:
      option_set_gray( flatspec );
      break;
    case option_mono_value:
      option_set_mono( flatspec );
      break;
    case option_pnm_value:
      option_set_pnm( flatspec );
      break;
    case option_truecolor_value:
      option_set_truecolor( flatspec );
      break;
    case option_for_gif_value:
      option_set_for_gif( flatspec );
      break;
    case option_dissolve_value:
      option_set_dissolve( flatspec );
      break;
    case option_full_image_value:
      option_set_full_image( flatspec );
      break;
    case option_size_value:
      option_set_size( flatspec, argval );
      break;
    case option_offset_value:
      option_set_offset( flatspec, argval );
      break;
    case option_autocrop_value:
      option_set_autocrop( flatspec );
      break;
    case option_mode_value:
      option_set_mode( flatspec, argval );
      break;
    case option_percent_value:
      option_set_percent( flatspec, argval );
      break;
    case option_opacity_value:
      option_set_opacity( flatspec, argval );
      break;
    case option_mask_value:
      option_set_mask( flatspec );
      break;
    case option_nomask_value:
      option_set_nomask( flatspec );
      break;
    case option_utf8_value:
      option_set_utf8( p );
      break;

    case 1:
      if ( p->inputFile == NULL )
        p->inputFile = argval;
      else
        add_layer_request( flatspec, optarg );
      break ;
    default:
      FatalUnexpected("Getopt(_long) unexpectedly returned '%c'", option);
    }

  return 0;
} // process_option()

/*----------------------------------------------------------------------------*/
int get_option_index( const struct option* long_options, int option )
{
  int i;

  for ( i=0; long_options[i].name != NULL; ++i )
    if ( long_options[i].val == option )
      return i;

  return -1;
} // get_option_index()

/*----------------------------------------------------------------------------*/
int has_layer_option( const struct option* long_options )
{
  return (get_option_index( long_options, option_mode_value ) != -1)
    || (get_option_index( long_options, option_percent_value ) != -1)
    || (get_option_index( long_options, option_opacity_value ) != -1)
    || (get_option_index( long_options, option_mask_value ) != -1)
    || (get_option_index( long_options, option_nomask_value ) != -1)
    || (get_option_index( long_options, option_path_separator_value ) != -1)
    || (get_option_index( long_options, option_utf8_value ) != -1);
} // has_layer_option()

/*----------------------------------------------------------------------------*/
void print_option_arg
( const struct option* long_options, int option_value, const char* message,
  const char* command )
{
  const int index = get_option_index( long_options, option_value );
  const struct option* option;
  int fill = 30;

  if ( index < 0 )
    return;
  else
    option = &long_options[index];

  fill -= printf( "  " );

  if ( isprint( long_options[index].val ) )
    fill -= printf( "-%c, ", (char)option->val );

  fill -= printf( "--%s", option->name );

  if ( command != NULL )
    fill -= printf( "=%s", command );

  if ( fill > 0 )
    printf( "%*.*s%s\n", fill, fill, " ", message );
  else
  printf( " %s\n", message );
} // print_option_arg()

/*----------------------------------------------------------------------------*/
void print_option
( const struct option* long_options, int option_value, const char* message )
{
  print_option_arg( long_options, option_value, message, NULL );
} // print_option()

/*----------------------------------------------------------------------------*/
void show_help
( const char* progname, const struct option* long_options )
{
  const int with_layer_option = has_layer_option( long_options );
  if ( with_layer_option )
    printf( gettext( "Usage: %s [options] filename.xcf[.gz | .bz2] [layers]\n"),
            progname );
  else
    printf( gettext( "Usage: %s [options] filename.xcf[.gz | .bz2]\n"),
            progname );

  printf( gettext( "Options:\n" ) );

  print_option( long_options, option_help_value, gettext("show this message") );
  print_option( long_options, option_version_value, gettext("show version") );
  print_option
    ( long_options, option_verbose_value, gettext("show progress messages") );
  print_option
    ( long_options, option_bzip_value, gettext("input is bzip2 compressed") );
  print_option
    ( long_options, option_gzip_value, gettext("input is gzip compressed") );
  print_option_arg
    ( long_options, option_unpack_value,
      gettext("use 'command' to decompress input"), "command" );
  print_option_arg
    ( long_options, option_output_value, gettext("name output file"),
      "filename" );
  print_option_arg
    ( long_options, option_alpha_value, gettext("write transparency map"),
      "filename" );
  print_option_arg
    ( long_options, option_background_value,
      gettext("select background color"), "color" );
  print_option
    ( long_options, option_checkered_background_value,
      gettext("render on a checkered background") );
  print_option
    ( long_options, option_force_alpha_value,
      gettext("force alpha channel in output") );
  print_option
    ( long_options, option_color_value, gettext("select color output") );
  print_option
    ( long_options, option_gray_value, gettext("select grayscale output") );
  print_option
    ( long_options, option_mono_value,
      gettext("select monochrome output") );
  print_option
    ( long_options, option_pnm_value,
      gettext("select --color, --gray or --mono by image content") );
  print_option
    ( long_options, option_truecolor_value,
      gettext("treat indexed images as RGB for flattening") );
  print_option
    ( long_options, option_for_gif_value,
      gettext("disallow partiel transparency") );
  print_option
    ( long_options, option_dissolve_value,
      gettext("dissolve partial transparency") );
  print_option
    ( long_options, option_full_image_value,
      gettext("flatten to memory; then analyse") );
  print_option_arg
    ( long_options, option_size_value, gettext("crop image while converting"),
      "WxH" );
  print_option_arg
    ( long_options, option_offset_value,
      gettext("translate converted part of image"), "x,y" );
  print_option
    ( long_options, option_autocrop_value,
      gettext("autocrop to visible layer boundaries") );

  if ( with_layer_option )
    {
      printf( gettext( "Layer-selection options:\n" ) );

      print_option_arg
        ( long_options, option_mode_value, gettext("set layer mode"), "mode" );
      print_option_arg
        ( long_options, option_percent_value,
          gettext("set opacity in percent"), "n" );
      print_option_arg
        ( long_options, option_opacity_value,
          gettext("set opacity in 1/255 units"), "n" );
      print_option
        ( long_options, option_mask_value, gettext("enable layer mask") );
      print_option
        ( long_options, option_nomask_value, gettext("disable layer mask") );
      print_option_arg
        ( long_options, option_path_separator_value,
          gettext("use 'string' to separate the groups in the paths"),
          "string" );
      print_option
        ( long_options, option_utf8_value,
          gettext("use UTF-8 for layer names") );
    }
} // show_help()

/*----------------------------------------------------------------------------*/
int option_parse
( int argc, char** argv, const char* short_options,
  const struct option* long_options, struct ProcessControl* p,
  struct FlattenSpec* flatspec )
{
  int option;
  const char* progname = argv[0];
  int option_result = 0;

  option = getopt_long( argc, argv, short_options, long_options, NULL );

  while ( (option != -1) && (option_result == 0) )
    {
      option_result = process_option( option, optarg, p, flatspec );
      option = getopt_long( argc, argv, short_options, long_options, NULL );
    }

  if ( option_result == 1 )
    option_show_version( progname );
  else if ( ( p->inputFile == NULL ) || ( option_result < 0 ) )
    show_help( progname, long_options );

  return option_result != 0;
} // option_parse()
