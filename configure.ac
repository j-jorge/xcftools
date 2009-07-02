dnl Configure script for xcftools
dnl
dnl This file was written by Henning Makholm <henning@makholm.net>
dnl It is hereby in the public domain.
dnl 
dnl In jurisdictions that do not recognise grants of copyright to the
dnl public domain: I, the author and (presumably, in those jurisdictions)
dnl copyright holder, hereby permit anyone to distribute and use this code,
dnl in source code or binary form, with or without modifications. This
dnl permission is world-wide and irrevocable.
dnl
dnl Of course, I will not be liable for any errors or shortcomings in the
dnl  code, since I give it away without asking any compenstations.
dnl
dnl If you use or distribute this code, I would appreciate receiving
dnl credit for writing it, in whichever way you find proper and customary.
dnl
dnl ------------------------
dnl The following line is the MASTER SOURCE for the version number
AC_INIT(Xcftools,1.0.7,henning@makholm.net,xcftools)
AC_CONFIG_SRCDIR(xcfinfo.c)
        
AC_PROG_INSTALL
AC_ARG_PROGRAM

AC_ARG_ENABLE(precomputed-scaletable,
        AC_HELP_STRING([--enable-precomputed-scaletable],
          [Use precomputed multiplication table (adds 64 KB per binary)]),
        [AC_DEFINE(PRECOMPUTED_SCALETABLE,1,
   [Define to use a precomputed multiplication table (adds 64 KB per binary)]
           )])

AC_PROG_CC
case $GCC::$CFLAGS in yes::*-Wall* ) ;;
                      yes::*) CFLAGS="$CFLAGS -Wall" ;;
esac
case $GCC::$CFLAGS in *-g* | *omit-frame-pointer* ) ;;
                      yes::*) CFLAGS="$CFLAGS -fomit-frame-pointer" ;;
esac
AC_C_INLINE

AC_C_BIGENDIAN
AC_CANONICAL_HOST
case $host_cpu in i?86 | x86_64 | powerpc)
      AC_DEFINE(CAN_DO_UNALIGNED_WORDS,1,
       [Define for CPUs that can read unaligned words without traps or faults])
esac
AC_CHECK_HEADERS(inttypes.h netinet/in.h arpa/inet.h getopt.h)

AC_CHECK_FUNCS(getopt_long strcasecmp)
AC_FUNC_MMAP
AM_GNU_GETTEXT(external)
AM_ICONV

AC_PATH_PROG(PERL,perl,perl)
                        
AC_CONFIG_HEADERS(config.h:config.hin)
AC_CONFIG_FILES(Makefile:Makefile.in)
AC_OUTPUT
