## Process this file with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AUTOMAKE_OPTIONS = dejagnu -Wno-portability

abs_top_builddir=$(shell cd $(top_builddir); pwd)
abs_builddir=$(shell cd $(top_builddir)/testsuite/misc-ming.all; pwd)
abs_mediadir = $(shell cd $(srcdir)/../media; pwd)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	testrun.sum \
	testrun.log

EXTRA_DIST = \
	empty.as \
	VarAndCharClashTest.as \
	PrototypeEventListeners.as \
	DragDropTest.as \
	remoting.as \
	remoting.php \
	gotoFrame2Test.as \
	DrawingApiTest.as \
	GradientFillTest.as \
	LC-Receive.as \
	LC-Send.as \
	FlashVarsTest.as \
	SharedObjectTest.as \
	intervalTest.as \
	FlashVarsTest.html \
	StageConfigTest.as \
	SharedObjectTest.as \
	SharedObjectTestRunner.sh \
	SharedObjectTest.sol/sol1.sol \
	SharedObjectTest.sol/README 


AM_CPPFLAGS = \
	-I$(top_srcdir)/libbase \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libsound \
	-I$(top_srcdir)/librender \
	-I$(top_srcdir)/libcore  \
	-I$(top_srcdir)/libcore/swf  \
	-I$(top_srcdir)/libcore/parser  \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/asobj \
	-I$(top_srcdir)/testsuite \
	$(MING_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(BOOST_CFLAGS) \
	-DMING_VERSION_CODE=$(MING_VERSION_CODE) \
	$(NULL)

AM_LDFLAGS = \
	$(MING_LIBS) \
	$(OPENGL_LIBS) \
	$(GIF_LIBS) \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/librender/libgnashrender.la \
	$(NULL)

if LIBLTDL2
AM_LDFLAGS  += $(top_builddir)/libltdl/libltdlc.la
endif
if LIBLTDL1
AM_LDFLAGS += $(top_builddir)/libbase/libltdlc.la
endif

check_PROGRAMS = \
	SpriteButtonEventsTest \
	SpriteButtonEventsTest-Runner \
	DefineTextTest \
	DefineTextTest-Runner \
	TextSnapshotTest \
	DeviceFontTest \
	EmbeddedFontTest \
	DefineEditTextTest \
	DefineEditTextTest-Runner \
	DefineEditTextVariableNameTest \
	DefineEditTextVariableNameTest-Runner \
	DefineEditTextVariableNameTest2 \
	Dejagnu \
	Dejagnu.swf \
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
	loop_test2 \
	loop_test2runner \
	loop_test3 \
	loop_test4 \
	loop_test5 \
	loop_test6 \
	loop_test7 \
	loop_test8 \
	loop_test9 \
	loop_test10 \
	loop_test-Runner \
	place_object_test \
	place_object_test2 \
	move_object_test \
	place_and_remove_object_test \
	place_and_remove_object_insane_test \
	unload_movieclip_test1 \
	displaylist_depths_test \
	displaylist_depths_test2 \
	displaylist_depths_test3 \
	displaylist_depths_test4 \
	displaylist_depths_test5 \
	displaylist_depths_test6 \
	displaylist_depths_test7 \
	displaylist_depths_test8 \
	displaylist_depths_test9 \
	displaylist_depths_test10 \
	displaylist_depths_test11 \
	DepthLimitsTest \
	masks_test \
	masks_test2 \
	masks_test2runner \
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
	action_execution_order_test9 \
	action_execution_order_test11 \
	reverse_execute_PlaceObject2_test1 \
	reverse_execute_PlaceObject2_test2 \
	loadImageTest \
	loadMovieTest \
	loadMovieTestRunner \
	DrawingApiTestRunner \
	frame_label_test \
	path_format_test \
	callFunction_test \
	duplicate_movie_clip_test \
	duplicate_movie_clip_test2 \
	event_handler_scope_test \
	masks_testrunner \
	KeyEventOrder \
	key_event_test \
	KeyEventOrderRunner \
	key_event_testrunner \
	static_vs_dynamic1 \
	static_vs_dynamic2 \
	getTimer_test \
	Video-EmbedSquareTest \
	LoadVarsTest \
	Video-EmbedSquareTestRunner \
	NetStream-SquareTest \
	NetStream-SquareTestRunner \
	shape_test \
	morph_test1 \
	morph_test1runner \
	runtime_vm_stack_test \
	new_child_in_unload_test \
	instanceNameTest \
	BeginBitmapFill \
	BeginBitmapFillRunner \
	BitmapDataTest \
	BitmapDataTestRunner \
	BitmapDataDraw \
	BitmapSmoothingTest \
	$(NULL)

if MING_VERSION_0_4_3
check_PROGRAMS += \
	EmbeddedSoundTest \
	eventSoundTest1 \
	eventSoundTest1-Runner \
	$(NULL)
endif

if MING_VERSION_0_4
check_PROGRAMS += \
	opcode_guard_test \
	get_frame_number_test
endif

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
check_PROGRAMS += \
	intervalTestRunner \
	DragDropTestRunner   \
	PrototypeEventListenersTestRunner \
	$(NULL)
endif

if MING_SUPPORTS_INIT_ACTIONS
check_PROGRAMS += \
	init_action_test \
	InitActionTest2 \
	ActionOrderTest3 \
	ActionOrderTest4 \
	ActionOrderTest5 \
	RegisterClassTest4 \
	RegisterClassTest3 \
	registerClassTest2 \
	$(NULL)
endif

if MING_SUPPORTS_REPLACE_TAG
check_PROGRAMS += \
	replace_shapes1test \
	replace_sprites1test \
	replace_buttons1test \
	$(NULL)
endif

if MING_SUPPORTS_STREAMING_SOUND
check_PROGRAMS += \
	streamingSoundTest1 \
	streamingSoundTest1-Runner 
endif

if MING_SUPPORTS_SWFBUTTON_SET_DEPTH
check_PROGRAMS += \
	ButtonPropertiesTest \
	ButtonEventsTest \
	ButtonPropertiesTestRunner \
	ButtonEventsTest-Runner \
	$(NULL)
endif

if MING_VERSION_0_4_3
check_PROGRAMS += EmbeddedSoundTest-Runner
endif

check_SCRIPTS = \
	BitmapDataDrawRunner \
	DeviceFontTestRunner \
	EmbeddedFontTestRunner \
	TextSnapshotTest-Runner \
	XMLSocketTester \
	LCTestRunner \
	timeline_var_test-Runner \
	place_object_testrunner \
	place_object_test2runner \
	move_object_testrunner \
	place_and_remove_object_testrunner \
	place_and_remove_object_insane_testrunner \
	unload_movieclip_test1runner \
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
	action_execution_order_test9runner \
	action_execution_order_test11runner \
	reverse_execute_PlaceObject2_test1runner \
	reverse_execute_PlaceObject2_test2runner \
	displaylist_depths_testrunner \
	displaylist_depths_test2runner \
	displaylist_depths_test3runner \
	displaylist_depths_test4runner \
	displaylist_depths_test5runner \
	displaylist_depths_test6runner \
	displaylist_depths_test7runner \
	displaylist_depths_test8runner \
	displaylist_depths_test9runner \
	displaylist_depths_test10runner \
	displaylist_depths_test11runner \
	frame_label_testrunner \
	path_format_testrunner \
	callFunction_testrunner \
	duplicate_movie_clip_testrunner \
	duplicate_movie_clip_test2runner \
	event_handler_scope_testrunner \
	matrix_testrunner \
	static_vs_dynamic1_testrunner \
	static_vs_dynamic2_testrunner \
	getTimer_testrunner \
	loop_test3runner \
	loop_test4runner \
	loop_test5runner \
	loop_test6runner \
	loop_test7runner \
	loop_test8runner \
	loop_test9runner \
	DepthLimitsTestrunner \
	loop_test10runner \
	shape_testrunner \
	LoadVarsTestRunner \
	DefineEditTextVariableNameTest-Runner \
	DefineEditTextVariableNameTest2-Runner \
	runtime_vm_stack_testrunner \
	new_child_in_unload_testrunner \
	instanceNameTestRunner \
	init_action_testrunner \
	InitActionTest2Runner \
	ActionOrderTest3Runner \
	ActionOrderTest4Runner \
	ActionOrderTest5Runner \
	$(NULL)

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
check_SCRIPTS += VarAndCharClashTest-Runner \
	gotoFrame2Test-Runner \
	FlashVarsTest-Runner \
	attachImported-Runner \
	attachExtImported-Runner \
	SharedObjectTestRunner
endif

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
if ENABLE_HTTP_TESTSUITE
check_SCRIPTS += remotingTestRunner
endif
endif

# check_SCRIPTS += netstreamTestRunner

if MING_VERSION_0_4
check_SCRIPTS += \
	opcode_guard_testrunner \
	get_frame_number_testrunner \
	$(NULL)	
endif

if MING_SUPPORTS_INIT_ACTIONS
check_SCRIPTS += \
	InitActionTest2Runner \
	ActionOrderTest3Runner \
	ActionOrderTest4Runner \
	ActionOrderTest5Runner \
	RegisterClassTest4Runner \
	RegisterClassTest3Runner \
	registerClassTest2runner \
	$(NULL)
endif

if MING_SUPPORTS_REPLACE_TAG
check_SCRIPTS += \
	PlaceObject2TestRunner \
	$(NULL)
check_PROGRAMS += \
	PlaceObject2Test \
	replace_shapes1test_runner \
	replace_sprites1test_runner \
	replace_buttons1test_runner \
	$(NULL)
endif


# TESTS = \
# 	DefineEditTextVariableNameTest-Runner 

check_LTLIBRARIES = libgnashmingutils.la 
libgnashmingutils_la_SOURCES =	\
	ming_utils.h		\
	ming_utils.c

definebitsjpeg2_SOURCES = definebitsjpeg2.c
definebitsjpeg2_LDADD = $(MING_LIBS)
definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2 $(srcdir)/../media/lynch.jpg

moviecliploader_test_SOURCES =  moviecliploader_test.c
moviecliploader_test_LDADD = $(MING_LIBS)

#FIXME: make this rule consistent with the others 
#and write a test runner for the absolute *and* relative
#versions
moviecliploader_tests: moviecliploader_test
	: test with relative urls 
	ln -fs $(abs_mediadir) media
	./moviecliploader_test media
	mv moviecliploader_test.swf moviecliploader_test_relative.swf
	: test absolute url
	./moviecliploader_test $(abs_mediadir)

DefineTextTest.swf: DefineTextTest
	./DefineTextTest $(abs_mediadir)

DefineTextTest_SOURCES =  DefineTextTest.c
DefineTextTest_LDADD = libgnashmingutils.la

DefineTextTest_Runner_SOURCES = \
	DefineTextTest-Runner.cpp \
	$(NULL)
DefineTextTest_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
DefineTextTest_Runner_LDADD =  \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
DefineTextTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	DefineTextTest.swf      \
	$(NULL)

TextSnapshotTest.swf: TextSnapshotTest
	./TextSnapshotTest $(abs_mediadir)

TextSnapshotTest_SOURCES =  TextSnapshotTest.c
TextSnapshotTest_LDADD = libgnashmingutils.la

TextSnapshotTest-Runner: $(srcdir)/../generic-testrunner.sh TextSnapshotTest.swf
	sh $< $(top_builddir) TextSnapshotTest.swf > $@
	chmod 755 $@

DeviceFontTestRunner: $(srcdir)/../generic-testrunner.sh DeviceFontTest.swf
	sh $< $(top_builddir) DeviceFontTest.swf > $@
	chmod 755 $@
DeviceFontTest.swf: DeviceFontTest
	./DeviceFontTest $(abs_mediadir)
DeviceFontTest_SOURCES =  DeviceFontTest.c
DeviceFontTest_LDADD = libgnashmingutils.la

EmbeddedFontTestRunner: $(srcdir)/../generic-testrunner.sh EmbeddedFontTest.swf
	sh $< $(top_builddir) EmbeddedFontTest.swf > $@
	chmod 755 $@
EmbeddedFontTest.swf: EmbeddedFontTest
	./EmbeddedFontTest $(abs_mediadir)
EmbeddedFontTest_SOURCES =  EmbeddedFontTest.c
EmbeddedFontTest_LDADD = libgnashmingutils.la

DefineEditTextTest.swf: DefineEditTextTest
	./DefineEditTextTest $(abs_mediadir)
DefineEditTextTest_SOURCES =  DefineEditTextTest.c
DefineEditTextTest_LDADD = libgnashmingutils.la

DefineEditTextTest_Runner_SOURCES = \
	DefineEditTextTest-Runner.cpp \
	$(NULL)
DefineEditTextTest_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
DefineEditTextTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	 $(AM_LDFLAGS) \
	$(NULL)
DefineEditTextTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	DefineEditTextTest.swf      \
	$(NULL)

spritehier_SOURCES =  spritehier.c
spritehier_LDADD = $(MING_LIBS)

opcode_guard_test_SOURCES =	\
	opcode_guard_test.c	\
	$(NULL)
opcode_guard_test_LDADD = libgnashmingutils.la

opcode_guard_test.swf: opcode_guard_test
	./opcode_guard_test $(abs_mediadir)

opcode_guard_testrunner: $(srcdir)/../generic-testrunner.sh opcode_guard_test.swf
	sh $< $(top_builddir) opcode_guard_test.swf > $@
	chmod 755 $@

runtime_vm_stack_test_SOURCES =	\
	runtime_vm_stack_test.c	\
	$(NULL)
runtime_vm_stack_test_LDADD = libgnashmingutils.la

runtime_vm_stack_test.swf: runtime_vm_stack_test
	./runtime_vm_stack_test $(abs_mediadir)

runtime_vm_stack_testrunner: $(srcdir)/../generic-testrunner.sh runtime_vm_stack_test.swf
	sh $< $(top_builddir) runtime_vm_stack_test.swf > $@
	chmod 755 $@

new_child_in_unload_test_SOURCES =	\
	new_child_in_unload_test.c	\
	$(NULL)
new_child_in_unload_test_LDADD = libgnashmingutils.la

new_child_in_unload_test.swf: new_child_in_unload_test
	./new_child_in_unload_test $(abs_mediadir)

new_child_in_unload_testrunner: $(srcdir)/../generic-testrunner.sh new_child_in_unload_test.swf
	sh $< $(top_builddir) new_child_in_unload_test.swf > $@
	chmod 755 $@

PlaceObject2Test_SOURCES =	\
	PlaceObject2Test.c	\
	$(NULL)
PlaceObject2Test_LDADD = libgnashmingutils.la

PlaceObject2Test.swf: PlaceObject2Test
	./PlaceObject2Test $(abs_mediadir)

PlaceObject2TestRunner: $(srcdir)/../generic-testrunner.sh PlaceObject2Test.swf
	sh $< -r5 $(top_builddir) PlaceObject2Test.swf > $@
	chmod 755 $@

RemoveObject2Test_SOURCES = RemoveObject2Test.c
RemoveObject2Test_LDADD = libgnashmingutils.la

eventSoundTest1_SOURCES = eventSoundTest1.c
eventSoundTest1_LDADD = libgnashmingutils.la

eventSoundTest1.swf: eventSoundTest1
	./eventSoundTest1 $(srcdir)/../media/brokenchord.wav \
	$(srcdir)/../media/

eventSoundTest1_Runner_SOURCES = \
	eventSoundTest1-Runner.cpp \
	$(NULL)
eventSoundTest1_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
eventSoundTest1_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
eventSoundTest1_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	eventSoundTest1.swf      \
	$(NULL)

streamingSoundTest1_SOURCES = streamingSoundTest1.c
streamingSoundTest1_LDADD = libgnashmingutils.la

streamingSoundTest1.swf: streamingSoundTest1
	./streamingSoundTest1 $(srcdir)/../media/sound1.mp3

streamingSoundTest1_Runner_SOURCES = \
	streamingSoundTest1-Runner.cpp \
	$(NULL)
streamingSoundTest1_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
streamingSoundTest1_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
streamingSoundTest1_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	streamingSoundTest1.swf      \
	$(NULL)


DefineEditTextVariableNameTest_SOURCES =	\
	DefineEditTextVariableNameTest.c	\
	$(NULL)
DefineEditTextVariableNameTest_LDADD = libgnashmingutils.la

DefineEditTextVariableNameTest.swf: DefineEditTextVariableNameTest
	./DefineEditTextVariableNameTest $(abs_mediadir)

DefineEditTextVariableNameTest_Runner_SOURCES = \
	DefineEditTextVariableNameTest-Runner.cpp \
	$(NULL)
DefineEditTextVariableNameTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
DefineEditTextVariableNameTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	DefineEditTextVariableNameTest.swf      \
	$(NULL)


DefineEditTextVariableNameTest2_SOURCES =	\
	DefineEditTextVariableNameTest2.c	\
	$(NULL)
DefineEditTextVariableNameTest2_LDADD = libgnashmingutils.la

DefineEditTextVariableNameTest2.swf: DefineEditTextVariableNameTest2
	./DefineEditTextVariableNameTest2 $(abs_mediadir)

DefineEditTextVariableNameTest2-Runner: $(srcdir)/../generic-testrunner.sh DefineEditTextVariableNameTest2.swf
	sh $< $(top_builddir) DefineEditTextVariableNameTest2.swf > $@
	chmod 755 $@

timeline_var_test_SOURCES =	\
	timeline_var_test.c	\
	$(NULL)
timeline_var_test_LDADD = libgnashmingutils.la

timeline_var_test.swf: timeline_var_test
	./timeline_var_test $(abs_mediadir)

timeline_var_test-Runner: $(srcdir)/../generic-testrunner.sh timeline_var_test.swf
	sh $< $(top_builddir) timeline_var_test.swf > $@
	chmod 755 $@

root_stop_test_SOURCES = root_stop_test.c
root_stop_test_LDADD = libgnashmingutils.la

root_stop_test.swf: root_stop_test
	./root_stop_test $(abs_mediadir)

root_stop_testrunner_SOURCES = \
	root_stop_testrunner.cpp \
	$(NULL)
root_stop_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
root_stop_testrunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' 
root_stop_testrunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	root_stop_test.swf      \
	$(NULL)

loop_test_SOURCES = loop_test.c	
loop_test_LDADD = libgnashmingutils.la

loop_test.swf: loop_test
	./loop_test $(abs_mediadir)

loop_test_Runner_SOURCES = loop_test-Runner.cpp 
loop_test_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

loop_test_Runner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

loop_test_Runner_DEPENDENCIES = \
	loop_test.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

loop_test2_SOURCES = loop_test2.c	
loop_test2_LDADD = libgnashmingutils.la

loop_test2.swf: loop_test2
	./loop_test2 $(abs_mediadir)

loop_test2runner_SOURCES = \
	loop_test2runner.cpp \
	$(NULL)
loop_test2runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
loop_test2runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
loop_test2runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	loop_test2.swf      \
	$(NULL)

loop_test3_SOURCES = loop_test3.c
loop_test3_LDADD = libgnashmingutils.la

loop_test3.swf: loop_test3
	./loop_test3 $(abs_mediadir)

loop_test3runner: $(srcdir)/../generic-testrunner.sh loop_test3.swf
	sh $< -f20 $(top_builddir) loop_test3.swf > $@
	chmod 755 $@

loop_test4_SOURCES = loop_test4.c
loop_test4_LDADD = libgnashmingutils.la

loop_test4.swf: loop_test4
	./loop_test4 $(abs_mediadir)

loop_test4runner: $(srcdir)/../generic-testrunner.sh loop_test4.swf
	sh $< -f20 $(top_builddir) loop_test4.swf > $@
	chmod 755 $@

loop_test5_SOURCES = loop_test5.c
loop_test5_LDADD = libgnashmingutils.la

loop_test5.swf: loop_test5
	./loop_test5 $(abs_mediadir)

loop_test5runner: $(srcdir)/../generic-testrunner.sh loop_test5.swf
	sh $< -f20 $(top_builddir) loop_test5.swf > $@
	chmod 755 $@

loop_test6_SOURCES = loop_test6.c
loop_test6_LDADD = libgnashmingutils.la

loop_test6.swf: loop_test6
	./loop_test6 $(abs_mediadir)

loop_test6runner: $(srcdir)/../generic-testrunner.sh loop_test6.swf
	sh $< -f20 $(top_builddir) loop_test6.swf > $@
	chmod 755 $@

loop_test7_SOURCES = loop_test7.c
loop_test7_LDADD = libgnashmingutils.la

loop_test7.swf: loop_test7
	./loop_test7 $(abs_mediadir)

loop_test7runner: $(srcdir)/../generic-testrunner.sh loop_test7.swf
	sh $< -f20 $(top_builddir) loop_test7.swf > $@
	chmod 755 $@

loop_test8_SOURCES = loop_test8.c
loop_test8_LDADD = libgnashmingutils.la

loop_test8.swf: loop_test8
	./loop_test8 $(abs_mediadir)

loop_test8runner: $(srcdir)/../generic-testrunner.sh loop_test8.swf
	sh $< -f20 $(top_builddir) loop_test8.swf > $@
	chmod 755 $@

loop_test9_SOURCES = loop_test9.c
loop_test9_LDADD = libgnashmingutils.la

loop_test9.swf: loop_test9
	./loop_test9 $(abs_mediadir)

loop_test9runner: $(srcdir)/../generic-testrunner.sh loop_test9.swf Makefile
	sh $< -r10 -f20 -c "#total tests" $(top_builddir) loop_test9.swf > $@
	chmod 755 $@


loop_test10_SOURCES = loop_test10.c
loop_test10_LDADD = libgnashmingutils.la

loop_test10.swf: loop_test10
	./loop_test10 $(abs_mediadir)

loop_test10runner: $(srcdir)/../generic-testrunner.sh loop_test10.swf Makefile
	sh $<  -f10 -c "#total tests" $(top_builddir) loop_test10.swf > $@
	chmod 755 $@

shape_test_SOURCES = shape_test.c
shape_test_LDADD = libgnashmingutils.la

shape_test.swf: shape_test
	./shape_test $(abs_mediadir)

shape_testrunner: $(srcdir)/../generic-testrunner.sh shape_test.swf
	sh $< -f20 $(top_builddir) shape_test.swf > $@
	chmod 755 $@

morph_test1_SOURCES = morph_test1.c
morph_test1_LDADD = libgnashmingutils.la

morph_test1.swf: morph_test1
	./morph_test1 $(abs_mediadir)

morph_test1runner_SOURCES = \
	morph_test1runner.cpp \
	$(NULL)
morph_test1runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
morph_test1runner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
morph_test1runner_DEPENDENCIES = \
	morph_test1.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

key_event_test_SOURCES = key_event_test.c
key_event_test_LDADD = libgnashmingutils.la

key_event_test.swf: key_event_test
	./key_event_test $(abs_mediadir)

key_event_testrunner_SOURCES = \
	key_event_testrunner.cpp \
	$(NULL)
key_event_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
key_event_testrunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
key_event_testrunner_DEPENDENCIES = \
	key_event_test.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

KeyEventOrder_SOURCES = KeyEventOrder.c
KeyEventOrder_LDADD = libgnashmingutils.la

KeyEventOrder.swf: KeyEventOrder
	./KeyEventOrder $(abs_mediadir)

KeyEventOrderRunner_SOURCES = \
	KeyEventOrderRunner.cpp \
	$(NULL)
KeyEventOrderRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
KeyEventOrderRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
KeyEventOrderRunner_DEPENDENCIES = \
	KeyEventOrder.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

place_object_test_SOURCES = place_object_test.c	
place_object_test_LDADD = libgnashmingutils.la

place_object_test.swf: place_object_test
	./place_object_test $(abs_mediadir)

place_object_testrunner: $(srcdir)/../generic-testrunner.sh place_object_test.swf
	sh $< -r5 $(top_builddir) place_object_test.swf > $@
	chmod 755 $@


place_object_test2_SOURCES = place_object_test2.c	
place_object_test2_LDADD = libgnashmingutils.la

place_object_test2.swf: place_object_test2
	./place_object_test2 $(abs_mediadir)

place_object_test2runner: $(srcdir)/../generic-testrunner.sh place_object_test2.swf
	sh $< -r5 $(top_builddir) place_object_test2.swf > $@
	chmod 755 $@

move_object_test_SOURCES =	\
	move_object_test.c	\
	$(NULL)
move_object_test_LDADD = libgnashmingutils.la

move_object_test.swf: move_object_test
	./move_object_test $(abs_mediadir)

move_object_testrunner: $(srcdir)/../generic-testrunner.sh move_object_test.swf
	sh $< -r5 $(top_builddir) move_object_test.swf > $@
	chmod 755 $@

place_and_remove_object_test_SOURCES =	\
	place_and_remove_object_test.c	\
	$(NULL)
place_and_remove_object_test_LDADD = libgnashmingutils.la

place_and_remove_object_test.swf: place_and_remove_object_test
	./place_and_remove_object_test $(abs_mediadir)

place_and_remove_object_testrunner: $(srcdir)/../generic-testrunner.sh place_and_remove_object_test.swf
	sh $< -r5 $(top_builddir) place_and_remove_object_test.swf > $@
	chmod 755 $@

place_and_remove_object_insane_test_SOURCES =	\
	place_and_remove_object_insane_test.c	\
	$(NULL)
place_and_remove_object_insane_test_LDADD = libgnashmingutils.la

place_and_remove_object_insane_test.swf: place_and_remove_object_insane_test
	./place_and_remove_object_insane_test $(abs_mediadir)

place_and_remove_object_insane_testrunner: $(srcdir)/../generic-testrunner.sh place_and_remove_object_insane_test.swf
	sh $< -r5 $(top_builddir) place_and_remove_object_insane_test.swf > $@
	chmod 755 $@

unload_movieclip_test1_SOURCES =	\
	unload_movieclip_test1.c	\
	$(NULL)
unload_movieclip_test1_LDADD = libgnashmingutils.la

unload_movieclip_test1.swf: unload_movieclip_test1
	./unload_movieclip_test1 $(abs_mediadir)

unload_movieclip_test1runner: $(srcdir)/../generic-testrunner.sh unload_movieclip_test1.swf
	sh $< -f10 $(top_builddir) unload_movieclip_test1.swf > $@
	chmod 755 $@

displaylist_depths_test_SOURCES =	\
	displaylist_depths_test.c	\
	$(NULL)
displaylist_depths_test_LDADD = libgnashmingutils.la

displaylist_depths_test.swf: displaylist_depths_test
	./displaylist_depths_test $(abs_mediadir)

displaylist_depths_testrunner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test.swf
	sh $< -r5 $(top_builddir) displaylist_depths_test.swf > $@
	chmod 755 $@

displaylist_depths_test2_SOURCES =	\
	displaylist_depths_test2.c	\
	$(NULL)
displaylist_depths_test2_LDADD = libgnashmingutils.la

displaylist_depths_test2.swf: displaylist_depths_test2
	./displaylist_depths_test2 $(abs_mediadir)

displaylist_depths_test2runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test2.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test2.swf > $@
	chmod 755 $@

displaylist_depths_test3_SOURCES =	\
	displaylist_depths_test3.c	\
	$(NULL)
displaylist_depths_test3_LDADD = libgnashmingutils.la

displaylist_depths_test3.swf: displaylist_depths_test3
	./displaylist_depths_test3 $(abs_mediadir)

displaylist_depths_test3runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test3.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test3.swf > $@
	chmod 755 $@

displaylist_depths_test4_SOURCES =	\
	displaylist_depths_test4.c	\
	$(NULL)
displaylist_depths_test4_LDADD = libgnashmingutils.la

displaylist_depths_test4.swf: displaylist_depths_test4
	./displaylist_depths_test4 $(abs_mediadir)

displaylist_depths_test4runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test4.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test4.swf > $@
	chmod 755 $@

displaylist_depths_test5_SOURCES =	\
	displaylist_depths_test5.c	\
	$(NULL)
displaylist_depths_test5_LDADD = libgnashmingutils.la

displaylist_depths_test5.swf: displaylist_depths_test5
	./displaylist_depths_test5 $(abs_mediadir)

displaylist_depths_test5runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test5.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test5.swf > $@
	chmod 755 $@

displaylist_depths_test6_SOURCES =	\
	displaylist_depths_test6.c	\
	$(NULL)
displaylist_depths_test6_LDADD = libgnashmingutils.la

displaylist_depths_test6.swf: displaylist_depths_test6
	./displaylist_depths_test6 $(abs_mediadir)

displaylist_depths_test6runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test6.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test6.swf > $@
	chmod 755 $@

displaylist_depths_test7_SOURCES =	\
	displaylist_depths_test7.c	\
	$(NULL)
displaylist_depths_test7_LDADD = libgnashmingutils.la

displaylist_depths_test7.swf: displaylist_depths_test7
	./displaylist_depths_test7 $(abs_mediadir)

displaylist_depths_test7runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test7.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test7.swf > $@
	chmod 755 $@

displaylist_depths_test8_SOURCES =	\
	displaylist_depths_test8.c	\
	$(NULL)
displaylist_depths_test8_LDADD = libgnashmingutils.la

displaylist_depths_test8.swf: displaylist_depths_test8
	./displaylist_depths_test8 $(abs_mediadir)

displaylist_depths_test8runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test8.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test8.swf > $@
	chmod 755 $@


displaylist_depths_test9_SOURCES =	\
	displaylist_depths_test9.c	\
	$(NULL)
displaylist_depths_test9_LDADD = libgnashmingutils.la

displaylist_depths_test9.swf: displaylist_depths_test9
	./displaylist_depths_test9 $(abs_mediadir)

displaylist_depths_test9runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test9.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test9.swf > $@
	chmod 755 $@

displaylist_depths_test10_SOURCES =	\
	displaylist_depths_test10.c	\
	$(NULL)
displaylist_depths_test10_LDADD = libgnashmingutils.la

displaylist_depths_test10.swf: displaylist_depths_test10
	./displaylist_depths_test10 $(abs_mediadir)

displaylist_depths_test10runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test10.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test10.swf > $@
	chmod 755 $@

displaylist_depths_test11_SOURCES = displaylist_depths_test11.c	
displaylist_depths_test11_LDADD = libgnashmingutils.la

displaylist_depths_test11.swf: displaylist_depths_test11
	./displaylist_depths_test11 $(abs_mediadir)

displaylist_depths_test11runner: $(srcdir)/../generic-testrunner.sh displaylist_depths_test11.swf
	sh $< -f20 $(top_builddir) displaylist_depths_test11.swf > $@
	chmod 755 $@


DepthLimitsTest_SOURCES = DepthLimitsTest.c
DepthLimitsTest_LDADD = libgnashmingutils.la

DepthLimitsTest.swf: DepthLimitsTest
	./DepthLimitsTest $(abs_mediadir)

DepthLimitsTestrunner: $(srcdir)/../generic-testrunner.sh DepthLimitsTest.swf
	sh $< -r5 $(top_builddir) DepthLimitsTest.swf > $@
	chmod 755 $@


replace_shapes1test_SOURCES = replace_shapes1test.c	
replace_shapes1test_LDADD = libgnashmingutils.la

replace_shapes1test.swf: replace_shapes1test
	./replace_shapes1test $(abs_mediadir)

replace_shapes1test_runner_SOURCES = \
	replace_shapes1test_runner.cpp \
	$(NULL)
replace_shapes1test_runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
replace_shapes1test_runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
replace_shapes1test_runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	replace_shapes1test.swf      \
	$(NULL)

replace_sprites1test_SOURCES = replace_sprites1test.c	
replace_sprites1test_LDADD = libgnashmingutils.la

replace_sprites1test.swf: replace_sprites1test
	./replace_sprites1test $(abs_mediadir)

replace_sprites1test_runner_SOURCES = \
	replace_sprites1test_runner.cpp \
	$(NULL)
replace_sprites1test_runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
replace_sprites1test_runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
replace_sprites1test_runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	replace_sprites1test.swf      \
	$(NULL)

replace_buttons1test_SOURCES = replace_buttons1test.c	
replace_buttons1test_LDADD = libgnashmingutils.la

replace_buttons1test.swf: replace_buttons1test
	./replace_buttons1test $(abs_mediadir)

replace_buttons1test_runner_SOURCES = \
	replace_buttons1test_runner.cpp \
	$(NULL)
replace_buttons1test_runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
replace_buttons1test_runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
replace_buttons1test_runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	replace_buttons1test.swf      \
	$(NULL)


masks_test_SOURCES =	\
	masks_test.c	\
	$(NULL)
masks_test_LDADD = libgnashmingutils.la

masks_test.swf: masks_test
	./masks_test $(abs_mediadir)

masks_testrunner_SOURCES = \
	masks_testrunner.cpp \
	$(NULL)
masks_testrunner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
masks_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
masks_testrunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	masks_test.swf      \
	$(NULL)


masks_test2_SOURCES =	\
	masks_test2.c	\
	$(NULL)
masks_test2_LDADD = libgnashmingutils.la

masks_test2.swf: masks_test2
	./masks_test2 $(abs_mediadir)

masks_test2runner_SOURCES = \
	masks_test2runner.cpp \
	$(NULL)
masks_test2runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
masks_test2runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
masks_test2runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	masks_test2.swf      \
	$(NULL)

matrix_test_SOURCES =	\
	matrix_test.c	\
	$(NULL)
matrix_test_LDADD = libgnashmingutils.la

matrix_test.swf: matrix_test
	./matrix_test $(abs_mediadir)

matrix_testrunner: $(srcdir)/../generic-testrunner.sh matrix_test.swf
	sh $< -r5 $(top_builddir) matrix_test.swf > $@
	chmod 755 $@

get_frame_number_test_SOURCES =	\
	get_frame_number_test.c	\
	$(NULL)
get_frame_number_test_LDADD = libgnashmingutils.la

get_frame_number_test.swf: get_frame_number_test
	./get_frame_number_test $(abs_mediadir)

get_frame_number_testrunner: $(srcdir)/../generic-testrunner.sh get_frame_number_test.swf
	sh $< -f20 $(top_builddir) get_frame_number_test.swf > $@
	chmod 755 $@

frame_label_test_SOURCES =	\
	frame_label_test.c	\
	$(NULL)
frame_label_test_LDADD = libgnashmingutils.la

frame_label_test.swf: frame_label_test
	./frame_label_test $(abs_mediadir)

frame_label_testrunner: $(srcdir)/../generic-testrunner.sh frame_label_test.swf
	sh $< -r5 $(top_builddir) frame_label_test.swf > $@
	chmod 755 $@

path_format_test_SOURCES =	\
	path_format_test.c	\
	$(NULL)
path_format_test_LDADD = libgnashmingutils.la

path_format_test.swf: path_format_test
	./path_format_test $(abs_mediadir)

path_format_testrunner: $(srcdir)/../generic-testrunner.sh path_format_test.swf
	sh $< -r5 $(top_builddir) path_format_test.swf > $@
	chmod 755 $@

callFunction_test_SOURCES =	\
	callFunction_test.c	\
	$(NULL)
callFunction_test_LDADD = libgnashmingutils.la

callFunction_test.swf: callFunction_test
	./callFunction_test $(abs_mediadir)

callFunction_testrunner: $(srcdir)/../generic-testrunner.sh callFunction_test.swf
	sh $< -r5 $(top_builddir) callFunction_test.swf > $@
	chmod 755 $@

duplicate_movie_clip_test_SOURCES =	\
	duplicate_movie_clip_test.c	\
	$(NULL)
duplicate_movie_clip_test_LDADD = libgnashmingutils.la

duplicate_movie_clip_test.swf: duplicate_movie_clip_test
	./duplicate_movie_clip_test $(abs_mediadir)

duplicate_movie_clip_testrunner: $(srcdir)/../generic-testrunner.sh duplicate_movie_clip_test.swf
	sh $< -r5 $(top_builddir) duplicate_movie_clip_test.swf > $@
	chmod 755 $@


duplicate_movie_clip_test2_SOURCES =	\
	duplicate_movie_clip_test2.c	\
	$(NULL)
duplicate_movie_clip_test2_LDADD = libgnashmingutils.la

duplicate_movie_clip_test2.swf: duplicate_movie_clip_test2
	./duplicate_movie_clip_test2 $(abs_mediadir)

duplicate_movie_clip_test2runner: $(srcdir)/../generic-testrunner.sh duplicate_movie_clip_test2.swf
	sh $< -r5 $(top_builddir) duplicate_movie_clip_test2.swf > $@
	chmod 755 $@


event_handler_scope_test_SOURCES =	\
	event_handler_scope_test.c	\
	$(NULL)
event_handler_scope_test_LDADD = libgnashmingutils.la

event_handler_scope_test.swf: event_handler_scope_test
	./event_handler_scope_test $(abs_mediadir)

event_handler_scope_testrunner: $(srcdir)/../generic-testrunner.sh event_handler_scope_test.swf
	sh $< -r5 $(top_builddir) event_handler_scope_test.swf > $@
	chmod 755 $@

attachMovieTest_SOURCES =	\
	attachMovieTest.c	\
	$(NULL)
attachMovieTest_LDADD = libgnashmingutils.la

attachMovieTest.swf: attachMovieTest
	./attachMovieTest $(abs_mediadir)

attachMovieTestRunner_SOURCES = \
	attachMovieTestRunner.cpp \
	$(NULL)
attachMovieTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
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
	$(NULL)
attachMovieLoopingTest_LDADD = libgnashmingutils.la

attachMovieLoopingTest.swf: attachMovieLoopingTest
	./attachMovieLoopingTest $(abs_mediadir)

attachMovieLoopingTestRunner_SOURCES = \
	attachMovieLoopingTestRunner.cpp \
	$(NULL)
attachMovieLoopingTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
attachMovieLoopingTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
attachMovieLoopingTestRunner_DEPENDENCIES = \
	attachMovieLoopingTest.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

# This will only work with Ming-0.4.0beta2
# (support for prebuilt clips inclusion)
attachImported.swf: Dejagnu.swf $(srcdir)/attachImported.as attachMovieTest.swf
	$(MAKESWF) -DOUTPUT_VERSION=6 -v6 -i attachMovieTest.swf:redsquare -o $@ Dejagnu.swf $(srcdir)/attachImported.as
attachImported-Runner: $(srcdir)/../generic-testrunner.sh attachImported.swf Makefile
	sh $< -r5 -c "END_OF_TEST" $(top_builddir) attachImported.swf > $@
	chmod 755 $@

# This will only work with Ming-0.4.0beta2
# (support for prebuilt clips inclusion)
attachExtImported.swf: Dejagnu.swf $(srcdir)/attachExtImported.as attachImported.swf attachMovieTest.swf
	$(MAKESWF) -DOUTPUT_VERSION=6 -v6 -i attachImported.swf:redsquare -o $@ Dejagnu.swf $(srcdir)/attachExtImported.as
attachExtImported-Runner: $(srcdir)/../generic-testrunner.sh attachExtImported.swf Makefile
	sh $< -r5 -c "END_OF_TEST" $(top_builddir) attachExtImported.swf > $@
	chmod 755 $@

simple_loop_test_SOURCES =	\
	simple_loop_test.c	\
	$(NULL)
simple_loop_test_LDADD = libgnashmingutils.la

simple_loop_test.swf: simple_loop_test
	./simple_loop_test $(abs_mediadir)

simple_loop_testrunner_SOURCES = \
	simple_loop_testrunner.cpp \
	$(NULL)
simple_loop_testrunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
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
	$(NULL)
goto_frame_test_LDADD = libgnashmingutils.la

goto_frame_test.swf: goto_frame_test
	./goto_frame_test $(abs_mediadir)

goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh goto_frame_test.swf
	sh $< $(top_builddir) goto_frame_test.swf > $@
	chmod 755 $@

consecutive_goto_frame_test_SOURCES =	\
	consecutive_goto_frame_test.c	\
	$(NULL)
consecutive_goto_frame_test_LDADD = libgnashmingutils.la

consecutive_goto_frame_test.swf: consecutive_goto_frame_test
	./consecutive_goto_frame_test $(abs_mediadir)

consecutive_goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh consecutive_goto_frame_test.swf
	sh $< $(top_builddir) consecutive_goto_frame_test.swf > $@
	chmod 755 $@

multi_doactions_and_goto_frame_test_SOURCES =	\
	multi_doactions_and_goto_frame_test.c	\
	$(NULL)
multi_doactions_and_goto_frame_test_LDADD = libgnashmingutils.la

multi_doactions_and_goto_frame_test.swf: multi_doactions_and_goto_frame_test
	./multi_doactions_and_goto_frame_test $(abs_mediadir)

multi_doactions_and_goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh multi_doactions_and_goto_frame_test.swf
	sh $< $(top_builddir) multi_doactions_and_goto_frame_test.swf > $@
	chmod 755 $@

action_execution_order_test_SOURCES =	\
	action_execution_order_test.c	\
	$(NULL)
action_execution_order_test_LDADD = libgnashmingutils.la

action_execution_order_test.swf: action_execution_order_test
	./action_execution_order_test $(abs_mediadir)

action_execution_order_testrunner: $(srcdir)/../generic-testrunner.sh action_execution_order_test.swf
	sh $< $(top_builddir) action_execution_order_test.swf > $@
	chmod 755 $@

action_execution_order_test1_SOURCES =	\
	action_execution_order_test1.c	\
	$(NULL)
action_execution_order_test1_LDADD = libgnashmingutils.la

action_execution_order_test1.swf: action_execution_order_test1
	./action_execution_order_test1 $(abs_mediadir)

action_execution_order_test1runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test1.swf
	sh $< $(top_builddir) action_execution_order_test1.swf > $@
	chmod 755 $@

action_execution_order_test2_SOURCES =	\
	action_execution_order_test2.c	\
	$(NULL)
action_execution_order_test2_LDADD = libgnashmingutils.la

action_execution_order_test2.swf: action_execution_order_test2
	./action_execution_order_test2 $(abs_mediadir)

action_execution_order_test2runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test2.swf
	sh $< $(top_builddir) action_execution_order_test2.swf > $@
	chmod 755 $@

action_execution_order_test3_SOURCES =	\
	action_execution_order_test3.c	\
	$(NULL)
action_execution_order_test3_LDADD = libgnashmingutils.la

action_execution_order_test3.swf: action_execution_order_test3
	./action_execution_order_test3 $(abs_mediadir)

action_execution_order_test3runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test3.swf
	sh $< $(top_builddir) action_execution_order_test3.swf > $@
	chmod 755 $@

action_execution_order_test4_SOURCES =	\
	action_execution_order_test4.c	\
	$(NULL)
action_execution_order_test4_LDADD = libgnashmingutils.la

action_execution_order_test4.swf: action_execution_order_test4
	./action_execution_order_test4 $(abs_mediadir)

action_execution_order_test4runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test4.swf
	sh $< $(top_builddir) action_execution_order_test4.swf > $@
	chmod 755 $@

action_execution_order_test5_SOURCES =	\
	action_execution_order_test5.c	\
	$(NULL)
action_execution_order_test5_LDADD = libgnashmingutils.la

action_execution_order_test5.swf: action_execution_order_test5
	./action_execution_order_test5 $(abs_mediadir)

action_execution_order_test5runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test5.swf
	sh $< $(top_builddir) action_execution_order_test5.swf > $@
	chmod 755 $@

action_execution_order_test6_SOURCES =	\
	action_execution_order_test6.c	\
	$(NULL)
action_execution_order_test6_LDADD = libgnashmingutils.la

action_execution_order_test6.swf: action_execution_order_test6
	./action_execution_order_test6 $(abs_mediadir)

action_execution_order_test6runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test6.swf
	sh $< $(top_builddir) action_execution_order_test6.swf > $@
	chmod 755 $@

action_execution_order_test7_SOURCES =	\
	action_execution_order_test7.c	\
	$(NULL)
action_execution_order_test7_LDADD = libgnashmingutils.la

action_execution_order_test7.swf: action_execution_order_test7
	./action_execution_order_test7 $(abs_mediadir)

action_execution_order_test7runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test7.swf
	sh $< $(top_builddir) action_execution_order_test7.swf > $@
	chmod 755 $@

action_execution_order_test8_SOURCES =	\
	action_execution_order_test8.c	\
	$(NULL)
action_execution_order_test8_LDADD = libgnashmingutils.la

action_execution_order_test8-v6.swf: action_execution_order_test8
	./action_execution_order_test8 $(abs_mediadir) 6

action_execution_order_test8-v5.swf: action_execution_order_test8
	./action_execution_order_test8 $(abs_mediadir) 5

action_execution_order_test8runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test8-v5.swf action_execution_order_test8-v6.swf
	sh $< -f8 $(top_builddir) action_execution_order_test8-v5.swf action_execution_order_test8-v6.swf > $@
	chmod 755 $@

action_execution_order_test9_SOURCES =	\
	action_execution_order_test9.c	\
	$(NULL)
action_execution_order_test9_LDADD = libgnashmingutils.la

action_execution_order_test9.swf: action_execution_order_test9
	./action_execution_order_test9 $(abs_mediadir)

action_execution_order_test9runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test9.swf
	sh $< -f8 $(top_builddir) action_execution_order_test9.swf > $@
	chmod 755 $@

action_execution_order_test11_SOURCES =	\
	action_execution_order_test11.c	\
	$(NULL)
action_execution_order_test11_LDADD = libgnashmingutils.la

action_execution_order_test11.swf: action_execution_order_test11
	./action_execution_order_test11 $(abs_mediadir)

action_execution_order_test11runner: $(srcdir)/../generic-testrunner.sh action_execution_order_test11.swf
	sh $< -f10 $(top_builddir) action_execution_order_test11.swf > $@
	chmod 755 $@

reverse_execute_PlaceObject2_test1_SOURCES =	\
	reverse_execute_PlaceObject2_test1.c	\
	$(NULL)
reverse_execute_PlaceObject2_test1_LDADD = libgnashmingutils.la

reverse_execute_PlaceObject2_test1.swf: reverse_execute_PlaceObject2_test1
	./reverse_execute_PlaceObject2_test1 $(abs_mediadir)

reverse_execute_PlaceObject2_test1runner: $(srcdir)/../generic-testrunner.sh reverse_execute_PlaceObject2_test1.swf
	sh $< $(top_builddir) reverse_execute_PlaceObject2_test1.swf > $@
	chmod 755 $@

reverse_execute_PlaceObject2_test2_SOURCES =	\
	reverse_execute_PlaceObject2_test2.c	\
	$(NULL)
reverse_execute_PlaceObject2_test2_LDADD = libgnashmingutils.la

reverse_execute_PlaceObject2_test2.swf: reverse_execute_PlaceObject2_test2
	./reverse_execute_PlaceObject2_test2 $(abs_mediadir)

reverse_execute_PlaceObject2_test2runner: $(srcdir)/../generic-testrunner.sh reverse_execute_PlaceObject2_test2.swf
	sh $< $(top_builddir) reverse_execute_PlaceObject2_test2.swf > $@
	chmod 755 $@

action_execution_order_extend_test_SOURCES =	\
	action_execution_order_extend_test.c	\
	$(NULL)
action_execution_order_extend_test_LDADD = libgnashmingutils.la

action_execution_order_extend_test.swf: action_execution_order_extend_test
	./action_execution_order_extend_test $(abs_mediadir)

action_execution_order_extend_testrunner: $(srcdir)/../generic-testrunner.sh action_execution_order_extend_test.swf
	sh $< $(top_builddir) action_execution_order_extend_test.swf > $@
	chmod 755 $@

BitmapDataTest_SOURCES = \
	BitmapDataTest.c \
	$(NULL)

BitmapDataTest_CFLAGS = \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

BitmapDataTest_LDADD = libgnashmingutils.la

BitmapDataTest.swf: BitmapDataTest
	./BitmapDataTest $(abs_mediadir)

BitmapDataTestRunner_SOURCES = \
	BitmapDataTestRunner.cpp \
	$(NULL)

BitmapDataTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

BitmapDataTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

BitmapDataTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	BitmapDataTest.swf	\
	$(NULL)

BitmapDataDraw_SOURCES = \
	BitmapDataDraw.c \
	$(NULL)

BitmapDataDraw_CFLAGS = \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

BitmapDataDraw_LDADD = libgnashmingutils.la

BitmapDataDraw.swf: BitmapDataDraw
	./BitmapDataDraw $(abs_mediadir)

BitmapDataDrawRunner: $(srcdir)/../generic-testrunner.sh BitmapDataDraw.swf
	sh $< $(top_builddir) BitmapDataDraw.swf > $@
	chmod 755 $@

BeginBitmapFill_SOURCES = \
	BeginBitmapFill.c \
	$(NULL)

BeginBitmapFill_CFLAGS = \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

BeginBitmapFill_LDADD = libgnashmingutils.la

BeginBitmapFill.swf: BeginBitmapFill
	./BeginBitmapFill $(abs_mediadir)

BeginBitmapFillRunner_SOURCES = \
	BeginBitmapFillRunner.cpp \
	$(NULL)

BeginBitmapFillRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

BeginBitmapFillRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

BeginBitmapFillRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	BeginBitmapFill.swf	\
	$(NULL)

EmbeddedSoundTest_SOURCES = \
	EmbeddedSoundTest.c \
	$(NULL)

EmbeddedSoundTest_CFLAGS = \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

EmbeddedSoundTest_LDADD = libgnashmingutils.la

EmbeddedSoundTest.swf: EmbeddedSoundTest
	./EmbeddedSoundTest $(abs_mediadir)

EmbeddedSoundTest_Runner_SOURCES = \
	EmbeddedSoundTest-Runner.cpp \
	$(NULL)

EmbeddedSoundTest_Runner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

EmbeddedSoundTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

EmbeddedSoundTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	EmbeddedSoundTest.swf	\
	$(NULL)

BitmapSmoothingTest_SOURCES = \
	BitmapSmoothingTest.c \
	$(NULL)

BitmapSmoothingTest_CFLAGS = \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

BitmapSmoothingTest_LDADD = libgnashmingutils.la

BitmapSmoothingTest-v7.swf: BitmapSmoothingTest
	./BitmapSmoothingTest 7

BitmapSmoothingTest-v8.swf: BitmapSmoothingTest
	./BitmapSmoothingTest 8

registerClassTest_SOURCES =	\
	registerClassTest.c	\
	$(NULL)
registerClassTest_LDADD = libgnashmingutils.la

registerClassTest.swf: registerClassTest
	./registerClassTest $(abs_mediadir)

registerClassTestRunner_SOURCES = \
	registerClassTestRunner.cpp \
	$(NULL)
registerClassTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
registerClassTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
registerClassTestRunner_DEPENDENCIES = \
	registerClassTest.swf \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(NULL)

registerClassTest2_SOURCES =	\
	registerClassTest2.c	\
	$(NULL)
registerClassTest2_LDADD = libgnashmingutils.la

registerClassTest2.swf: registerClassTest2
	./registerClassTest2 $(abs_mediadir)

registerClassTest2runner: $(srcdir)/../generic-testrunner.sh registerClassTest2.swf
	sh $< $(top_builddir) registerClassTest2.swf > $@
	chmod 755 $@

spritehier.swf: spritehier
	./spritehier

RemoveObject2Test.swf: RemoveObject2Test 
	./RemoveObject2Test

RollOverOutTest_SOURCES =	\
	RollOverOutTest.c	\
	$(NULL)
RollOverOutTest_LDADD = libgnashmingutils.la
RollOverOutTest.swf: RollOverOutTest
	./RollOverOutTest $(abs_mediadir)
RollOverOutTest_Runner_SOURCES = \
	RollOverOutTest-Runner.cpp \
	$(NULL)
RollOverOutTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
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
	$(NULL)
ButtonEventsTest_LDADD = libgnashmingutils.la
ButtonEventsTest.swf: ButtonEventsTest
	./ButtonEventsTest $(abs_mediadir)
ButtonEventsTest_Runner_SOURCES = \
	ButtonEventsTest-Runner.cpp \
	$(NULL)
ButtonEventsTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
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
	$(NULL)
SpriteButtonEventsTest_LDADD = libgnashmingutils.la
SpriteButtonEventsTest.swf: SpriteButtonEventsTest
	./SpriteButtonEventsTest $(abs_mediadir)
SpriteButtonEventsTest_Runner_SOURCES = \
	SpriteButtonEventsTest-Runner.cpp \
	$(NULL)
SpriteButtonEventsTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
SpriteButtonEventsTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	SpriteButtonEventsTest.swf	\
	$(NULL)
SpriteButtonEventsTest_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

loadImageTest_SOURCES =	\
	loadImageTest.c	\
	$(NULL)
loadImageTest_LDADD = libgnashmingutils.la
loadImageTest.swf: loadImageTest
	./loadImageTest $(abs_mediadir)

loadMovieTest_SOURCES =	\
	loadMovieTest.c	\
	$(NULL)
loadMovieTest_LDADD = libgnashmingutils.la
loadMovieTest.swf: loadMovieTest
	./loadMovieTest $(abs_mediadir)
loadMovieTestRunner_SOURCES = \
	loadMovieTestRunner.cpp \
	$(NULL)
loadMovieTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
loadMovieTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	-DMEDIADIR='"$(abs_mediadir)"' \
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
VarAndCharClashTest.swf: $(srcdir)/../media/green.swf Dejagnu.swf $(srcdir)/VarAndCharClashTest.as 
	$(MAKESWF) -o $@ Dejagnu.swf $(srcdir)/../media/green.swf $(srcdir)/VarAndCharClashTest.as
VarAndCharClashTest-Runner: $(srcdir)/../generic-testrunner.sh VarAndCharClashTest.swf
	sh $< $(top_builddir) VarAndCharClashTest.swf > $@
	chmod 755 $@

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
FlashVarsTest.swf: Dejagnu.swf $(srcdir)/FlashVarsTest.as 
	$(MAKESWF) -DOUTPUT_VERSION=6 -v6 -o $@ Dejagnu.swf $(srcdir)/FlashVarsTest.as
FlashVarsTest-Runner: $(srcdir)/../generic-testrunner.sh FlashVarsTest.swf Makefile
	echo "$(top_builddir)/gui/gnash -1 -r0 -v -P 'FlashVars=?a=a_in_fvars&v=v_in_fvars&MixCaseFvars=mixFvars&dejagnu.nested=chFVars&_root.fqv=fqFVars&complex={a:'1',b:2}' 'FlashVarsTest.swf?a=a_in_qstring&q=q_in_qstring&MixCaseQstr=mixQstr&dejagnu.nested=chQstr&_root.fqv=fqQstr'" > $@
	chmod 755 $@

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
SharedObjectTest.swf: Dejagnu.swf $(srcdir)/SharedObjectTest.as 
	$(MAKESWF) -DOUTPUT_VERSION=6 -v6 -o $@ Dejagnu.swf $(srcdir)/SharedObjectTest.as

SharedObjectTestRunner: SharedObjectTestRunner.sh Makefile SharedObjectTest.swf
	sed -e 's#@@TOP_BUILDDIR@@#${abs_top_builddir}#' -e 's#@@BASEINPUTSOLDIR@@#${srcdir}/SharedObjectTest.sol#' $< > $@
	chmod +x $@

StageConfigTest.swf: $(srcdir)/StageConfigTest.as 
	$(MAKESWF) -o $@ -s 512x512 $(srcdir)/StageConfigTest.as

ButtonPropertiesTest_SOURCES = \
	ButtonPropertiesTest.c \
	$(NULL)
ButtonPropertiesTest_LDADD = libgnashmingutils.la
ButtonPropertiesTest.swf: ButtonPropertiesTest
	./ButtonPropertiesTest $(abs_mediadir)
ButtonPropertiesTestRunner_SOURCES = \
	ButtonPropertiesTestRunner.cpp \
	$(NULL)
ButtonPropertiesTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
ButtonPropertiesTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	ButtonPropertiesTest.swf	\
	$(NULL)
ButtonPropertiesTestRunner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

InitActionTest2_SOURCES =	\
	InitActionTest2.c	\
	$(NULL)
InitActionTest2_LDADD = libgnashmingutils.la

InitActionTest2.swf: InitActionTest2
	./InitActionTest2 $(abs_mediadir)

InitActionTest2Runner: $(srcdir)/../generic-testrunner.sh InitActionTest2.swf
	sh $< $(top_builddir) InitActionTest2.swf > $@
	chmod 755 $@

ActionOrderTest3_SOURCES =	\
	ActionOrderTest3.c	\
	$(NULL)
ActionOrderTest3_LDADD = libgnashmingutils.la

ActionOrderTest3.swf: ActionOrderTest3
	./ActionOrderTest3 $(abs_mediadir)

ActionOrderTest3Runner: $(srcdir)/../generic-testrunner.sh ActionOrderTest3.swf
	sh $< $(top_builddir) ActionOrderTest3.swf > $@
	chmod 755 $@

ActionOrderTest4_SOURCES =	\
	ActionOrderTest4.c	\
	$(NULL)
ActionOrderTest4_LDADD = libgnashmingutils.la

ActionOrderTest4.swf: ActionOrderTest4
	./ActionOrderTest4 $(abs_mediadir)

ActionOrderTest4Runner: $(srcdir)/../generic-testrunner.sh ActionOrderTest4.swf
	sh $< $(top_builddir) ActionOrderTest4.swf > $@
	chmod 755 $@

ActionOrderTest5_SOURCES =	\
	ActionOrderTest5.c	\
	$(NULL)
ActionOrderTest5_LDADD = libgnashmingutils.la

ActionOrderTest5.swf: ActionOrderTest5
	./ActionOrderTest5 $(abs_mediadir)

ActionOrderTest5Runner: $(srcdir)/../generic-testrunner.sh ActionOrderTest5.swf
	sh $< $(top_builddir) ActionOrderTest5.swf > $@
	chmod 755 $@

RegisterClassTest3_SOURCES =	\
	RegisterClassTest3.c	\
	$(NULL)
RegisterClassTest3_LDADD = libgnashmingutils.la

RegisterClassTest3.swf: RegisterClassTest3
	./RegisterClassTest3 $(abs_mediadir)

RegisterClassTest3Runner: $(srcdir)/../generic-testrunner.sh RegisterClassTest3.swf
	sh $< $(top_builddir) RegisterClassTest3.swf > $@
	chmod 755 $@

RegisterClassTest4_SOURCES =	\
	RegisterClassTest4.c	\
	$(NULL)
RegisterClassTest4_LDADD = libgnashmingutils.la

RegisterClassTest4.swf: RegisterClassTest4
	./RegisterClassTest4 $(abs_mediadir)

RegisterClassTest4Runner: $(srcdir)/../generic-testrunner.sh RegisterClassTest4.swf
	sh $< $(top_builddir) RegisterClassTest4.swf > $@
	chmod 755 $@

init_action_test_SOURCES =	\
	init_action_test.c	\
	$(NULL)
init_action_test_LDADD = libgnashmingutils.la

init_action_test.swf: init_action_test
	./init_action_test $(abs_mediadir)

init_action_testrunner: $(srcdir)/../generic-testrunner.sh init_action_test.swf
	sh $< $(top_builddir) init_action_test.swf > $@
	chmod 755 $@

intervalTest.swf: $(srcdir)/../media/green.swf Dejagnu.swf $(srcdir)/intervalTest.as 
	$(MAKESWF) -o $@ Dejagnu.swf $(srcdir)/intervalTest.as
intervalTestRunner_SOURCES = \
	intervalTestRunner.cpp \
	$(NULL)
intervalTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
intervalTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
intervalTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	intervalTest.swf	\
	$(NULL)

gotoFrame2Test.swf: $(srcdir)/../media/green.swf Dejagnu.swf $(srcdir)/gotoFrame2Test.as 
	$(MAKESWF) -o $@ -iDejagnu.swf:dejagnu $(srcdir)/../actionscript.all/dejagnu_so_init.as \
		$(srcdir)/gotoFrame2Test.as 

gotoFrame2Test-Runner: $(srcdir)/../generic-testrunner.sh gotoFrame2Test.swf
	sh $< $(top_builddir) gotoFrame2Test.swf > $@
	chmod 755 $@

LC-Receive.swf: $(srcdir)/LC-Receive.as 
	$(MAKESWF) -v 8 -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/LC-Receive.as

LC-Send.swf: $(srcdir)/LC-Send.as 
	$(MAKESWF) -v 8 -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/LC-Send.as

LCTestRunner: $(srcdir)/../simultaneous-testrunner.sh LC-Receive.swf LC-Send.swf
	sh $< -c "ENDOFTEST" $(top_builddir) LC-Receive.swf LC-Send.swf > $@
	chmod 755 $@

XMLSocketTest.swf: $(srcdir)/XMLSocketTest.as 
	$(MAKESWF) -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/XMLSocketTest.as

XMLSocketTester: $(srcdir)/XMLSocketTester.sh XMLSocketTest.swf
	sh $< -c "ENDOFTEST" $(top_builddir) $(top_srcdir) $(PERL) \
		XMLSocketTest.swf > $@
	chmod 755 $@

GradientFillTest.swf: $(srcdir)/GradientFillTest.as 
	$(MAKESWF) -v 8 -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/GradientFillTest.as

DrawingApiTest.swf: $(srcdir)/DrawingApiTest.as 
	$(MAKESWF) -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/DrawingApiTest.as

DrawingApiTestRunner_SOURCES = \
	DrawingApiTestRunner.cpp \
	$(NULL)
DrawingApiTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
DrawingApiTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
DrawingApiTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	DrawingApiTest.swf	\
	$(NULL)


PrototypeEventListeners.swf: $(srcdir)/PrototypeEventListeners.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) -r12 -o $@ -v6 -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=6 Dejagnu.swf $(srcdir)/PrototypeEventListeners.as

