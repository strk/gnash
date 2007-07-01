## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

# $Id: Makefile.am,v 1.4 2007/07/01 10:54:43 bjacques Exp $

AUTOMAKE_OPTIONS = dejagnu

AM_LDFLAGS = \
	../../libbase/libgnashbase.la \
	../../libamf/libgnashamf.la \
	$(LIBLTDL) \
	$(GLIB_LIBS) \
	$(LIBXML_LIBS) \
	$(CURL_LIBS) \
	$(BOOST_LIBS) \
	$(PTHREAD_LIBS)

AM_CPPFLAGS = -DSRCDIR=\"$(srcdir)\"

INCLUDES = -I.. \
        -I$(top_srcdir)	\
        -I$(top_srcdir)/libamf \
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server \
        $(LIBXML_CFLAGS) \
	$(DMALLOC_CFLAGS) \
	$(CURL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PTHREAD_CFLAGS)

# AM_LDFLAGS  = $(OPENGL_LIBNS) $(SDL_LIBS)


check_PROGRAMS = test_number test_string test_object test_variable
test_number_SOURCES = test_number.cpp
test_number_LDADD = $(AM_LDFLAGS)
test_string_SOURCES = test_string.cpp
test_string_LDADD = $(AM_LDFLAGS)
test_object_SOURCES = test_object.cpp
test_object_LDADD = $(AM_LDFLAGS)
test_variable_SOURCES = test_variable.cpp
test_variable_LDADD = $(AM_LDFLAGS)


# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-local:
	-rm -f core.* site.exp.bak testrun.sum testrun.log

TEST_DRIVERS = ../simple.exp


check-DEJAGNU: site-update $(TEST_CASES)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(check_PROGRAMS)"; do \
	    $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(check_PROGRAMS)\"" >> site.exp
