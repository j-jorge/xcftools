#! /usr/bin/perl
# This script compiles final manpages for xcftools
# 
# This file was written by Henning Makholm <henning@makholm.net>
# It is hereby in the public domain.
# 
# In jurisdictions that do not recognise grants of copyright to the
# public domain: I, the author and (presumably, in those jurisdictions)
# copyright holder, hereby permit anyone to distribute and use this code,
# in source code or binary form, with or without modifications. This
# permission is world-wide and irrevocable.
#
# Of course, I will not be liable for any errors or shortcomings in the
# code, since I give it away without asking any compenstations.
#
# If you use or distribute this code, I would appreciate receiving
# credit for writing it, in whichever way you find proper and customary.

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
