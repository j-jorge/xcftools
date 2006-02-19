#! /usr/bin/perl
# This script extracts translatable manpage fragments from
# option.i
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