PrototypeEventListenersTestRunner_SOURCES = \
	PrototypeEventListenersTestRunner.cpp \
	$(NULL)
PrototypeEventListenersTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
PrototypeEventListenersTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
PrototypeEventListenersTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	PrototypeEventListeners.swf	\
	$(NULL)

remoting.swf: $(srcdir)/remoting.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) -n network -r12 -o $@ -v7 -DHTTP_TESTSUITE='\"$(HTTP_TESTSUITE)\"' -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 Dejagnu.swf $(srcdir)/remoting.as $(srcdir)/../actionscript.all/dejagnu_so_fini.as

remotingTestRunner: $(srcdir)/../generic-testrunner.sh remoting.swf
	sh $< $(top_builddir) remoting.swf > $@
	chmod 755 $@

DragDropTest.swf: $(srcdir)/DragDropTest.as Dejagnu.swf DragDropTestLoaded.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) -r12 -o $@ -v6 -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=6 Dejagnu.swf $(srcdir)/DragDropTest.as

DragDropTestLoaded.swf: $(srcdir)/DragDropTest.as Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) -r12 -o $@ -v6 -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=6 -DLOADED_VERSION $(srcdir)/DragDropTest.as

DragDropTestRunner_SOURCES = \
	DragDropTestRunner.cpp \
	$(NULL)
DragDropTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)
DragDropTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
DragDropTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	DragDropTest.swf	\
	$(NULL)


static_vs_dynamic1_SOURCES = \
	static_vs_dynamic1.c \
	$(NULL)

static_vs_dynamic1_LDADD = libgnashmingutils.la

static_vs_dynamic1.swf: static_vs_dynamic1
	./static_vs_dynamic1 $(srcdir)/../media

static_vs_dynamic1_testrunner: $(srcdir)/../generic-testrunner.sh static_vs_dynamic1.swf
	sh $< $(top_builddir) static_vs_dynamic1.swf > $@
	chmod 755 $@

static_vs_dynamic2_SOURCES = \
	static_vs_dynamic2.c \
	$(NULL)

static_vs_dynamic2_LDADD = libgnashmingutils.la

static_vs_dynamic2.swf: static_vs_dynamic2
	./static_vs_dynamic2 $(srcdir)/../media

static_vs_dynamic2_testrunner: $(srcdir)/../generic-testrunner.sh static_vs_dynamic2.swf
	sh $< $(top_builddir) static_vs_dynamic2.swf > $@
	chmod 755 $@


getTimer_test_SOURCES = \
	getTimer_test.c \
	$(NULL)

getTimer_test_LDADD = libgnashmingutils.la

