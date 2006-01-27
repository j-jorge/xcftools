use warnings ;
use strict ;

binmode(STDIN);

my $a ;
read STDIN,$a,8 ;
if( $a ne "\x89PNG\x0d\x0a\x1a\x0a" ) {
    die "Malformed PNG header\n" ;
}
sub bv($) {
    ord(substr($a,$_[0],1)) ;
}
sub wv($) {
    my $a = shift ;
    return (bv($a) << 24) + (bv($a+1) << 16) + (bv($a+2)<<8) + bv($a+3);
}
my %all ;
while( !eof STDIN ) {
    read STDIN,$a,4 ;
    my $len = wv(0);
    read STDIN,$a,$len+8 ;
    my $type = substr($a,0,4) ;
    next if $type eq 'IDAT' ;
    last if $type eq 'IEND' ;
    if( $type eq 'IHDR' && $len == 13 ) {
        print wv(4),"x",wv(8),"x",bv(12),"\n" ;
        my $cmode = bv(13) ;
        print $cmode & 3 ? "color" : "gray" ;
        print "+index" if $cmode & 1 ;
        print "+alpha" if $cmode & 4 ;
        print "\nz",bv(14)," f",bv(15)," i",bv(16),"\n" ;
        next ;
    }
    my $aref = ($all{$type} ||= []) ;
    push @$aref, "$type($len)" ;
    {
        my $w = 16 ;
        $w = 24 if $type eq 'PLTE' ;
        $w = 8 if $type eq 'tRNS' ;
        for my $i ( 0 .. $len - 1 ) {
            push @$aref, sprintf("%s%02X", $i%$w ? " " : "\n ", bv($i+4) ) ;
            push @$aref, " " if
                $type eq 'PLTE' &&
                $i%3 == 2 &&
                ($i+1)%$w != 0 &&
                $i != $len-1 ;
        }
        push @$aref, "\n" ;
    }
}
for my $k ( sort keys %all ) {
    print @{$all{$k}} ;
}
    
    
    

