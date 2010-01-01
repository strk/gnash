#! perl

use strict;
use warnings;

use FindBin qw/$Bin/;
use lib $Bin.'/../lib';
use Test::More tests => 1;
use Gnash::Distribution;
use Gnash::Utils qw/clean/;

my $DIST = Gnash::Distribution->new;

## Jerry Gay explained that this construction was needed because Windows
## does not get @ARGV the same way as most systems.
my @files = @ARGV 
    ? ( $^O eq 'MSwin32' ? <@ARGV> : @ARGV )
    : $DIST->get_cpp_language_files();

my @failures;  

foreach my $path (@files) {
    open my $fh, '<', $path
        or die "Cannot open '$path' for reading: $!\n";

    my $prefix = qq<  $path line(s):>;
    my $message = '';

    ## Read in the entire file, as some of the cleaning spans lines
    local $/ = undef;
    my $entire_file = clean(<$fh>);
    my @lines = split /\n/, $entire_file;    

    ## We need the array index to report the correct line number.
    foreach my $i (0..$#lines) {
        my $string = $lines[$i];

        ## Skip unless some tabs are found.
        next unless ($string =~ /\t/);
        $message .= " ".($i+1);
    }
    push @failures => "$prefix$message\n" if ($message);
    close $fh;
}

ok( !scalar(@failures), "tabs" )
    or diag( "hard tabs found in ".scalar @failures." files:\n@failures" );

=head1 NAME

devtools/testsuite/tabs.t - checks for tabs in C++ source and headers

=head1 SYNOPSIS

    # test all files
    % prove devtools/testsuite/tabs.t

    # test specific files
    % perl devtools/testsuite/tabs.t recently/modified/file

=head1 DESCRIPTION

This test checks for code which contains tabs.

=head1 AUTHORS

Ann Barcomb <kudra@domaintje.com>, based upon ideas from the Parrot
L<http://http://dev.perl.org/perl6/> test suite.

=head1 COPYRIGHT

Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

=cut
