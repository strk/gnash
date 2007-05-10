## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-ming.all; pwd)

if ENABLE_MING

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log

EXTRA_DIST = VarAndCharClashTest.as \
	KeyTest.as \
	gotoFrame2Test.as \
	DrawingApiTest.as \
	FlashVarsTest.as \
	intervalTest.as \
	FlashVarsTest.html


AM_CPPFLAGS = -I.. \
	-I$(top_srcdir)		\
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/backend \
	-I$(top_srcdir)/server  \
	-I$(top_srcdir)/server/parser  \
	-I$(top_srcdir)/libgeometry \
	-I$(top_srcdir)/testsuite \
	$(MING_CFLAGS) \
	-DMING_VERSION_CODE=$(MING_VERSION_CODE) \
	$(NULL)

check_PROGRAMS = \
	ButtonEventsTest \
	ButtonEventsTest-Runner \
	SpriteButtonEventsTest \
	SpriteButtonEventsTest-Runner \
	DefineEditTextTest \
	DefineEditTextVariableNameTest \
	DefineEditTextVariableNameTest-Runner \
	Dejagnu \
	Dejagnu.swf \
	PlaceObject2Test \
	RemoveObject2Test \
	RollOverOutTest \
	RollOverOutTest-Runner \
	definebitsjpeg2 \
	moviecliploader_test \
	spritehier \
	timeline_var_test \
	root_stop_test \
	root_stop_testrunner \
	loop_test \
	loop_test-Runner \
	place_object_test \
	move_object_test \
	place_and_remove_object_test \
	place_and_remove_object_insane_test \
	displaylist_depths_test \
	displaylist_depths_test2 \
	displaylist_depths_test3 \
	masks_test \
	attachMovieTest \
	attachMovieTestRunner \
	attachMovieLoopingTest \
	attachMovieLoopingTestRunner \
	registerClassTest \
	registerClassTestRunner \
	goto_frame_test \
	consecutive_goto_frame_test \
	matrix_test \
	multi_doactions_and_goto_frame_test \
	simple_loop_test \
	simple_loop_testrunner \
	action_execution_order_test \
	action_execution_order_extend_test \
	action_execution_order_test1 \
	action_execution_order_test2 \
	action_execution_order_test3 \
	action_execution_order_test4 \
	action_execution_order_test5 \
	action_execution_order_test6 \
	action_execution_order_test7 \
	action_execution_order_test8 \
	reverse_execute_PlaceObject2_test1 \
	reverse_execute_PlaceObject2_test2 \
	loadMovieTest \
	loadMovieTestRunner \
	eventSoundTest1 \
	eventSoundTest1-Runner \
	DrawingApiTestRunner \
	get_frame_number_test \
	frame_label_test \
	path_format_test \
	duplicate_movie_clip_test \
	event_handler_scope_test \
	masks_testrunner \
	key_event_test \
	key_event_testrunner \
	static_vs_dynamic1 \
	static_vs_dynamic2 \
	$(NULL)

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
check_PROGRAMS += KeyTest-Runner \
	intervalTestRunner 
endif

if MING_SUPPORTS_INIT_ACTIONS
check_PROGRAMS += \
	init_action_test \
	$(NULL)
endif

check_SCRIPTS = \
	timeline_var_test-Runner \
	place_object_testrunner \
	move_object_testrunner \
	place_and_remove_object_testrunner \
	place_and_remove_object_insane_testrunner \
	goto_frame_testrunner \
	consecutive_goto_frame_testrunner \
	multi_doactions_and_goto_frame_testrunner \
	action_execution_order_testrunner \
	action_execution_order_extend_testrunner \
	gotoFrame2Test-Runner \
	action_execution_order_test1runner \
	action_execution_order_test2runner \
	action_execution_order_test3runner \
	action_execution_order_test4runner \
	action_execution_order_test5runner \
	action_execution_order_test6runner \
	action_execution_order_test7runner \
	action_execution_order_test8runner \
	reverse_execute_PlaceObject2_test1runner \
	reverse_execute_PlaceObject2_test2runner \
	displaylist_depths_testrunner \
	displaylist_depths_test2runner \
	displaylist_depths_test3runner \
	get_frame_number_testrunner \
	frame_label_testrunner \
	path_format_testrunner \
	duplicate_movie_clip_testrunner \
	event_handler_scope_testrunner \
	matrix_testrunner \
	static_vs_dynamic1_testrunner \
	static_vs_dynamic2_testrunner \
	$(NULL)

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
check_SCRIPTS += VarAndCharClashTest-Runner \
	gotoFrame2Test-Runner \
	FlashVarsTest-Runner