getTimer_test.swf: getTimer_test
	./getTimer_test $(srcdir)/../media

getTimer_testrunner: $(srcdir)/../generic-testrunner.sh getTimer_test.swf
	sh $< $(top_builddir) getTimer_test.swf > $@
	chmod 755 $@

NetStream_SquareTest_SOURCES = \
	NetStream-SquareTest.c \
	$(NULL)

NetStream_SquareTest_LDADD = libgnashmingutils.la

NetStream-SquareTest.swf: NetStream-SquareTest
	./NetStream-SquareTest $(abs_mediadir)

NetStream_SquareTestRunner_SOURCES = \
	NetStream-SquareTestRunner.cpp \
	$(NULL)

NetStream_SquareTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	-DMEDIADIR='"$(abs_mediadir)"' \
	$(NULL)

NetStream_SquareTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

NetStream_SquareTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	NetStream-SquareTest.swf	\
	$(NULL)

Video_EmbedSquareTest_SOURCES = \
	Video-EmbedSquareTest.c \
	$(NULL)

Video_EmbedSquareTest_LDADD = libgnashmingutils.la

Video-EmbedSquareTest.swf: Video-EmbedSquareTest $(srcdir)/../media/square.flv
	./Video-EmbedSquareTest $(srcdir)/../media

