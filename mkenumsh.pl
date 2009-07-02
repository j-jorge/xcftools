#! /usr/bin/perl
# This short script extracts enum definitions from files stolen
# from the Gimp's sources.
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
    
        