endif

if MING_SUPPORTS_INIT_ACTIONS
check_SCRIPTS += \
	init_action_testrunner \
	$(NULL)
endif


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

eventSoundTest1_SOURCES = eventSoundTest1.c
eventSoundTest1_LDADD = $(MING_LIBS)

eventSoundTest1.swf: eventSoundTest1
	./eventSoundTest1 $(srcdir)/../media/sound1.mp3

eventSoundTest1_Runner_SOURCES = \
	eventSoundTest1-Runner.cpp \
	$(NULL)
eventSoundTest1_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
eventSoundTest1_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
eventSoundTest1_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	eventSoundTest1.swf      \
	$(NULL)

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
root_stop_testrunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' 
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
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
loop_test_Runner_DEPENDENCIES = \
	loop_test.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)


key_event_test_SOURCES =	\
	key_event_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
key_event_test_LDADD = $(MING_LIBS)

key_event_test.swf: key_event_test
	./key_event_test $(top_srcdir)/testsuite/media

key_event_testrunner_SOURCES = \
	key_event_testrunner.cpp \
	$(NULL)
key_event_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
key_event_testrunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
key_event_testrunner_DEPENDENCIES = \
	key_event_test.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

place_object_test_SOURCES =	\
	place_object_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
place_object_test_LDADD = $(MING_LIBS)

place_object_test.swf: place_object_test
	./place_object_test $(top_srcdir)/testsuite/media

place_object_testrunner: $(srcdir)/../generic-testrunner.sh place_object_test.swf
	sh $< -r5 $(top_builddir) place_object_test.swf > $@
	chmod 755 $@

move_object_test_SOURCES =	\
	move_object_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
move_object_test_LDADD = $(MING_LIBS)

move_object_test.swf: move_object_test
	./move_object_test $(top_srcdir)/testsuite/media

move_object_testrunner: $(srcdir)/../generic-testrunner.sh move_object_test.swf
	sh $< -r5 $(top_builddir) move_object_test.swf > $@
	chmod 755 $@

place_and_remove_object_test_SOURCES =	\
	place_and_remove_object_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
place_and_remove_object_test_LDADD = $(MING_LIBS)

place_and_remove_object_test.swf: place_and_remove_object_test
	./place_and_remove_object_test $(top_srcdir)/testsuite/media

place_and_remove_object_testrunner: $(srcdir)/../generic-testrunner.sh place_and_remove_object_test.swf
	sh $< -r5 $(top_builddir) place_and_remove_object_test.swf > $@
	chmod 755 $@

place_and_remove_object_insane_test_SOURCES =	\
	place_and_remove_object_insane_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
place_and_remove_object_insane_test_LDADD = $(MING_LIBS)

place_and_remove_object_insane_test.swf: place_and_remove_object_insane_test
	./place_and_remove_object_insane_test $(top_srcdir)/testsuite/media

place_and_remove_object_insane_testrunner: $(srcdir)/../generic-testrunner.sh place_and_remove_object_insane_test.swf
	sh $< -r5 $(top_builddir) place_and_remove_object_insane_test.swf > $@
	chmod 755 $@

displaylist_depths_test_SOURCES =	\
	displaylist_depths_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
displaylist_depths_test_LDADD = $(MING_LIBS)

displaylist_depths_test.swf: displaylist_depths_test
	./displaylist_depths_test $(top_srcdir)/testsuite/media

displaylist_depths_testrunner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test.swf
	sh $< -r5 $(top_builddir) displaylist_depths_test.swf > $@
	chmod 755 $@

displaylist_depths_test2_SOURCES =	\
	displaylist_depths_test2.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
displaylist_depths_test2_LDADD = $(MING_LIBS)

displaylist_depths_test2.swf: displaylist_depths_test2
	./displaylist_depths_test2 $(top_srcdir)/testsuite/media

