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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

use strict ; use warnings ;

if( @ARGV != 1 ) {
    print STDERR "Usage: $0 infile.10\n" ;
    exit 1 ;
}

my %defs ;

sub copyfile($);
sub copyfile($) {
    my ($fn) = @_ ;
    local *FILE ;
    open FILE, "<", $fn or die "Cannot read $fn" ;
    my $ignore = 0 ;
    while( <FILE> ) {
        if( /^\#else/ ) {
            if( $ignore ) {
                $ignore-- ;
            }  else {
                $ignore = 1 ;
            }
        } elsif( /^\#endif/ ) {
            $ignore-- if $ignore ;
            print ".\\\"---\n" unless $ignore ;
        } elsif( $ignore ) {
            if( /^\#if/ ) {
                $ignore++ ;
            }
        } elsif( /^\#ifdef\s+(\S+)/ ) {
            print ".\\\"---\n" ;
            $ignore = 1 unless $defs{$1} ;
        } elsif( /^\s*.so\s*(.*)/ ) {
            copyfile($1) ;
        } else {
            print ;
        }
    }
}


if( open CONFIG, "<", "config.h" ) {
    while( <CONFIG> ) {
        if( /^#define\s+(\S*)/ ) {
            $defs{$1} = 1 ;
        }
    }
    close CONFIG ;
}

my $fn0 = $ARGV[0] ;
$fn0 =~ s/\.\d*$// ;
$defs{"\U$fn0"} = 1 ;
$defs{"XCF2FOO"} = 1 if $fn0 =~ /^xcf2/ ;

copyfile($ARGV[0]) ;
