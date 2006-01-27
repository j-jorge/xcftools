#! /usr/bin/perl
# This short script extracts enum definitions from files stolen
# from the Gimp's sources.
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

my @wantenums = qw( GimpImageBaseType
                    GimpImageType
                    GimpLayerModeEffects
                    PropType
                    XcfCompressionType
                    );
my %wantenums ;
@wantenums{@wantenums} = (-1) x @wantenums ;

my $last ;
my @collect ;
print join("\n *   ","/* Extracted from",@ARGV),"\n * by $0\n */\n" ;
while( <> ) {
    if( /^\s*typedef\s+enum\s/ ) {
        @collect = ($_) ;
    } elsif( /^\}\s+(\w+)\s*;/ && @collect ) {
        my $enum = $1 ;
        if( ++$wantenums{$enum} == 0 ) {
            if( $enum eq 'GimpLayerModeEffects' ) {
                push @collect, "  ,GIMP_NORMAL_NOPARTIAL_MODE=-1\n" ;
            }
            push @collect, $_ ;
            print @collect ;
            print "const char *show$enum($enum);\n" ;
            print "#define ${enum}_LAST $last\n" ;
        }
        @collect = () ;
    } elsif( @collect ) {
        push @collect, $_ ;
        $last = $1 if /^\s*(\w+)/ ;
    }
}
for my $enum ( @wantenums ) {
    my $count = 1 + $wantenums{$enum} ;
    if( $count != 1 ) {
        print STDERR "$count definitions of $enum\n" ;
    }
}
    
        
