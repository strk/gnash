#!perl


########################### NOTES ##################################
## If you wish to change the output of this program, search for
## 'sub create_headerfile' and 'sub create_cppfile'.
##
## To run this program, type 'gen-class.pl --help' (you may need
## to use '/path/to/perl gen-clas.pl --help'), which will display
## available options.
####################################################################


use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;

our $VERBOSE = 0;  ## default setting; can be changed by command-line option

our $LICENSE = q|//
//   Copyright (C) | . (join ', ', (2005..((localtime)[5]+1900))) .
q| Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
|;

my %args = %{process_arguments()};
create_headerfile(%args);
create_cppfile(%args);

########################### Output Functions #######################

## Create the ClassName.h file
sub create_headerfile {
    my %args = @_;

    my $fh;
    open ($fh, '>', $args{headerfile}) or die
        "Cannot open file '$args{headerfile}': $!\n";
    notify("Creating file '$args{headerfile}'.");
    print $fh <<EOF;
// $args{headerfile}:  ActionScript "$args{class}" class, for Gnash.
EOF
    print $fh $LICENSE;

    ## The text between the EOFs will be printed in the header file.
    print $fh <<EOF;

#ifndef __GNASH_ASOBJ_$args{up}_H__
#define __GNASH_ASOBJ_$args{up}_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <memory> // for auto_ptr

namespace gnash {

class as_object;

/// Initialize the global $args{class} class
void $args{lc}_class_init(as_object& global);

/// Return a $args{class} instance (in case the core lib needs it)
//std::auto_ptr<as_object> init_$args{lc}_instance();

} // end of gnash namespace

// __GNASH_ASOBJ_$args{up}_H__
#endif
EOF

    close $fh;
    return;
}

## Create the ClassName.cpp file
sub create_cppfile {
    my %args = @_;

    my $fh;
    open ($fh, '>', $args{cppfile}) or die
        "Cannot open file '$args{cppfile}': $!\n";
    notify("Creating file '$args{cppfile}'.");
    print $fh <<EOF;
// $args{cppfile}:  ActionScript "$args{class}" class, for Gnash.
EOF
    print $fh $LICENSE;


    ## Loop through the list of methods.  Generate code, stored in
    ## $declarations, $registrations, and $implementations.  $declarations
    ## contains the declaration code for all methods, etc.  These will
    ## be printed later, in the section delimited with EOF.  You may
    ## change the output of these three looped sections.
    my ($declarations, $registrations, $implementations);
    foreach my $m (@{$args{methods}}) {

        ## Where you see 'qq|' and '|', read '"' and '"'.  Using the quote
        ## operator eliminates the need to escape literal quotes.  Thus,
        ##   qq|This is an "example".\n|  ==  "This is an \"example\".\n"
        ## A '.' concatenates a string.

        $declarations .=
          qq|\nstatic void $args{lc}_| .$m. qq|(const fn_call& fn);|;

        $registrations .=
          qq|\n    o.init_member("$m", new builtin_function($args{lc}_$m));|;

        $implementations .=
          qq|\nstatic as_value\n$args{lc}_| .$m.
          qq|(const fn_call& fn)
{
	$args{lc}_as_object* ptr = ensureType<$args{lc}_as_object>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value();
}
|;

    }

    ## The text between the EOFs will be printed in the C++ source file.
    print $fh <<EOF;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "$args{class}.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {
$declarations
void $args{lc}_ctor(const fn_call& fn);

static void
attach$args{class}Interface(as_object& o)
{$registrations
}

static as_object*
get$args{class}Interface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attach$args{class}Interface(*o);
	}
	return o.get();
}

class $args{lc}_as_object: public as_object
{

public:

	$args{lc}_as_object()
		:
		as_object(get$args{class}Interface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "$args{class}"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

$implementations
as_value
$args{lc}_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new $args{lc}_as_object;

	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void $args{lc}_class_init(as_object& global)
{
	// This is going to be the global $args{class} "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&$args{lc}_ctor, get$args{class}Interface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attach$args{class}Interface(*cl);
	}

	// Register _global.$args{class}
	global.init_member("$args{class}", cl.get());
}

} // end of gnash namespace
EOF

    close $fh;
    return;
}



########################### Helper Functions #######################

## Accept and process the user's arguments
sub accept_arguments {
    my %args;
    GetOptions(
        "class=s"   => \$args{class},
        "help"      => \$args{help},
        "verbose!"  => \$VERBOSE,
        "force!"    => \$args{force},
        "notes=s"   => \$args{datafile},
    );

    ## Class is a required argument
    pod2usage(-verbose => 0), exit if ($args{help} || !$args{class});
    delete $args{help};

    ## Output files should not already exist.
    $args{headerfile} = "$args{class}.h";
    $args{cppfile}    = "$args{class}.cpp";
    unless($args{force}) {
        die "$args{headerfile} exists!  Aborting.\n" if (-e $args{headerfile});
        die "$args{cppfile} exists!  Aborting.\n" if (-e $args{cppfile});
    }
    notify("The following files will be written: '$args{headerfile}', ".
      "'$args{cppfile}'");

    ## Use the default note file unless one was specified; ensure it exists.
    $args{datafile} = '../../doc/C/NOTES' if (!$args{datafile});
    die "Could not find file '$args{datafile}'; aborting.\n"
        unless (-e $args{datafile});
    notify("Using notes file '$args{datafile}'");

    return \%args;
}

## Create a hash containing data which will be of use throughout the script.
sub process_arguments {
    my %args = %{accept_arguments()};

    ## Find our methods and properties
    %args = (%args, %{parse_notefile(%args)});

    ## Add some extra variables we'll use often
    $args{lc} = lc($args{class});
    $args{up} = uc($args{class});

    return \%args;
}

## Report a message if the user has enabled verbose.
sub notify {
    return unless ($VERBOSE);
    my $message = shift;
    print "$message\n";
}

## Take all property names out of notefile data.
sub get_properties {
    my %args = @_;
    my @want;
    foreach my $row (@{$args{notes}}) {
        push @want, $1 if ($row =~ /^$args{class}\.(\w+)$/);
    }
    return \@want;
}

## Take all method names out of notefile data.
sub get_methods {
    my %args = @_;
    my @want;

    ## Note that case should not be converted, as SWF7 is case-sensitive
    foreach my $row (@{$args{notes}}) {
        push @want, $1 if ($row =~ /^$args{class}\.(\w+)\(\)$/);
    }
    return \@want;
}

## Get references to the class out of the notefile
sub parse_notefile {
    my %args = @_;
    my $fh;
    my @data;

    open($fh, '<', $args{datafile}) or die
        "Cannot open file '$args{datafile}': $!\n";

    while (<$fh>) {
        push @data, $_ if ($_ =~ /^$args{class}\b/);
    }

    close $fh;

    my $methods = get_methods(%args, notes => \@data);
    my $props   = get_properties(%args, notes => \@data);
    return { methods => $methods, properties => $props };
}

########################### Documentation ############################
=pod

=head1 SYNOPSIS

    perl gen-asclass.pl --class <classname> [--verbose] [--notes <filename>] [--force]

=over 4

=item --class <classname>

This required argument specifies the name of the class you wish to create.

=item --notes <filename>

This allows you to specify the 'notes' where method names and (static)
properties are specified.  By default, the file is F<../../doc/C/NOTES>,
but if you wish to create a custom class, you should create your own version
of this file following the same format.

=item --force

With this option enabled, any pre-existing class files of the same name
will be overwritten.

=item --verbose

This is an optional argument.  If you enable it, the script will
inform you of its progress.

=back

=head1 LICENSE

Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.

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