displaylist_depths_test2runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test2.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test2.swf > $@
	chmod 755 $@

displaylist_depths_test3_SOURCES =	\
	displaylist_depths_test3.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
displaylist_depths_test3_LDADD = $(MING_LIBS)

displaylist_depths_test3.swf: displaylist_depths_test3
	./displaylist_depths_test3 $(top_srcdir)/testsuite/media

displaylist_depths_test3runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test3.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test3.swf > $@
	chmod 755 $@

masks_test_SOURCES =	\
	masks_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
masks_test_LDADD = $(MING_LIBS)

masks_test.swf: masks_test
	./masks_test $(top_srcdir)/testsuite/media

masks_testrunner_SOURCES = \
	masks_testrunner.cpp \
	$(NULL)
masks_testrunner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
masks_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
masks_testrunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	masks_test.swf      \
	$(NULL)

matrix_test_SOURCES =	\
	matrix_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
matrix_test_LDADD = $(MING_LIBS)

matrix_test.swf: matrix_test
	./matrix_test $(top_srcdir)/testsuite/media

matrix_testrunner: $(srcdir)/../generic-testrunner.sh matrix_test.swf
	sh $< -r5 $(top_builddir) matrix_test.swf > $@
	chmod 755 $@

get_frame_number_test_SOURCES =	\
	get_frame_number_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
get_frame_number_test_LDADD = $(MING_LIBS)

get_frame_number_test.swf: get_frame_number_test
	./get_frame_number_test $(top_srcdir)/testsuite/media

get_frame_number_testrunner: $(srcdir)/../generic-testrunner.sh get_frame_number_test.swf
	sh $< -f20 $(top_builddir) get_frame_number_test.swf > $@
	chmod 755 $@

frame_label_test_SOURCES =	\
	frame_label_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
frame_label_test_LDADD = $(MING_LIBS)

frame_label_test.swf: frame_label_test
	./frame_label_test $(top_srcdir)/testsuite/media

frame_label_testrunner: $(srcdir)/../generic-testrunner.sh frame_label_test.swf
	sh $< -r5 $(top_builddir) frame_label_test.swf > $@
	chmod 755 $@

path_format_test_SOURCES =	\
	path_format_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
path_format_test_LDADD = $(MING_LIBS)

path_format_test.swf: path_format_test
	./path_format_test $(top_srcdir)/testsuite/media

path_format_testrunner: $(srcdir)/../generic-testrunner.sh path_format_test.swf
	sh $< -r5 $(top_builddir) path_format_test.swf > $@
	chmod 755 $@

duplicate_movie_clip_test_SOURCES =	\
	duplicate_movie_clip_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
duplicate_movie_clip_test_LDADD = $(MING_LIBS)

duplicate_movie_clip_test.swf: duplicate_movie_clip_test
	./duplicate_movie_clip_test $(top_srcdir)/testsuite/media

duplicate_movie_clip_testrunner: $(srcdir)/../generic-testrunner.sh duplicate_movie_clip_test.swf
	sh $< -r5 $(top_builddir) duplicate_movie_clip_test.swf > $@
	chmod 755 $@


event_handler_scope_test_SOURCES =	\
	event_handler_scope_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
event_handler_scope_test_LDADD = $(MING_LIBS)

event_handler_scope_test.swf: event_handler_scope_test
	./event_handler_scope_test $(top_srcdir)/testsuite/media

event_handler_scope_testrunner: $(srcdir)/../generic-testrunner.sh event_handler_scope_test.swf
	sh $< -r5 $(top_builddir) event_handler_scope_test.swf > $@
	chmod 755 $@
		
attachMovieTest_SOURCES =	\
	attachMovieTest.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
attachMovieTest_LDADD = $(MING_LIBS)

attachMovieTest.swf: attachMovieTest
	./attachMovieTest $(top_srcdir)/testsuite/media

attachMovieTestRunner_SOURCES = \
	attachMovieTestRunner.cpp \
	$(NULL)
attachMovieTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
attachMovieTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
attachMovieTestRunner_DEPENDENCIES = \
	attachMovieTest.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

attachMovieLoopingTest_SOURCES =	\
	attachMovieLoopingTest.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
attachMovieLoopingTest_LDADD = $(MING_LIBS)