Video_EmbedSquareTestRunner_SOURCES = \
	Video-EmbedSquareTestRunner.cpp \
	$(NULL)

Video_EmbedSquareTestRunner_CXXFLAGS = \
	-DSRCDIR='"$(srcdir)"' \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

Video_EmbedSquareTestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)

Video_EmbedSquareTestRunner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	Video-EmbedSquareTest.swf	\
	$(NULL)

LoadVarsTest_SOURCES = \
	LoadVarsTest.c \
	$(NULL)

LoadVarsTest_LDADD = libgnashmingutils.la

LoadVarsTest.swf: LoadVarsTest
	./LoadVarsTest $(abs_mediadir)

LoadVarsTestRunner: $(srcdir)/../generic-testrunner.sh LoadVarsTest.swf
	sh $< $(top_builddir) LoadVarsTest.swf > $@
	chmod 755 $@


instanceNameTest_SOURCES =	\
	instanceNameTest.c	\
	$(NULL)
instanceNameTest_LDADD = libgnashmingutils.la

instanceNameTest.swf: instanceNameTest
	./instanceNameTest $(abs_mediadir)

instanceNameTestRunner: $(srcdir)/../generic-testrunner.sh instanceNameTest.swf
	sh $< $(top_builddir) instanceNameTest.swf > $@
	chmod 755 $@

