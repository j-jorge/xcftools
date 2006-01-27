$L = 64 ;
print "P3 $L $L 255\n" ;
for $y ( 0 .. $L-1 ) {
    for $x ( 0 .. $L-1 ) {
        $m = int($y / 4)*16 + int($x/4) ;
        if( $m < 216 ) {
            print( 51 * ($m % 6), " ",
                   51 * (int($m/6) % 6), " ",
                   51 * (int($m/36)), "\n" );
        } else {
            $m = ($m-216) * 6 + 10 ;
            print( "$m $m $m\n" );
        }
    }
}
