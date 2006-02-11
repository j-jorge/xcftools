dnl Configure script for xcftools
dnl
dnl Copyright (C) 2006  Henning Makholm
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of version 2 of the GNU General Public License as
dnl published by the Free Software Foundation.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
dnl
dnl ------------------------
dnl The following line is the MASTER SOURCE for the version number
AC_INIT(Xcftools,0.9,henning@makholm.net,xcftools)
AC_COPYRIGHT(General copyright notice for AC_PACKAGE_NAME:
   [@COPYRIGHT@  Henning Makholm]
)
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
dnl AM_GNU_GETTEXT(external)
AM_ICONV

AC_CHECK_PROG(PERL,perl,perl)
                        
AC_CONFIG_HEADERS(config.h:config.hin)
AC_CONFIG_FILES(Makefile:Makefile.in)
AC_OUTPUT
