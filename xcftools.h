/* Generic functions and macros for reading XCF files
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

#ifndef XCFTOOLS_H
#define XCFTOOLS_H

#include "config.h"
#include "enums.h"
#include <stddef.h>
#include <stdio.h>

/* Change to gettext for l10n */
#define _(s) (s)
#define N_(s) (s)

#if HAVE_INTTYPES_H
# define __STDC_FORMAT_MACROS
# include <inttypes.h>
#else
/* These legacy fall-backs will probably work on every system
 * that does not supply a inttypes.h ... */
typedef unsigned char     uint8_t ;
typedef unsigned long int uint32_t, uintptr_t ;
typedef signed char       int8_t ;
typedef signed long int   int32_t ;
# define PRIX32 "lX"
# define PRIu32 "lu"
# define PRIXPTR "lX"
#endif

#if __GNUC__
# define __ATTRIBUTE__ __attribute__
#else
# define __ATTRIBUTE__(x)
#endif

#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#elif HAVE_ARPA_INET_H
# include <arpa/inet.h>
#elif WORDS_BIGENDIAN
# define ntohl(x) (x)
#else
static inline uint32_t ntohl(uint32_t a) {
  return (a << 24) + ((a & 0xFF00) << 8) + ((a >> 8) & 0xFF00) + (a >> 24) ;
}
#endif

#ifndef HAVE_STRCASECMP
#define strcasecmp strcmp
#endif

/* Read a single word value from the XCF file */

/* Use + instead of | because that allows LEA instructions */
#define xcfBE(a) ( ((uint32_t)xcf_file[(a)  ] << 24) + \
                   ((uint32_t)xcf_file[(a)+1] << 16) + \
                   ((uint32_t)xcf_file[(a)+2] << 8 ) + \
                   ((uint32_t)xcf_file[(a)+3]      ) )
#define xcfLE(a) ( ((uint32_t)xcf_file[(a)  ]      ) + \
                   ((uint32_t)xcf_file[(a)+1] << 8 ) + \
                   ((uint32_t)xcf_file[(a)+2] << 16) + \
                   ((uint32_t)xcf_file[(a)+3] << 24) )

#if CAN_DO_UNALIGNED_WORDS
# define xcfL(a) ntohl(*(uint32_t *)(xcf_file + (a)))
#else
# define xcfL(a) ((a) & 3 ? xcfBE(a) : ntohl(*(uint32_t *)(xcf_file + (a))))
#endif

/* ****************************************************************** */

/* The following are exported from am OS-specific source file;
 * io-unix.c on unixish systems.
 */
void read_or_mmap_xcf(const char* filename, const char *unzipper);
void free_or_close_xcf(void);

/* ****************************************************************** */
/* utils.c */

extern const char *progname ;
extern int verboseFlag ;

void *xcfmalloc(size_t size);
void xcffree(void*);

void FatalGeneric(int status,const char* format,...)
     __ATTRIBUTE__((format(printf,2,3),noreturn)) ;
void FatalUnexpected(const char* format,...)
     __ATTRIBUTE__((format(printf,1,2),noreturn)) ;
void FatalBadXCF(const char* format,...)
     __ATTRIBUTE__((format(printf,1,2),noreturn)) ;
void FatalUnsupportedXCF(const char* format,...)
     __ATTRIBUTE__((format(printf,1,2),noreturn)) ;

void gpl_blurb(void) __ATTRIBUTE__((noreturn));
     
FILE* openout(const char*);
void closeout(FILE *,const char*);

struct rect {
  int t, b, l, r ;
};

#define isSubrect(A,B) \
  ((A).l >= (B).l && (A).r <= (B).r && (A).t >= (B).t && (A).b <= (B).b)
#define disjointRects(A,B) \
  ((A).l >= (B).r || (A).r <= (B).l || (A).t >= (B).b || (A).b <= (B).t)

/* ****************************************************************** */
/* xcf-general.c */

extern uint8_t *xcf_file ;
extern size_t xcf_length ;
extern int use_utf8 ;

void xcfCheckspace(uint32_t addr,int spaceafter, const char *format,...)
     __ATTRIBUTE__((format(printf,3,4)));
uint32_t xcfOffset(uint32_t addr,int spaceafter);

int xcfNextprop(uint32_t *master,uint32_t *body);
const char* xcfString(uint32_t ptr,uint32_t *after);

/* These are hardcoded in the Gimp sources: */
#define TILE_WIDTH 64
#define TILE_HEIGHT 64

struct tileDimensions {
  struct rect c ;
  unsigned width, height ;
  unsigned tilesx, tilesy ;
  unsigned ntiles ;
};
/* computeDimensions assumes that width, height, c.l, and c.t are set */
void computeDimensions(struct tileDimensions *);

struct xcfTiles {
  const struct _convertParams *params ;
  uint32_t *tileptrs ;
  uint32_t hierarchy ;
};

struct xcfLayer {
  struct tileDimensions dim ;
  const char *name ;
  GimpLayerModeEffects mode ;
  GimpImageType type ;
  unsigned int opacity ;
  int isVisible, hasMask ;
  uint32_t propptr ;
  struct xcfTiles pixels ;
  struct xcfTiles mask ;
}; 

extern struct xcfImage {
  int version ;
  unsigned width, height ;
  GimpImageBaseType type ;
  XcfCompressionType compression ;
  int numLayers ;
  struct xcfLayer *layers ;
  uint32_t colormapptr ;
} XCF ;

void getBasicXcfInfo(void);

#endif /* XCFTOOLS_H */