attachMovieLoopingTest.swf: attachMovieLoopingTest
	./attachMovieLoopingTest $(top_srcdir)/testsuite/media

attachMovieLoopingTestRunner_SOURCES = \
	attachMovieLoopingTestRunner.cpp \
	$(NULL)
attachMovieLoopingTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
attachMovieLoopingTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
attachMovieLoopingTestRunner_DEPENDENCIES = \
	attachMovieLoopingTest.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

simple_loop_test_SOURCES =	\
	simple_loop_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
simple_loop_test_LDADD = $(MING_LIBS)

simple_loop_test.swf: simple_loop_test
	./simple_loop_test $(top_srcdir)/testsuite/media

simple_loop_testrunner_SOURCES = \
	simple_loop_testrunner.cpp \
	$(NULL)
simple_loop_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
simple_loop_testrunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
simple_loop_testrunner_DEPENDENCIES = \
	simple_loop_test.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)


goto_frame_test_SOURCES =	\
	goto_frame_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
goto_frame_test_LDADD = $(MING_LIBS)

goto_frame_test.swf: goto_frame_test
	./goto_frame_test $(top_srcdir)/testsuite/media

goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh goto_frame_test.swf
	sh $< $(top_builddir) goto_frame_test.swf > $@
	chmod 755 $@

consecutive_goto_frame_test_SOURCES =	\
	consecutive_goto_frame_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
consecutive_goto_frame_test_LDADD = $(MING_LIBS)

consecutive_goto_frame_test.swf: consecutive_goto_frame_test
	./consecutive_goto_frame_test $(top_srcdir)/testsuite/media

consecutive_goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh consecutive_goto_frame_test.swf
	sh $< $(top_builddir) consecutive_goto_frame_test.swf > $@
	chmod 755 $@

multi_doactions_and_goto_frame_test_SOURCES =	\
	multi_doactions_and_goto_frame_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
multi_doactions_and_goto_frame_test_LDADD = $(MING_LIBS)

multi_doactions_and_goto_frame_test.swf: multi_doactions_and_goto_frame_test
	./multi_doactions_and_goto_frame_test $(top_srcdir)/testsuite/media

multi_doactions_and_goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh multi_doactions_and_goto_frame_test.swf
	sh $< $(top_builddir) multi_doactions_and_goto_frame_test.swf > $@
	chmod 755 $@

action_execution_order_test_SOURCES =	\
	action_execution_order_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test_LDADD = $(MING_LIBS)

action_execution_order_test.swf: action_execution_order_test
	./action_execution_order_test $(top_srcdir)/testsuite/media

action_execution_order_testrunner: $(srcdir)/../generic-testrunner.sh action_execution_order_test.swf
	sh $< $(top_builddir) action_execution_order_test.swf > $@
	chmod 755 $@

action_execution_order_test1_SOURCES =	\
	action_execution_order_test1.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test1_LDADD = $(MING_LIBS)

action_execution_order_test1.swf: action_execution_order_test1
	./action_execution_order_test1 $(top_srcdir)/testsuite/media

action_execution_order_test1runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test1.swf
	sh $< $(top_builddir) action_execution_order_test1.swf > $@
	chmod 755 $@

action_execution_order_test2_SOURCES =	\
	action_execution_order_test2.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test2_LDADD = $(MING_LIBS)

action_execution_order_test2.swf: action_execution_order_test2
	./action_execution_order_test2 $(top_srcdir)/testsuite/media

action_execution_order_test2runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test2.swf
	sh $< $(top_builddir) action_execution_order_test2.swf > $@
	chmod 755 $@

action_execution_order_test3_SOURCES =	\
	action_execution_order_test3.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test3_LDADD = $(MING_LIBS)

action_execution_order_test3.swf: action_execution_order_test3
	./action_execution_order_test3 $(top_srcdir)/testsuite/media

action_execution_order_test3runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test3.swf
	sh $< $(top_builddir) action_execution_order_test3.swf > $@
	chmod 755 $@

action_execution_order_test4_SOURCES =	\
	action_execution_order_test4.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test4_LDADD = $(MING_LIBS)

action_execution_order_test4.swf: action_execution_order_test4
	./action_execution_order_test4 $(top_srcdir)/testsuite/media

