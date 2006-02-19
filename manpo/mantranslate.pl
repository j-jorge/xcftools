#! /usr/bin/perl
# This script extracts translateable strings from a manpage and/or
# translates a manpage using gettext.
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

my ($gettext,$shipout) ;

sub fontexpand1($$) {
    my ($kinds,$text) = @_ ;
    return "\\f$kinds$text\\fP" if 1 == length $kinds ;
    my $out = "" ;
    my $last = "R" ;
    while( $text )  {
        $kinds =~ s/^(.)(.*)/$2$1/ ;
        $last = $1 ;
        if( $text =~ s/^([^\" ]+)\s*// ||
            $text =~ s/^"([^\"]+)"\s*// ) {
            $out .= "\\f$last$1" ;
        } else {
            die "Bad font-change tail '$text'" ;
        }
    }
    $out .= "\\fR" unless $last eq "R" ;
    return $out ;
}
            

sub ggettext($$$) {
    my ($filename,$lineno,$text) = @_ ;
    return $text if $text =~ /^\d*$/ ;
    $text =~ s/\\\".*//g ;
    $text =~ s/\\\*p/%s/g ;
    $text =~ s/([^-])\\(-[a-zA-Z])/$1$2/g ;
    $text =~ s/^\.([BIR]+)\s+(.*)$/ fontexpand1($1,$2) /mge ;
    my $fontcode = $text !~ /[{}]/ ;
    if( $fontcode ) {
        $text =~ s/\\f([BI])(([^\\]|\\[^f])*)\\f[RP]/$1\{$2\}/mg ;
    }
    my $newline = $text =~ s/\n$//s ;
    if( $text !~ /^[.\']/m || $text =~ /\n/ ) {
        $text =~ s/\s+/ /gs ;
        my $pre = "" ;
        while( length($text) > 65 ) {
            $text =~ s/^(.{1,65}) // or $text =~ s/^([^ ]*) // ;
            $pre .= "$1\n" ;
        }
        $text = "$pre$text" ;
        $text =~ s/\s*$// ;
    }
    $text = $gettext->($filename,$lineno,$text);
    $text .= "\n" if $newline ;
    if( $fontcode ) {
        $text =~ s/([BI])\{([^{}]*)\}/\\f$1$2\\fP/g ;
    }
    $text =~ s/([^-])(-[a-zA-Z])/$1\\$2/g ;
    $text =~ s/%s/\\*p/g ;
    return $text ;
}

sub partraverse {
    my ($filename,$lineno,@lines) = @_ ;
    return unless @lines ;
    my @front ;
    foreach my $line ( @lines ) {
        if( @front &&
            $front[$#front] =~ /\.\)?\s*$/ &&
            $line =~ /^\(?[A-Z]/ ) {
            $shipout->(ggettext($filename,$lineno,join "",@front));
            $lineno += @front ;
            @front = () ;
        }
        push @front, $line ;
    }
    $shipout->(ggettext($filename,$lineno,join "",@front));
}

sub traverser($) {
    my ($filename) = @_ ;
    open MAN, "<", $filename or die "Cannot open $filename" ;
    my $lineno = 0 ;
    my @saved = () ;
    while( <MAN> ) {
        $lineno++ ;
        if( /^([^.\'\#]|.([BI]|[BRI][IRB]) )/ ) {
            push @saved, $_ ;
            next ;
        }
        if( @saved ) {
            partraverse($filename,$lineno-@saved,@saved);
            @saved = () ;
        }
        if( /^#line (\d+) \"(.*)\"/ ) {
            my ($newline,$newfile) = ($1,$2);
            $filename = $newfile ;
            $lineno = $newline-1 ;
        } elsif( /^\#/ ) {
            # Ok, breaking the translation block is all we need.
        } elsif( /^\.\\\"-+$/ ){
            # Another way to request block breaking.
        } elsif( /^\.(\\\"|TH|so|P|IP|ds)($| )/ ) {
            $shipout->($_) ;
        } elsif( /^\.(SH) (.*)$/ ) {
            $shipout->(".$1 ".ggettext($filename,$lineno,$2)."\n") ;
        } elsif( /^\.TP/ ) {
            $shipout->($_) ;
            $_ = <MAN> ; $lineno++ ;
            if( /^\\fB\\-/ ) {
                s/\\fI(.*?)\\fR/
                    "\\fI".ggettext($filename,$lineno,$1)."\\fR"/ge ;
                $shipout->($_) ;
            } else {
                partraverse($filename,$lineno,$_) ;
            }
        } else {
            /^.?(.?.?)/ ;
            die "$filename:$lineno: unsupported request .$1\n" ;
        }
    }
    if( @saved ) {
        partraverse($filename,$lineno-@saved,@saved);
    }
    close MAN ;
}


if( @ARGV && $ARGV[0] eq '-x' ) {
    shift @ARGV ;
    my %seenwhere = () ;
    my @strings = () ;
    $gettext = sub {
        my ($file,$line,$text) = @_ ;
        push @strings, $text unless exists $seenwhere{$text} ;
        push @{$seenwhere{$text}}, "$file:$line" ;
    };
    $shipout = sub {} ;
    for my $filename ( @ARGV ) {
        traverser($filename) ;
    }
    my $date = `date "+%Y-%m-%d %H:%M%z"` ;
    chomp $date ;
    print << "EOF" ;
# SOME DESCRIPTIVE TITLE.
# This file is put in the public domain.
# FIRST AUTHOR <EMAIL\@ADDRESS>, YEAR.
#
#, fuzzy
msgid ""
msgstr ""
"Project-Id-Version: Xcftools-manpages VERSION\\n"
"Report-Msgid-Bugs-To: henning\@makholm.net\\n"
"POT-Creation-Date: $date\\n"
"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n"
"Last-Translator: FULL NAME <EMAIL\@ADDRESS>\\n"
"Language-Team: LANGUAGE <LL\@li.org>\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=CHARSET\\n"
"Content-Transfer-Encoding: 8bit\\n"

EOF
;
    for my $string ( @strings )  {
        print(join(" ","#:",@{$seenwhere{$string}}),"\n");
        $string =~ s/([\\\"])/\\$1/g ;
        $string =~ s/\n$/\\n/ ;
        print "msgid \"" ;
        print(join "\\n\"\n      \"",split /\n/, $string) ;
        print "\"\n";
        print "msgstr \"\"\n\n" ;
    }
    exit 0 ;
}

if( @ARGV == 2 ) {
    my ($pofile,$manfile) = @ARGV ;
    open PO, "<", $pofile or die "Pofile $pofile not found" ;
    my %catalog ;
    my $msgid ;
    my $addto ;
    while( <PO> ) {
        # This is an extremely simple-minded parser.
        if( s/^\s*msgid\s+\"/\"/ ) {
            $msgid = "" ;
            $addto = \$msgid ;
        } elsif( s/^\s*msgstr\s+\"/\"/ ) {
            $catalog{$msgid} = "" ;
            $addto = \$catalog{$msgid} ;
        }
        if( /^\s*\"(.*)\"\s*$/ ) {
            $_ = $1 ;
            s/\\(.)/$1 eq 'n' ? "\n" : $1/ge ;
            $$addto .= $_ ;
        }
    }
    close PO ;
    $gettext = sub {
        my ($file,$line,$text) = @_ ;
        return $catalog{$text} if $catalog{$text} ;
        return $text ;
    };
    $shipout = sub {
        print @_ ;
    };
    traverser($manfile) ;
    exit 0 ;
}

print STDERR "Usage: $0 -x man-source-files\n" ;
print STDERR "   or: $0 pofile manfile\n" ;

    
    
    
