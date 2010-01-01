package Gnash::Utils;

use strict;
use warnings;
use Regexp::Common qw/comment delimited/;

use Exporter 'import';
our @EXPORT_OK = qw/
    clean 
    clean_cpp_comment 
    clean_c_comment 
    clean_single_quoted_string 
    clean_double_quoted_string
/;

sub clean {
    return 
      clean_cpp_comment(
        clean_c_comment(
          clean_quoted_string(
            $_[0]
          )
        )
      );
}

sub clean_cpp_comment {
    my $string = shift;
    while ($string =~ /$RE{comment}{'C++'}{-keep}/) {
        my $matched = $1;
        (my $newlines = $matched) =~ s/[^\n]//;
        $string =~ s/$RE{comment}{'C++'}/$newlines/;
    }
    return $string;
}

sub clean_c_comment {
    my $string = shift;
    while ($string =~ /$RE{comment}{C}{-keep}/) {
        my $matched = $1;
        (my $newlines = $matched) =~ s/[^\n]//;
        $string =~ s/$RE{comment}{C}/$newlines/;
    }
    return $string;
}

sub clean_quoted_string {
    my $string = shift;
 
    while ($string =~ /$RE{delimited}{-delim=>q{"'}}{-keep}/) {
        my $matched = $1;
        (my $newlines = $matched) =~ s/[^\n]//;
        $string =~ s/$RE{delimited}{-delim=>q{"'}}/$newlines/;
    }
    return $string;
}


1;

=pod

=head1 NAME

Gnash::Utils - Utility functions for the coding standards test suite.

=head1 SYNOPSIS

    use Gnash::Utils qw/clean/;
 
    clean($source_code);

=head1 DESCRIPTION

It is easier to run tests which check code quality if the code doesn't
contain comments or quoted strings.  This module provides some functions
to remove these distractions from the source code. 

=head1 FUNCTIONS

All functions may be optionally imported.

=over 4

=item clean($source)

This function calls C<clean_quoted_string>, 
C<clean_c_comment>, and C<clean_cpp_comment>
and returns a string.  It expects to receive the entire contents of a source
file.

=item clean_cpp_comment($source)

This routine removes comments which begin with C<//>.  It can operate on
an entire file or on just a single line of source code.  It returns the
modified input as a string.  For instance,
    return 1; // return true

becomes:
    return 1;

=item clean_c_comment($source)

This function removes comments of the C</* ... */> style.  It expects
to receive the entire source code and will return the modified code as a
string.  For example,
    /* This is a
       comment */
    return 1;

becomes:
    return 1;

=item clean_quoted_string($source)

This routine will remove quoted strings.  It expects to receive the
entire source code and will return the modified code as a string.  For
instance,
    return 'hello world';

becomes
    return ;

=back

=head1 AUTHORS

Ann Barcomb <kudra@domaintje.com>

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

