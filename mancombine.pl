#! /usr/bin/perl
# This script compiles final manpages for xcftools
# Copyright (C) 2006  Henning Makholm
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

use strict ; use warnings ;

my %defs ;

if( open CONFIG, "<", "config.h" ) {
    while( <CONFIG> ) {
        if( /^#define\s+(\S*)/ ) {
            $defs{$1} = 1 ;
        }
    }
    close CONFIG ;
}

my $ignore = 0 ;

while( <> ) {
    if( /^\#else/ ) {
        if( $ignore ) {
            $ignore-- ;
        }  else {
            $ignore = 1 ;
        }
    } elsif( /^\#endif/ ) {
        $ignore-- if $ignore ;
    } elsif( $ignore ) {
        if( /^\#if/ ) {
            $ignore++ ;
        }
    } elsif( /^\#ifdef\s+(\S+)/ ) {
        $ignore = 1 unless $defs{$1} ;
    } elsif( /^\s*.so\s*(.*)/ ) {
        my $filename = $1 ;
        open IN, "<", $filename or die "Cannot read $filename" ;
        print <IN> ;
        close IN ;
    } else {
        print ;
    }
}