action_execution_order_test4runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test4.swf
	sh $< $(top_builddir) action_execution_order_test4.swf > $@
	chmod 755 $@

action_execution_order_test5_SOURCES =	\
	action_execution_order_test5.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test5_LDADD = $(MING_LIBS)

action_execution_order_test5.swf: action_execution_order_test5
	./action_execution_order_test5 $(top_srcdir)/testsuite/media

action_execution_order_test5runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test5.swf
	sh $< $(top_builddir) action_execution_order_test5.swf > $@
	chmod 755 $@

action_execution_order_test6_SOURCES =	\
	action_execution_order_test6.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test6_LDADD = $(MING_LIBS)

action_execution_order_test6.swf: action_execution_order_test6
	./action_execution_order_test6 $(top_srcdir)/testsuite/media

action_execution_order_test6runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test6.swf
	sh $< $(top_builddir) action_execution_order_test6.swf > $@
	chmod 755 $@

action_execution_order_test7_SOURCES =	\
	action_execution_order_test7.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test7_LDADD = $(MING_LIBS)

action_execution_order_test7.swf: action_execution_order_test7
	./action_execution_order_test7 $(top_srcdir)/testsuite/media

action_execution_order_test7runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test7.swf
	sh $< $(top_builddir) action_execution_order_test7.swf > $@
	chmod 755 $@

action_execution_order_test8_SOURCES =	\
	action_execution_order_test8.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_test8_LDADD = $(MING_LIBS)

action_execution_order_test8.swf: action_execution_order_test8
	./action_execution_order_test8 $(top_srcdir)/testsuite/media

action_execution_order_test8runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test8.swf
	sh $< -f8 $(top_builddir) action_execution_order_test8.swf > $@
	chmod 755 $@

reverse_execute_PlaceObject2_test1_SOURCES =	\
	reverse_execute_PlaceObject2_test1.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
reverse_execute_PlaceObject2_test1_LDADD = $(MING_LIBS)

reverse_execute_PlaceObject2_test1.swf: reverse_execute_PlaceObject2_test1
	./reverse_execute_PlaceObject2_test1 $(top_srcdir)/testsuite/media

reverse_execute_PlaceObject2_test1runner: $(srcdir)/../generic-testrunner.sh reverse_execute_PlaceObject2_test1.swf
	sh $< $(top_builddir) reverse_execute_PlaceObject2_test1.swf > $@
	chmod 755 $@

reverse_execute_PlaceObject2_test2_SOURCES =	\
	reverse_execute_PlaceObject2_test2.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
reverse_execute_PlaceObject2_test2_LDADD = $(MING_LIBS)

reverse_execute_PlaceObject2_test2.swf: reverse_execute_PlaceObject2_test2
	./reverse_execute_PlaceObject2_test2 $(top_srcdir)/testsuite/media

reverse_execute_PlaceObject2_test2runner: $(srcdir)/../generic-testrunner.sh reverse_execute_PlaceObject2_test2.swf
	sh $< $(top_builddir) reverse_execute_PlaceObject2_test2.swf > $@
	chmod 755 $@

action_execution_order_extend_test_SOURCES =	\
	action_execution_order_extend_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
action_execution_order_extend_test_LDADD = $(MING_LIBS)

action_execution_order_extend_test.swf: action_execution_order_extend_test
	./action_execution_order_extend_test $(top_srcdir)/testsuite/media

action_execution_order_extend_testrunner: $(srcdir)/../generic-testrunner.sh action_execution_order_extend_test.swf
	sh $< $(top_builddir) action_execution_order_extend_test.swf > $@
	chmod 755 $@

registerClassTest_SOURCES =	\
	registerClassTest.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
registerClassTest_LDADD = $(MING_LIBS)

registerClassTest.swf: registerClassTest
	./registerClassTest $(top_srcdir)/testsuite/media

registerClassTestRunner_SOURCES = \
	registerClassTestRunner.cpp \
	$(NULL)
registerClassTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
registerClassTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
registerClassTestRunner_DEPENDENCIES = \
	registerClassTest.swf \
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
RollOverOutTest_Runner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' 
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
ButtonEventsTest_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

SpriteButtonEventsTest_SOURCES =	\
	SpriteButtonEventsTest.c	\
	ming_utils.h	\
	ming_utils.c	\
	$(NULL)
