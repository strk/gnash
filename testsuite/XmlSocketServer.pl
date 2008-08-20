use IO::Socket;
use IO::Select;
use Time::HiRes;

$SIG{PIPE}='IGNORE';

$m=new IO::Socket::INET(Listen=>1,LocalPort=>2229);
$O=new IO::Select($m);


$/ = "\0";

while (@S = $O->can_read) {
    foreach (@S) {
        if ($_==$m) {
            $C=$m->accept;
            $O->add($C);
        }
        else {
            my $R=sysread($_, $i, 2048);
            
            # Log message received:
            print "XmlSocketServer: received \"$i\"\n";
            
            if ($R==0) {
                $T=syswrite($_, "\n", 2048);
                if ($T==undef) {
                    $O->remove($_);
                }
            }
            else {
            
                # Sleep a bit before sending a reply to mimic web traffic
                # (well, sort of).
                Time::HiRes::sleep(0.5);
                print "XmlSocketServer: sending \"$i\" \n";
              
                $i =~ s/\*NEWLINE\*/\n/;
                $i =~ s/\*NULL\*/\0/;

                foreach $C($O->handles) {
                    $T=syswrite($C, $i, 2048);
                }
            }
        }
    }
}
