#! /usr/bin/perl
# This script extracts translatable manpage fragments from
# option.i
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

my $masterfile = "options.i" ;

open OPTIONS, "<", $masterfile or die "Cannot open $masterfile" ;

while( <OPTIONS> ) {
    if( /OPTION\(([^,]*),([^,]*),(.*),\s*$/ ) {
        my ($option,$long,$help) = ($1,$2,$3) ;
        if( $help =~ s/^\s*\(([^()]+)\)\s*// ) {
            my $arg = $1 ;
            $arg =~ s/"([^\"]*)"/\\fB$1\\fP/g ;
            print "#line $. \"$masterfile\"\n.TP\n" ;
            print "\\fB\\-X\\fR \\fI$arg\\fR\n" ;
        }
        $_ = <OPTIONS> ;
        s/^\s*\(// ;
        print "#line $. \"$masterfile\"\n$_" ;
        while( <OPTIONS> ) {
            last if /^\s*\)\);/ ;
            s/^\s*// ;
            s/''/'/g ;
            print $_ ;
        }
    }
}

close OPTIONS ;
