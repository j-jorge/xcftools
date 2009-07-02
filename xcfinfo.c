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
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#if HAVE_GETOPT_H
#include <getopt.h>
#else
#include <unistd.h>
#endif
#ifndef HAVE_GETOPT_LONG
#define getopt_long(argc,argv,optstring,l1,l2) getopt(argc,argv,optstring)
#endif

#include "xcfinfo.oi"

static void
usage(FILE *where)
{
  fprintf(where,_("Usage: %s [options] filename.xcf[.gz]\n"),progname) ;
  fprintf(where,_("Options:\n"));
  opt_usage(where) ;
  if( where == stderr ) {
    exit(1);
  }
}

int
main(int argc,char **argv)
{
  int i ;
  int option ;
  const char *unzipper = NULL ;
  const char *infile = NULL ;

  setlocale(LC_ALL,"");
  progname = argv[0] ;
  nls_init();

  if( argc <= 1 ) gpl_blurb() ;

  while( (option=getopt_long(argc,argv,"-"OPTSTRING,longopts,NULL)) >= 0 )
    switch(option) {
      #define OPTION(char,long,desc,man) case char:
      #include "options.i"
    case 1:
      if( infile ) {
        FatalGeneric
          (20,_("Only one XCF file per command line, please"));
      } else {
        infile = optarg ;
        break ;
      }
    case '?':
      usage(stderr);
    default:
      FatalUnexpected("Getopt(_long) unexpectedly returned '%c'",option);
    }
  if( infile == NULL ) {
    usage(stderr);
  }

  read_or_mmap_xcf(infile,unzipper);
  getBasicXcfInfo() ;
  printf(_("Version %d, %dx%d %s, %d layers, compressed %s\n"),
         XCF.version,XCF.width,XCF.height,
         _(showGimpImageBaseType(XCF.type)),
         XCF.numLayers,
         _(showXcfCompressionType(XCF.compression)));
  for( i = XCF.numLayers ; i-- ; ) {
    printf("%c %dx%d%+d%+d %s %s",
           XCF.layers[i].isVisible ? '+' : '-',
           XCF.layers[i].dim.width, XCF.layers[i].dim.height,
           XCF.layers[i].dim.c.l, XCF.layers[i].dim.c.t,
           _(showGimpImageType(XCF.layers[i].type)),
           _(showGimpLayerModeEffects(XCF.layers[i].mode)));
    if( XCF.layers[i].opacity < 255 )
      printf("/%02d%%",XCF.layers[i].opacity * 100 / 255);
    if( XCF.layers[i].hasMask )
      printf(_("/mask"));
    printf(" %s\n",XCF.layers[i].name);
  }
      
  return 0 ;
}
