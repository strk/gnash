## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
#   2011  Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


AUTOMAKE_OPTIONS = dejagnu

AM_LDFLAGS = \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(GLIB_LIBS) \
	$(CURL_LIBS) \
	$(BOOST_LIBS) \
	$(DEJAGNU_CFLAGS) \
	$(PTHREAD_LIBS) \
	$(NULL)

AM_CPPFLAGS = -DSRCDIR=\"$(srcdir)\"

INCLUDES = \
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)/libnet \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libcore \
	-I$(top_srcdir)/testsuite \
	$(CURL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(DEJAGNU_CFLAGS) \
	$(PTHREAD_CFLAGS)

check_PROGRAMS = \
	test_amf \
	test_amfmsg \
	test_buffer \
	test_lc \
	test_el \
	test_sol \
	test_flv

test_el_SOURCES = test_el.cpp
test_el_LDADD = $(AM_LDFLAGS)

test_flv_SOURCES = test_flv.cpp
test_flv_LDADD = $(AM_LDFLAGS)

test_sol_SOURCES = test_sol.cpp
test_sol_LDADD = $(AM_LDFLAGS)
test_sol_DEPENDENCIES = $(solfiles)

test_amf_SOURCES = test_amf.cpp
test_amf_LDADD = $(AM_LDFLAGS)

test_amfmsg_SOURCES = test_amfmsg.cpp
test_amfmsg_LDADD = $(AM_LDFLAGS)

test_lc_SOURCES = test_lc.cpp
test_lc_LDADD = $(AM_LDFLAGS)

test_buffer_SOURCES = test_buffer.cpp
test_buffer_LDADD = $(AM_LDFLAGS)

# test_number_SOURCES = test_number.cpp
# test_number_LDADD = $(AM_LDFLAGS)

# test_string_SOURCES = test_string.cpp
# test_string_LDADD = $(AM_LDFLAGS)

# test_object_SOURCES = test_object.cpp
# test_object_LDADD = $(AM_LDFLAGS)

# test_variable_SOURCES = test_variable.cpp
# test_variable_LDADD = $(AM_LDFLAGS)

# These are captured AMF object in binary form.
AMFS =	\
	connect-object.amf \
	number-variable.amf \
	string-variable.amf \
	connect-string.amf \
	result-fail.amf \
	f03f.amf \
	result-success.amf

SOLS = \
	array.sol \
	boolean.sol \
	date.sol \
	null.sol \
	number.sol \
	object.sol \
	string.sol \
	settings.sol \
	undefined.sol \
	xml.sol

LCS = \
	soundoff.lc \
	string.lc

EXTRA_DIST = $(AMFS) $(SOLS)

# solfiles := $(patsubst %.xml,%.sol,$(SOLS))
#CLEANFILES = $(solfiles)
# solfiles: $(solfiles) 

# this script converts between SOL Shared Objects and XML files. Very useful for testing.
dist_noinst_SCRIPTS = s2x.py

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-local:
	-rm -f core.* site.exp.bak testrun.sum testrun.log

TEST_DRIVERS = ../simple.exp

SUFFIXES = .sol .xml

# .sol.xml:
# 	$(PYTHON) $(srcdir)/s2x.py -x $< $@

.xml.sol:
	$(PYTHON) $(srcdir)/s2x.py -s $(srcdir)/xmlfiles/$< $@

check-DEJAGNU: site-update $(TEST_CASES)
	runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASHRC="$(top_builddir)/testsuite/gnashrc" \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(check_PROGRAMS)"; do \
	    GNASHRC="$(top_builddir)/testsuite/gnashrc" \
	    $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(check_PROGRAMS)\"" >> site.exp
