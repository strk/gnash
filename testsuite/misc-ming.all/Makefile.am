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

EXTRA_DIST = VarAndCharClashTest.as 


AM_CPPFLAGS = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/server/parser  \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/testsuite \
        $(MING_CFLAGS) \
	$(NULL)


check_PROGRAMS = \
	ButtonEventsTest \
	ButtonEventsTest-Runner \
	DefineEditTextTest \
	DefineEditTextVariableNameTest \
	DefineEditTextVariableNameTest-Runner \
	Dejagnu \
	Dejagnu.swf \
	PlaceObject2Test \
	RemoveObject2Test \
	RollOverOutTest \
	RollOverOutTest-Runner \
	VarAndCharClashTest-Runner \
	definebitsjpeg2 \
	moviecliploader_test \
	spritehier \
	timeline_var_test \
	timeline_var_test-Runner \
	root_stop_test \
	root_stop_testrunner \
	loop_test \
	loop_test-Runner \
	$(NULL)

# TESTS = \
# 	DefineEditTextVariableNameTest-Runner 

definebitsjpeg2_SOURCES = definebitsjpeg2.c
definebitsjpeg2_LDADD = $(MING_LIBS)

moviecliploader_test_SOURCES =  moviecliploader_test.c
moviecliploader_test_LDADD = $(MING_LIBS)

#FIXME: make this rule consistent with the others 
#and write a test runner for the absolute *and* relative
#versions
moviecliploader_tests: moviecliploader_test
	: test with relative urls 
	ln -fs $(top_srcdir)/testsuite/media media
	./moviecliploader_test media
	mv moviecliploader_test.swf moviecliploader_test_relative.swf
	: test absolute url
	./moviecliploader_test $(top_srcdir)/testsuite/media


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

definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2 $(srcdir)/../media/lynch.jpg

DefineEditTextTest.swf: DefineEditTextTest
	./DefineEditTextTest $(top_srcdir)/testsuite/media

DefineEditTextVariableNameTest.swf: DefineEditTextVariableNameTest
	./DefineEditTextVariableNameTest $(top_srcdir)/testsuite/media

DefineEditTextVariableNameTest_Runner_SOURCES = \
	DefineEditTextVariableNameTest-Runner.cpp \
	$(NULL)
DefineEditTextVariableNameTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
DefineEditTextVariableNameTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	DefineEditTextVariableNameTest.swf      \
	$(NULL)

timeline_var_test_SOURCES =	\
	timeline_var_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
timeline_var_test_LDADD = $(MING_LIBS)

timeline_var_test.swf: timeline_var_test
	./timeline_var_test $(top_srcdir)/testsuite/media

timeline_var_test-Runner: $(srcdir)/../generic-testrunner.sh timeline_var_test.swf
	sh $< $(top_builddir) timeline_var_test.swf > $@
	chmod 755 $@

root_stop_test_SOURCES =	\
	root_stop_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
root_stop_test_LDADD = $(MING_LIBS)

root_stop_test.swf: root_stop_test
	./root_stop_test $(top_srcdir)/testsuite/media

root_stop_testrunner_SOURCES = \
	root_stop_testrunner.cpp \
	$(NULL)
root_stop_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
root_stop_testrunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	root_stop_test.swf      \
	$(NULL)

loop_test_SOURCES =	\
	loop_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
loop_test_LDADD = $(MING_LIBS)

loop_test.swf: loop_test
	./loop_test $(top_srcdir)/testsuite/media

loop_test_Runner_SOURCES = \
	loop_test-Runner.cpp \
	$(NULL)
loop_test_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
loop_test_Runner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	$(NULL)
loop_test_Runner_DEPENDENCIES = \
	loop_test.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

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
	$(top_builddir)/testsuite/libtestsuite.la \
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
	$(top_builddir)/testsuite/libtestsuite.la \
	ButtonEventsTest.swf	\
	$(NULL)

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
VarAndCharClashTest.swf: $(srcdir)/../media/green.swf ../Dejagnu.swf $(srcdir)/VarAndCharClashTest.as 
	$(MAKESWF) -o $@ ../Dejagnu.swf $(srcdir)/../media/green.swf $(srcdir)/VarAndCharClashTest.as
VarAndCharClashTest-Runner: $(srcdir)/../generic-testrunner.sh VarAndCharClashTest.swf
	sh $< $(top_builddir) VarAndCharClashTest.swf > $@
	chmod 755 $@

Dejagnu_SOURCES =	\
	Dejagnu.c	\
	ming_utils.h				\
	ming_utils.c				\
	$(NULL)
Dejagnu_LDADD = $(MING_LIBS)

Dejagnu.swf: Dejagnu
	./Dejagnu $(top_srcdir)/testsuite/media

clean-local: 
	-rm *.swf media

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	DefineEditTextVariableNameTest-Runner \
	RollOverOutTest-Runner \
	ButtonEventsTest-Runner \
	VarAndCharClashTest-Runner \
	timeline_var_test-Runner \
	root_stop_testrunner \
	loop_test-Runner \
	$(NULL)


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

