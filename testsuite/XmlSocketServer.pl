use IO::Socket;
use IO::Select;
use Time::HiRes;

$SIG{PIPE}='IGNORE';

my $m=new IO::Socket::INET(Listen=>1,LocalPort=>2229,Reuse=>1,Proto=>'tcp');
my $O=new IO::Select($m);

$verbose = ( $ARGV[0] eq '-v' ) ? 1 : 0;

$/ = "\0";

while (@S = $O->can_read) {
    foreach (@S) {
        if ($_==$m) {
            $C=$m->accept;
            $O->add($C);
        }
        else {
            my $R=sysread($_, $i, 16000);
            
            # Log message received:
            print "XmlSocketServer: received \"$i\"\n" if $verbose;
            
            if ($R==0) {
                $T=syswrite($_, "\n", 16000);
                if ($T==undef) {
                    $O->remove($_);
                }
            }
            else {
            
                # Sleep a bit before sending a reply to mimic web traffic
                # (well, sort of).
                Time::HiRes::sleep(0.1);
                print "XmlSocketServer: sending \"$i\" \n" if $verbose;
              
                $i =~ s/\*NEWLINE\*/\n/g;
                $i =~ s/\*NULL\*/\0/g;

                $T=syswrite($_, $i, 16000);
            }

            if ($i =~ m/closeNow/) {
                print("XmlSocketServer: closing...\n") if $verbose;
                close($C) || die "Could not close reading Socket";
            }
        }
    }
}

close($m) || die "Could not close server Socket";
print("XmlSocketServer: salut!\n") if $verbose;