SpriteButtonEventsTest_LDADD = $(MING_LIBS)
SpriteButtonEventsTest.swf: SpriteButtonEventsTest
	./SpriteButtonEventsTest $(top_srcdir)/testsuite/media
SpriteButtonEventsTest_Runner_SOURCES = \
	SpriteButtonEventsTest-Runner.cpp \
	$(NULL)
SpriteButtonEventsTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
SpriteButtonEventsTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	SpriteButtonEventsTest.swf	\
	$(NULL)
SpriteButtonEventsTest_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

loadMovieTest_SOURCES =	\
	loadMovieTest.c	\
	ming_utils.h	\
	ming_utils.c	\
	$(NULL)
loadMovieTest_LDADD = $(MING_LIBS)
loadMovieTest.swf: loadMovieTest
	./loadMovieTest $(top_srcdir)/testsuite/media
loadMovieTestRunner_SOURCES = \
	loadMovieTestRunner.cpp \
	$(NULL)
loadMovieTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
loadMovieTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	-DMEDIADIR='"$(top_srcdir)/testsuite/media"'
	$(NULL)
loadMovieTestRunner_DEPENDENCIES = \
	loadMovieTest.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

#loadMovieTest_Runner_SOURCES = \
#	loadMovieTest-Runner.cpp \
#	$(NULL)
#loadMovieTest_Runner_LDADD = \
#	$(top_builddir)/testsuite/libtestsuite.la \
#	$(NULL)
#loadMovieTest_Runner_DEPENDENCIES = \
#	$(top_builddir)/testsuite/libtestsuite.la \
#	loadMovieTest.swf	\
#	$(NULL)

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
VarAndCharClashTest.swf: $(srcdir)/../media/green.swf ../Dejagnu.swf $(srcdir)/VarAndCharClashTest.as 
	$(MAKESWF) -o $@ ../Dejagnu.swf $(srcdir)/../media/green.swf $(srcdir)/VarAndCharClashTest.as
VarAndCharClashTest-Runner: $(srcdir)/../generic-testrunner.sh VarAndCharClashTest.swf
	sh $< $(top_builddir) VarAndCharClashTest.swf > $@
	chmod 755 $@

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
FlashVarsTest.swf: ../Dejagnu.swf $(srcdir)/FlashVarsTest.as 
	$(MAKESWF) -o $@ ../Dejagnu.swf $(srcdir)/FlashVarsTest.as
FlashVarsTest-Runner: $(srcdir)/../generic-testrunner.sh FlashVarsTest.swf
	echo "$(top_builddir)/gui/gnash -1 -r0 -v -P 'FlashVars=?a=a_in_fvars&v=v_in_fvars' 'FlashVarsTest.swf?a=a_in_qstring&q=q_in_qstring'" > $@
	chmod 755 $@

init_action_test_SOURCES =	\
	init_action_test.c	\
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)
init_action_test_LDADD = $(MING_LIBS)

init_action_test.swf: init_action_test
	./init_action_test $(top_srcdir)/testsuite/media

init_action_testrunner: $(srcdir)/../generic-testrunner.sh init_action_test.swf
	sh $< $(top_builddir) init_action_test.swf > $@
	chmod 755 $@

intervalTest.swf: $(srcdir)/../media/green.swf ../Dejagnu.swf $(srcdir)/intervalTest.as 
	$(MAKESWF) -o $@ ../Dejagnu.swf $(srcdir)/intervalTest.as
intervalTestRunner_SOURCES = \
	intervalTestRunner.cpp \
	$(NULL)
intervalTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
intervalTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
intervalTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	intervalTest.swf	\
	$(NULL)

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
# Uses a low frame rate to test immediate redraw on key event
KeyTest.swf: $(srcdir)/../media/green.swf ../Dejagnu.swf $(srcdir)/KeyTest.as 
	$(MAKESWF) -o $@ ../Dejagnu.swf $(srcdir)/../media/green.swf $(srcdir)/KeyTest.as
#
KeyTest_Runner_SOURCES = \
	KeyTest-Runner.cpp \
	$(NULL)
KeyTest_Runner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
KeyTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
KeyTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	KeyTest.swf	\
	$(NULL)

