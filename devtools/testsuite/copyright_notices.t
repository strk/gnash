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

my $year = (localtime())[5] + 1900;

my @failures;
FILE: foreach my $path (@files) {
    open my $fh, '<', $path
        or die "Cannot open '$path' for reading: $!\n";

    ## Read in the entire file, as some of the cleaning spans lines
    local $/ = undef;
    my $entire_file = <$fh>;
    close $fh;

    my @reason;

    ## If there is no copyright notice, skip other checks
    if ($entire_file !~ m|This program is free software; you can|) {
        push @failures => "$path:\n\t* Copyright notice is missing.\n";
        next FILE;
    }

    ## 'You should have received a copy of the GNU General Public License
    ## along with the program; see the file COPYING.' should begin a new
    ## paragraph.
    if ($entire_file =~ m|details.\s+[*/]+\s+You|s) {
        push @reason, "* 'You should have received...' does not ".
          "begin a new paragraph."
    }

    ## Each file should start with a one-line comment describing it.
    my ($fn) = $path =~ m|([^/\\]+)$|;
    if ($entire_file !~ m|^\s*//\s+$fn|is) {
        push @reason, "* Missing intro comment with filename and description."
    }

    ## There should be no unusual whitespace before the CVS tag.
    # TODO (I haven't thought of a good way to check for this)

    ## Copyright notices should include the current year
    if ($entire_file !~ m|Copyright \(C\)[,\s\d]+$year|is) {
        push @reason, "* Copyright does not extend to $year.";
    }

    push @failures => "$path:\n\t". (join "\n\t", @reason) ."\n" if (@reason);
}

ok( !scalar(@failures), "Incorrect GNU copyright notice" )
    or diag("Incorrect GNU copyright notice found in ".
    scalar @failures." files:\n@failures");

=pod

=head1 NAME

devtools/testsuite/copyright_notices.t - look for incorrect copyright notices

=head1 SYNOPSIS

    # test all files
    % prove devtools/testsuite/copyright_notices.t

    # test specific files
    % perl devtools/testsuite/copyright_notices.t recently/modified/file

=head1 DESCRIPTION

This test looks for errors in the copyright notices in all C++ files.  
Specifically, it looks for:

=over 4

=item * That there is a copyright notice

=item * Instances where the line 'You should have received...' does not
begin a new paragraph

=item * Files which don't begin with a comment which includes the filename
and a description of the file

=item * Copyright notices which don't include the current year

=back

A test to check for odd whitespace after the copyright notice and before
the CVS Id tag should be added.

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
