## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#

AUTOMAKE_OPTIONS = dejagnu

if ENABLE_MING

CLEANFILES =  \
      gnash-dbg.log \
      site.exp.bak \
      testrun.sum \
      testrun.log


INCLUDES = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/server/parser  \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/testsuite \
        $(MING_CFLAGS) \
	$(NULL)


check_PROGRAMS = \
	moviecliploader_test \
	definebitsjpeg2 \
	DefineEditTextTest \
	DefineEditTextVariableNameTest \
	DefineEditTextVariableNameTest-Runner \
	RollOverOutTest-Runner \
	ButtonEventsTest-Runner \
	spritehier \
	RollOverOutTest \
	ButtonEventsTest \
	PlaceObject2Test \
	RemoveObject2Test

# TESTS = \
# 	DefineEditTextVariableNameTest-Runner 

definebitsjpeg2_SOURCES = definebitsjpeg2.c
definebitsjpeg2_LDADD = $(MING_LIBS)

moviecliploader_test_SOURCES =  moviecliploader_test.c
moviecliploader_test_LDADD = $(MING_LIBS)

DefineEditTextTest_SOURCES =  DefineEditTextTest.c
DefineEditTextTest_LDADD = $(MING_LIBS)

DefineEditTextVariableNameTest_SOURCES =	\
	DefineEditTextVariableNameTest.c	\
	ming_utils.h				\
	ming_utils.c				\
	$(NULL)
DefineEditTextVariableNameTest_LDADD = $(MING_LIBS)

spritehier_SOURCES =  spritehier.c
spritehier_LDADD = $(MING_LIBS)

PlaceObject2Test_SOURCES =	\
	PlaceObject2Test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
PlaceObject2Test_LDADD = $(MING_LIBS)

RemoveObject2Test_SOURCES = RemoveObject2Test.c
RemoveObject2Test_LDADD = $(MING_LIBS)

moviecliploader_tests: moviecliploader_test
	: test with relative urls 
	ln -fs $(top_srcdir)/testsuite/media media
	./moviecliploader_test media
	mv moviecliploader_test.swf moviecliploader_test_relative.swf
	: test absolute url
	./moviecliploader_test $(top_srcdir)/testsuite/media

definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2 $(srcdir)/../media/lynch.jpg

DefineEditTextTest.swf: DefineEditTextTest
	./DefineEditTextTest $(top_srcdir)/testsuite/media

DefineEditTextVariableNameTest.swf: DefineEditTextVariableNameTest
	./DefineEditTextVariableNameTest $(top_srcdir)/testsuite/media

DefineEditTextVariableNameTest-Runner: $(srcdir)/../generic-testrunner.sh DefineEditTextVariableNameTest.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) DefineEditTextVariableNameTest.swf > $@
	chmod +x $@

spritehier.swf: spritehier
	./spritehier

PlaceObject2Test.swf: PlaceObject2Test 
	./PlaceObject2Test

RemoveObject2Test.swf: RemoveObject2Test 
	./RemoveObject2Test

RollOverOutTest_SOURCES =	\
	RollOverOutTest.c	\
	ming_utils.h	\
	ming_utils.c	\
	$(NULL)
RollOverOutTest_LDADD = $(MING_LIBS)
RollOverOutTest.swf: RollOverOutTest
	./RollOverOutTest $(top_srcdir)/testsuite/media
RollOverOutTest_Runner_SOURCES = \
	RollOverOutTest-Runner.cpp \
	$(NULL)
RollOverOutTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
RollOverOutTest_Runner_DEPENDENCIES = \
	RollOverOutTest.swf	\
	$(NULL)

ButtonEventsTest_SOURCES =	\
	ButtonEventsTest.c	\
	ming_utils.h	\
	ming_utils.c	\
	$(NULL)
ButtonEventsTest_LDADD = $(MING_LIBS)
ButtonEventsTest.swf: ButtonEventsTest
	./ButtonEventsTest $(top_srcdir)/testsuite/media
ButtonEventsTest_Runner_SOURCES = \
	ButtonEventsTest-Runner.cpp \
	$(NULL)
ButtonEventsTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
ButtonEventsTest_Runner_DEPENDENCIES = \
	ButtonEventsTest.swf	\
	$(NULL)

clean-local: 
	-rm *.swf media

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	RollOverOutTest-Runner \
	DefineEditTextVariableNameTest-Runner \
	ButtonEventsTest-Runner

check-DEJAGNU: site-update $(check_PROGRAMS)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp
endif