Dejagnu_SOURCES =	\
	Dejagnu.c	\
	$(NULL)
Dejagnu_LDADD = libgnashmingutils.la

Dejagnu.swf: Dejagnu
	./Dejagnu $(abs_mediadir)

clean-local: 
	-rm *.swf media *.pp *unner

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	BeginBitmapFillRunner \
	BitmapDataTestRunner \
	BitmapDataDrawRunner \
	LoadVarsTestRunner \
	DeviceFontTestRunner \
	EmbeddedFontTestRunner \
	DefineEditTextVariableNameTest-Runner \
	DefineEditTextVariableNameTest2-Runner \
	DefineTextTest-Runner \
	DefineEditTextTest-Runner \
	RollOverOutTest-Runner \
	SpriteButtonEventsTest-Runner \
	timeline_var_test-Runner \
	root_stop_testrunner \
	loop_test-Runner \
	place_object_testrunner \
	place_object_test2runner \
	move_object_testrunner \
	place_and_remove_object_testrunner \
	place_and_remove_object_insane_testrunner \
	unload_movieclip_test1runner \
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
	XMLSocketTester \
	LCTestRunner \
	DrawingApiTestRunner \
	TextSnapshotTest-Runner \
	action_execution_order_test1runner \
	action_execution_order_test2runner \
	action_execution_order_test3runner \
	action_execution_order_test4runner \
	action_execution_order_test5runner \
	action_execution_order_test6runner \
	action_execution_order_test7runner \
	action_execution_order_test8runner \
	action_execution_order_test9runner \
	action_execution_order_test11runner \
	reverse_execute_PlaceObject2_test1runner \
	reverse_execute_PlaceObject2_test2runner \
	displaylist_depths_testrunner \
	displaylist_depths_test2runner \
	displaylist_depths_test3runner \
	displaylist_depths_test4runner \
	displaylist_depths_test5runner \
	displaylist_depths_test6runner \
	displaylist_depths_test7runner \
	displaylist_depths_test8runner \
	displaylist_depths_test9runner \
	displaylist_depths_test10runner \
	displaylist_depths_test11runner \
	DepthLimitsTestrunner \
	matrix_testrunner \
	frame_label_testrunner \
	path_format_testrunner \
	callFunction_testrunner \
	duplicate_movie_clip_testrunner \
	duplicate_movie_clip_test2runner \
	event_handler_scope_testrunner \
	masks_testrunner \
	masks_test2runner \
	KeyEventOrderRunner \
	key_event_testrunner \
	static_vs_dynamic1_testrunner \
	static_vs_dynamic2_testrunner \
	getTimer_testrunner \
	Video-EmbedSquareTestRunner \
	NetStream-SquareTestRunner \
	loop_test2runner \
	loop_test3runner \
	loop_test4runner \
	loop_test5runner \
	loop_test6runner \
	loop_test7runner \
	loop_test8runner \
	loop_test9runner \
	loop_test10runner \
	shape_testrunner \
	morph_test1runner \
	runtime_vm_stack_testrunner \
	instanceNameTestRunner \
	$(NULL)