gotoFrame2Test.swf: $(srcdir)/../media/green.swf ../Dejagnu.swf $(srcdir)/gotoFrame2Test.as 
	$(MAKESWF) -o $@ -i../Dejagnu.swf:dejagnu $(srcdir)/../actionscript.all/dejagnu_so_init.as \
		$(srcdir)/gotoFrame2Test.as 

gotoFrame2Test-Runner: $(srcdir)/../generic-testrunner.sh gotoFrame2Test.swf
	sh $< $(top_builddir) gotoFrame2Test.swf > $@
	chmod 755 $@


DrawingApiTest.swf: $(srcdir)/DrawingApiTest.as 
	$(MAKESWF) -r12 -o $@ $(srcdir)/DrawingApiTest.as

DrawingApiTestRunner_SOURCES = \
	DrawingApiTestRunner.cpp \
	$(NULL)
DrawingApiTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
DrawingApiTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)
DrawingApiTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	DrawingApiTest.swf	\
	$(NULL)


static_vs_dynamic1_SOURCES = \
	static_vs_dynamic1.c \
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)

static_vs_dynamic1_LDADD = $(MING_LIBS)

static_vs_dynamic1.swf: static_vs_dynamic1
	./static_vs_dynamic1 $(srcdir)/../media

static_vs_dynamic1_testrunner: $(srcdir)/../generic-testrunner.sh static_vs_dynamic1.swf
	sh $< $(top_builddir) static_vs_dynamic1.swf > $@
	chmod 755 $@

static_vs_dynamic2_SOURCES = \
	static_vs_dynamic2.c \
	ming_utils.h		\
	ming_utils.c		\
	$(NULL)

static_vs_dynamic2_LDADD = $(MING_LIBS)

static_vs_dynamic2.swf: static_vs_dynamic2
	./static_vs_dynamic2 $(srcdir)/../media

static_vs_dynamic2_testrunner: $(srcdir)/../generic-testrunner.sh static_vs_dynamic2.swf
	sh $< $(top_builddir) static_vs_dynamic2.swf > $@
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
	-rm *.swf media *.pp *unner

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	DefineEditTextVariableNameTest-Runner \
	RollOverOutTest-Runner \
	ButtonEventsTest-Runner \
	SpriteButtonEventsTest-Runner \
	timeline_var_test-Runner \
	root_stop_testrunner \
	loop_test-Runner \
	place_object_testrunner \
	move_object_testrunner \
	place_and_remove_object_testrunner \
	place_and_remove_object_insane_testrunner \
	attachMovieTestRunner \
	attachMovieLoopingTestRunner \
	goto_frame_testrunner \
	consecutive_goto_frame_testrunner \
	multi_doactions_and_goto_frame_testrunner \
	registerClassTestRunner \
	action_execution_order_testrunner \
	action_execution_order_extend_testrunner \
	simple_loop_testrunner \
	loadMovieTestRunner \
	eventSoundTest1-Runner \
	DrawingApiTestRunner \
	action_execution_order_test1runner \
	action_execution_order_test2runner \
	action_execution_order_test3runner \
	action_execution_order_test4runner \
	action_execution_order_test5runner \
	action_execution_order_test6runner \
	action_execution_order_test7runner \
	action_execution_order_test8runner \
	reverse_execute_PlaceObject2_test1runner \
	reverse_execute_PlaceObject2_test2runner \
	displaylist_depths_testrunner \
	displaylist_depths_test2runner \
	displaylist_depths_test3runner \
	matrix_testrunner \
	get_frame_number_testrunner \
	frame_label_testrunner \
	path_format_testrunner \
	duplicate_movie_clip_testrunner \
	event_handler_scope_testrunner \
	masks_testrunner \
	key_event_testrunner \
	static_vs_dynamic1_testrunner \
	static_vs_dynamic2_testrunner \
	$(NULL)

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
TEST_CASES += KeyTest-Runner \
	VarAndCharClashTest-Runner \
	FlashVarsTest-Runner \
	intervalTestRunner \
	gotoFrame2Test-Runner 
endif

if MING_SUPPORTS_INIT_ACTIONS
TEST_CASES += \
	init_action_testrunner \
	$(NULL)
endif

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

