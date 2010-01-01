package Gnash::Distribution;

use strict;
use warnings;

use Data::Dumper;
use File::Spec;
use FindBin qw/$Bin/;
use File::Find;

my $dist;

sub new {
    my $class = shift;

    return $dist if defined $dist;
    my $self = bless {}, $class;
    return $self->_initialize(@_);
}

{
    no warnings 'File::Find';
    my @found_files;
    sub _initialize {
        my $self = shift;

        my $dist = $_[0] || $Bin.'/../..';
        $self->{_distribution_root} = $dist; 
        _croak(undef, 
          "Failed to find distribution root; did you move the test directory?"
        ) unless (-d $dist);

        find(\&_search_for_files, $dist);
        if( defined $dist ) {
            $self->_dist_files( [
                @found_files
            ] );
        }

        return $self;
   }

   sub _search_for_files {
       ## Get only files, which are not in the CVS directory or otherwise
       ## related to CVS/subversion.  Prune might make this more efficient.
       return unless (-f $_);
       return if ($File::Find::name =~ m|/CVS/| || 
                  $File::Find::name =~ m|/\.svn/| ||
                  $File::Find::name =~ m|\.svnignore/| ||
                  $File::Find::name =~ m|/\.cvsignore/|);

       push @found_files, $File::Find::name;
   }

}

sub distribution_root {
    my $self = shift;
    return $self->{_distribution_root};
}

sub _croak {
    my( $self, @message ) = @_;
    require Carp;
    Carp::croak(@message);
}

BEGIN {
    my @getter_setters = qw{ _dist_files };

    for my $method ( @getter_setters ) {
        no strict 'refs';

        *$method = sub {
            my $self = shift;
            unless (@_) {
                $self->{$method} ||= [];
                return wantarray
                    ? @{ $self->{$method} }
                    : $self->{$method};
            }
            $self->{$method} = shift;
            return $self;
        };
    }
}


BEGIN {
    my %file_class = (
        source => {
            cpp    => { file_exts => ['cpp', 'cc'] },
            m4     => { file_exts => ['m4'] },
        },
        header => {
            cpp    => { file_exts => ['h'] },
        },
    );

    ## Some of this can probably be cropped out, since we're ignoring dirs
    my @ignore_dirs = qw{ .svn CVS };

    for my $class ( keys %file_class ) {
        for my $type ( keys %{ $file_class{$class} } ) {
            no strict 'refs';

            my @exts       = @{ $file_class{$class}{$type}{file_exts} };
            my @exceptions = defined $file_class{$class}{$type}{except_dirs}
                ? @{ $file_class{$class}{$type}{except_dirs} }
                : ();
            my $method     = join '_' => $type, $class;
            my $filter_ext = join '|' => map { "\\.${_}\$" } @exts;
            my $filter_dir = join '|' =>
                map { qr{\b$_\b} }
                map { quotemeta($_) }
                @ignore_dirs,
                @exceptions;

            next unless $method;

            *{ $method . '_file_directories' } = sub {
                my $self = shift;

                # Look through the distribution files for
                # file endings in the proper extensions and make
                # a hash out of the directories.
                my %dirs =
                    map { ( ( File::Spec->splitpath($_) )[1] => 1 ) }
                    grep { m|(?i)(?:$filter_ext)| }
                    $self->_dist_files;

                # Filter out ignored directories
                # and return the results
                return
                    sort
                    grep {
                      -d File::Spec->catdir($_) and File::Spec->catdir($_)
                    } grep { !m|(?:$filter_dir)|
                    } keys %dirs;
            };


            *{ $method . '_files' } = sub {
                my( $self ) = @_;

                # Look through the filelist
                # for files ending in the proper extension(s)
                # and return a sorted list of filenames
                return
                    sort
                    grep { m|(?i)(?:$filter_ext)| }
                    $self->_dist_files;
            };
        }
    }
}


sub get_m4_language_files {
    my $self = shift;

    my @files = (
        $self->m4_source_files,
    );

    my @m4_language_files = ();
    foreach my $file ( @files ) {
        next if $self->is_m4_exemption($file);
        push @m4_language_files, $file;
    }

    return @m4_language_files;
}

{
    my @exemptions;

    sub is_m4_exemption {
        my( $self, $file ) = @_;

        push @exemptions => map { File::Spec->canonpath($_) } qw{
        } unless @exemptions;

        $file->path =~ /\Q$_\E$/ && return 1
            for @exemptions;
        return;
    }
}

sub get_cpp_language_files {
    my $self = shift;

    my @files = (
        $self->cpp_source_files,
        $self->cpp_header_files,
    );

    my @cpp_language_files = ();
    foreach my $file ( @files ) {
        next if $self->is_cpp_exemption($file);
        push @cpp_language_files, $file;
    }

    return @cpp_language_files;
}


{
    my @exemptions;

    sub is_cpp_exemption {
        my( $self, $file ) = @_;

        push @exemptions => map { File::Spec->canonpath($_) } qw{
        } unless @exemptions;

        $file->path =~ /\Q$_\E$/ && return 1
            for @exemptions;
        return;
    }
}


1;

=pod

=head1 NAME

Gnash::Distribution - Get information about files in the Gnash distribution

=head1 SYNOPSIS

    use Gnash::Distribution;

    my $dist = Gnash::Distribution->new();
    my @cpp = $dist->get_cpp_language_files();  ## Get all C++ files

=head1 DESCRIPTION

This module can generate a list of all files found in your checkout of
the Gnash distribution.  Particular categories of files can then be
requested.

In order to find files, it works on the assumption that it will be used
by files in the directory devtools/testsuite, which will be located within
the Gnash top-level checkout directory.  You may also explicitly specify
a distribution root at construction.

=head1 METHODS

=over 4

=item new([DISTRIBUTION_ROOT])

The constructor will search the file system as described above, then
create a list of all files in the top-level checkout directory or in
nested directories.  It will throw an exception if the distribution
root is not found.

You may optionally supply your own distribution root, using a full
path.

=item distribution_root()

Returns the full path of the directory this library considers to be
your distribution root.  All files under that directory will be 
considered for inclusion in the lists returned by the next two methods.

Use this method if you want to ensure that you are operating on the
correct directory (for example if you are writing a one-off script).

=item get_cpp_language_files()

This method will return an array containing all C++ files, which includes
files ending with the following extensions: .h, .cc, and .cpp.

=item get_m4_language_files()

This method returns an array containing all files ending with the .m4
extension.

=back

=head1 AUTHORS

Ann Barcomb <kudra@domaintje.com> and Jerry Gay, based upon ideas from the Parrot
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