if MING_VERSION_0_4_3
TEST_CASES += \
    EmbeddedSoundTest-Runner \
	eventSoundTest1-Runner \
    $(NULL)
endif

if MING_VERSION_0_4
TEST_CASES += \
	opcode_guard_testrunner \
	get_frame_number_testrunner
endif

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
TEST_CASES += VarAndCharClashTest-Runner \
	PrototypeEventListenersTestRunner \
	DragDropTestRunner   \
	FlashVarsTest-Runner \
	SharedObjectTestRunner \
	intervalTestRunner \
	gotoFrame2Test-Runner \
	attachImported-Runner \
	attachExtImported-Runner
endif

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
if ENABLE_HTTP_TESTSUITE
TEST_CASES += remotingTestRunner
endif
endif

if MING_SUPPORTS_INIT_ACTIONS
TEST_CASES += \
	registerClassTest2runner \
	init_action_testrunner \
	InitActionTest2Runner \
	ActionOrderTest3Runner \
	ActionOrderTest4Runner \
	ActionOrderTest5Runner \
	RegisterClassTest3Runner \
	RegisterClassTest4Runner \
	$(NULL)
endif

if MING_SUPPORTS_REPLACE_TAG
TEST_CASES += \
	PlaceObject2TestRunner \
	replace_shapes1test_runner \
	replace_sprites1test_runner \
	replace_buttons1test_runner \
	$(NULL)
endif

if MING_SUPPORTS_SWFBUTTON_SET_DEPTH
TEST_CASES += \
	ButtonPropertiesTestRunner \
	ButtonEventsTest-Runner \
	$(NULL)
endif

check-DEJAGNU: site-update $(check_PROGRAMS)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../gnashrc $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

