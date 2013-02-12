/* A program that extracts metadata from an XCF file
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

#include "xcftools.h"
#include "nlsini.h"
#include "options.h"

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <getopt.h>

/*----------------------------------------------------------------------------*/
const struct option long_options[] = {
  option_help,
  option_version,
  option_verbose,
  option_bzip,
  option_gzip,
  option_unpack,
  option_path_separator,
  option_utf8,
  { 0 }
};

/*----------------------------------------------------------------------------*/
const char* const short_options = short_options_prefix
  short_option_help
  short_option_version
  short_option_verbose
  short_option_bzip
  short_option_gzip
  short_option_unpack
  short_option_path_separator
  short_option_utf8
  ;

static void
printLayerPath
( unsigned layerIndex, const char* pathSeparator )
{
  int depth = XCF.layers[layerIndex].pathLength ;
  int i = layerIndex;

  if ( depth != 0 ) {
    do {
      i++;
    } while ( XCF.layers[i].pathLength != depth - 1 );

    printLayerPath( i, pathSeparator ) ;
    printf( "%s%s", pathSeparator, XCF.layers[i].name );
  }
}

int
main(int argc,char **argv)
{
  int i ;

  struct ProcessControl process;

  setlocale(LC_ALL,"");
  progname = argv[0] ;
  nls_init();

  if( argc <= 1 ) gpl_blurb() ;

  init_process_control( &process );

  if ( option_parse
       ( argc, argv, short_options, long_options, &process, NULL ) )
    exit(1);

  // set the global flags
  verboseFlag = process.verboseFlag;
  use_utf8 = process.use_utf8;

  read_or_mmap_xcf( process.inputFile, process.unzipper );
  getBasicXcfInfo() ;

  printf(gettext("Version %d, %dx%d %s, %d layers, compressed %s\n"),
         XCF.version,XCF.width,XCF.height,
         gettext(showGimpImageBaseType(XCF.type)),
         XCF.numLayers,
         gettext(showXcfCompressionType(XCF.compression)));

  for( i = XCF.numLayers ; i-- ; ) {
    printf("%c %dx%d%+d%+d %s %s",
           XCF.layers[i].isVisible ? '+' : '-',
           XCF.layers[i].dim.width, XCF.layers[i].dim.height,
           XCF.layers[i].dim.c.l, XCF.layers[i].dim.c.t,
           gettext(showGimpImageType(XCF.layers[i].type)),
           gettext(showGimpLayerModeEffects(XCF.layers[i].mode)));
    if( XCF.layers[i].opacity < 255 )
      printf("/%02d%%",XCF.layers[i].opacity * 100 / 255);
    if( XCF.layers[i].hasMask )
      printf(gettext("/mask"));
    if( XCF.layers[i].isGroup )
      printf(gettext("/group"));

    printf( " " );

    if ( XCF.version > 2 ) {
      printLayerPath( i, process.pathSeparator );
      printf( "%s", process.pathSeparator );
    }

    printf("%s\n",XCF.layers[i].name);
  }
      
  return 0 ;
}
