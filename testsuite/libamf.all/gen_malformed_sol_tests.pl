#!perl

# The script looks for files in ../tmpSharedObject/malformed, so you will have
# to put your malformed files there or change where the script looks.
#
# Use 'makeswf -v6 SOLrobustness.as -o SOLrobustness.swf' and run with Gnash.

use strict;
use warnings;

my @filelist;
my $file;

my $dir="../tmpSharedObject/malformed";

opendir(DIR, $dir) || die("Cannot open directory: $dir");
@filelist = readdir(DIR);
closedir(DIR);

open(OUTF, ">SOLrobustness.as");

print OUTF <<EE;
// SOL file robustness testing. Feed with malformed SOL files in
// tmpSharedObject/malformed
//
// 'makeswf -v6 SOLrobustness.as -o SOLrobustness.swf' and run with Gnash.
//
EE

# ActionScript generation
for $file (@filelist)
{
    # Skip hidden files, directories and anything that doesn't end in
    # .sol
    next if ($file =~ m/^\./ || $file !~ m/\.sol$/);
    $file =~ s/\.sol$//;
    print OUTF "so = SharedObject.getLocal(\"malformed/$file\", \"/\");\n";
    print OUTF "trace(so.getSize());\n";
    print OUTF "delete so;\n";

}

close(OUTF);
