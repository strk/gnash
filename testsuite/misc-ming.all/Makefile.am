## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
#   Free Software Foundation, Inc.
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

DISTCLEANFILES = XMLSocketTester

SUBDIRS = . loop loading displaylist_depths action_order register_class \
          init_action sound

EXTRA_DIST = \
	DragDropTest.as \
	DrawingApiTest.as \
	FlashVarsTest.as \
	GradientFillTest.as \
	LC-Receive.as \
	LC-Send.as \
	PrototypeEventListeners.as \
	SharedObjectTest.as \
	StageConfigTest.as \
	VarAndCharClashTest.as \
	XMLSocketTest.as \
	attachExtImported.as \
	attachImported.as \
	Version4Loader.as \
	Version5Loaded.as \
	case6.as \
	case7.as \
	easysound.as \
	empty.as \
	gotoFrame2Test.as \
	intervalTest.as \
	red5test.as \
	oflaDemo.as \
	remoting.as \
	rtmpytest.as \
	widgets.as \
	remoting.php \
	FlashVarsTest.html \
	SharedObjectTestRunner.sh \
	SharedObjectTest.sol/sol1.sol \
	SharedObjectTest.sol/README \
	XMLSocketTester.sh

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
	$(top_builddir)/libcore/libgnashcore.la \
	$(NULL)

if ANDROID
AM_LDFLAGS +=  -lui -llog
endif	# ANDROID

check_PROGRAMS = \
	ResolveEventsTest \
	ResolveEventsTest-Runner \
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
	spritehier \
	timeline_var_test \
	root_stop_test \
	root_stop_testrunner \
	place_object_test \
	place_object_test2 \
	move_object_test \
	place_and_remove_object_test \
	place_and_remove_object_insane_test \
	unload_movieclip_test1 \
	DepthLimitsTest \
	masks_test \
	masks_test2 \
	masks_test2runner \
	attachMovieTest \
	attachMovieTestRunner \
	attachMovieLoopingTest \
	attachMovieLoopingTestRunner \
	goto_frame_test \
	consecutive_goto_frame_test \
	matrix_test \
	multi_doactions_and_goto_frame_test \
	reverse_execute_PlaceObject2_test1 \
	reverse_execute_PlaceObject2_test2 \
	DrawingApiTestRunner \
	frame_label_test \
	path_format_test \
	callFunction_test \
	duplicate_movie_clip_test \
	duplicate_movie_clip_test2 \
	empty_clip_action_test \
	event_handler_scope_test \
	masks_testrunner \
	key_event_test \
	key_event_testrunner \
	static_vs_dynamic1 \
	static_vs_dynamic2 \
	getTimer_test \
	Video-EmbedSquareTest \
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
	$(NULL)

if MING_VERSION_0_4_3
check_PROGRAMS += \
	BitmapSmoothingTest \
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

if MING_SUPPORTS_REPLACE_TAG
check_PROGRAMS += \
	replace_shapes1test \
	replace_sprites1test \
	replace_buttons1test \
	$(NULL)
endif

if MING_SUPPORTS_SWFBUTTON_SET_DEPTH
check_PROGRAMS += \
	KeyEventOrder \
	KeyEventOrderRunner \
	ButtonPropertiesTest \
	ButtonEventsTest \
	ButtonPropertiesTestRunner \
	ButtonEventsTest-Runner \
	$(NULL)
endif

check_SCRIPTS = \
	GradientFillTestRunner \
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
	gotoFrame2Test-Runner \
	reverse_execute_PlaceObject2_test1runner \
	reverse_execute_PlaceObject2_test2runner \
	frame_label_testrunner \
	path_format_testrunner \
	callFunction_testrunner \
	duplicate_movie_clip_testrunner \
	duplicate_movie_clip_test2runner \
	empty_clip_action_testrunner \
	event_handler_scope_testrunner \
	matrix_testrunner \
	static_vs_dynamic1_testrunner \
	static_vs_dynamic2_testrunner \
	getTimer_testrunner \
	DepthLimitsTestrunner \
	shape_testrunner \
	DefineEditTextVariableNameTest-Runner \
	DefineEditTextVariableNameTest2-Runner \
	runtime_vm_stack_testrunner \
	new_child_in_unload_testrunner \
	instanceNameTestRunner \
	$(NULL)

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
check_SCRIPTS += VarAndCharClashTest-Runner \
	gotoFrame2Test-Runner \
	FlashVarsTest-Runner \
	attachImported-Runner \
	attachExtImported-Runner \
	MixedVersion4And5Runner \
	SharedObjectTestRunner
endif

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
if ENABLE_HTTP_TESTSUITE
check_SCRIPTS += remotingTestRunner
endif
if ENABLE_RED5_TESTING
check_SCRIPTS += red5test-runner
check_SCRIPTS += oflaDemo-runner
endif
if ENABLE_RTMPY_TESTING
check_SCRIPTS += rtmpytest-runner
endif
endif

if MING_VERSION_0_4
check_SCRIPTS += \
	opcode_guard_testrunner \
	get_frame_number_testrunner \
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


# This is so check.as finds revno.h
MAKESWF_FLAGS = -I$(top_builddir)

# TESTS = \
# 	DefineEditTextVariableNameTest-Runner 

check_LTLIBRARIES = libgnashmingutils.la 
libgnashmingutils_la_SOURCES =	\
	ming_utils.h		\
	ming_utils.c

definebitsjpeg2_SOURCES = definebitsjpeg2.c
definebitsjpeg2_LDADD = $(MING_LIBS)
definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2 $(srcdir)/../media/red.jpg

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

Version4Loader.swf: $(srcdir)/Version4Loader.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -o $@ -v4 $(srcdir)/Version4Loader.as
Version5Loaded.swf: $(srcdir)/Version5Loaded.as Dejagnu.swf
	$(MAKESWF) $(MAKESWF_FLAGS) -o $@ Dejagnu.swf -v5 $(srcdir)/Version5Loaded.as
MixedVersion4And5Runner: Version4Loader.swf Version5Loaded.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) Version5Loaded.swf > $@
	chmod 755 $@

TextSnapshotTest.swf: TextSnapshotTest
	./TextSnapshotTest $(abs_mediadir)

TextSnapshotTest_SOURCES =  TextSnapshotTest.c
TextSnapshotTest_LDADD = libgnashmingutils.la

TextSnapshotTest-Runner: $(srcdir)/../generic-testrunner.sh TextSnapshotTest.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) TextSnapshotTest.swf > $@
	chmod 755 $@

DeviceFontTestRunner: $(srcdir)/../generic-testrunner.sh DeviceFontTest.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) DeviceFontTest.swf > $@
	chmod 755 $@
DeviceFontTest.swf: DeviceFontTest
	./DeviceFontTest $(abs_mediadir)
DeviceFontTest_SOURCES =  DeviceFontTest.c
DeviceFontTest_LDADD = libgnashmingutils.la

EmbeddedFontTestRunner: $(srcdir)/../generic-testrunner.sh EmbeddedFontTest.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) EmbeddedFontTest.swf > $@
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
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) opcode_guard_test.swf > $@
	chmod 755 $@

runtime_vm_stack_test_SOURCES =	\
	runtime_vm_stack_test.c	\
	$(NULL)
runtime_vm_stack_test_LDADD = libgnashmingutils.la

runtime_vm_stack_test.swf: runtime_vm_stack_test
	./runtime_vm_stack_test $(abs_mediadir)

runtime_vm_stack_testrunner: $(srcdir)/../generic-testrunner.sh runtime_vm_stack_test.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) runtime_vm_stack_test.swf > $@
	chmod 755 $@

new_child_in_unload_test_SOURCES =	\
	new_child_in_unload_test.c	\
	$(NULL)
new_child_in_unload_test_LDADD = libgnashmingutils.la

new_child_in_unload_test.swf: new_child_in_unload_test
	./new_child_in_unload_test $(abs_mediadir)

new_child_in_unload_testrunner: $(srcdir)/../generic-testrunner.sh new_child_in_unload_test.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) new_child_in_unload_test.swf > $@
	chmod 755 $@

PlaceObject2Test_SOURCES =	\
	PlaceObject2Test.c	\
	$(NULL)
PlaceObject2Test_LDADD = libgnashmingutils.la

PlaceObject2Test.swf: PlaceObject2Test
	./PlaceObject2Test $(abs_mediadir)

PlaceObject2TestRunner: $(srcdir)/../generic-testrunner.sh PlaceObject2Test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) PlaceObject2Test.swf > $@
	chmod 755 $@

RemoveObject2Test_SOURCES = RemoveObject2Test.c
RemoveObject2Test_LDADD = libgnashmingutils.la

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
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) DefineEditTextVariableNameTest2.swf > $@
	chmod 755 $@

timeline_var_test_SOURCES =	\
	timeline_var_test.c	\
	$(NULL)
timeline_var_test_LDADD = libgnashmingutils.la

timeline_var_test.swf: timeline_var_test
	./timeline_var_test $(abs_mediadir)

timeline_var_test-Runner: $(srcdir)/../generic-testrunner.sh timeline_var_test.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) timeline_var_test.swf > $@
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

shape_test_SOURCES = shape_test.c
shape_test_LDADD = libgnashmingutils.la

shape_test.swf: shape_test
	./shape_test $(abs_mediadir)

shape_testrunner: $(srcdir)/../generic-testrunner.sh shape_test.swf
	sh $(srcdir)/../generic-testrunner.sh -f20 $(top_builddir) shape_test.swf > $@
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
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) place_object_test.swf > $@
	chmod 755 $@


place_object_test2_SOURCES = place_object_test2.c	
place_object_test2_LDADD = libgnashmingutils.la

place_object_test2.swf: place_object_test2
	./place_object_test2 $(abs_mediadir)

place_object_test2runner: $(srcdir)/../generic-testrunner.sh place_object_test2.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) place_object_test2.swf > $@
	chmod 755 $@

move_object_test_SOURCES =	\
	move_object_test.c	\
	$(NULL)
move_object_test_LDADD = libgnashmingutils.la

move_object_test.swf: move_object_test
	./move_object_test $(abs_mediadir)

move_object_testrunner: $(srcdir)/../generic-testrunner.sh move_object_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) move_object_test.swf > $@
	chmod 755 $@

place_and_remove_object_test_SOURCES =	\
	place_and_remove_object_test.c	\
	$(NULL)
place_and_remove_object_test_LDADD = libgnashmingutils.la

place_and_remove_object_test.swf: place_and_remove_object_test
	./place_and_remove_object_test $(abs_mediadir)

place_and_remove_object_testrunner: $(srcdir)/../generic-testrunner.sh place_and_remove_object_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) place_and_remove_object_test.swf > $@
	chmod 755 $@

place_and_remove_object_insane_test_SOURCES =	\
	place_and_remove_object_insane_test.c	\
	$(NULL)
place_and_remove_object_insane_test_LDADD = libgnashmingutils.la

place_and_remove_object_insane_test.swf: place_and_remove_object_insane_test
	./place_and_remove_object_insane_test $(abs_mediadir)

place_and_remove_object_insane_testrunner: $(srcdir)/../generic-testrunner.sh place_and_remove_object_insane_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) place_and_remove_object_insane_test.swf > $@
	chmod 755 $@

unload_movieclip_test1_SOURCES =	\
	unload_movieclip_test1.c	\
	$(NULL)
unload_movieclip_test1_LDADD = libgnashmingutils.la

unload_movieclip_test1.swf: unload_movieclip_test1
	./unload_movieclip_test1 $(abs_mediadir)

unload_movieclip_test1runner: $(srcdir)/../generic-testrunner.sh unload_movieclip_test1.swf
	sh $(srcdir)/../generic-testrunner.sh -f10 $(top_builddir) unload_movieclip_test1.swf > $@
	chmod 755 $@

DepthLimitsTest_SOURCES = DepthLimitsTest.c
DepthLimitsTest_LDADD = libgnashmingutils.la

DepthLimitsTest.swf: DepthLimitsTest
	./DepthLimitsTest $(abs_mediadir)

DepthLimitsTestrunner: $(srcdir)/../generic-testrunner.sh DepthLimitsTest.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) DepthLimitsTest.swf > $@
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
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) matrix_test.swf > $@
	chmod 755 $@

get_frame_number_test_SOURCES =	\
	get_frame_number_test.c	\
	$(NULL)
get_frame_number_test_LDADD = libgnashmingutils.la

get_frame_number_test.swf: get_frame_number_test
	./get_frame_number_test $(abs_mediadir)

get_frame_number_testrunner: $(srcdir)/../generic-testrunner.sh get_frame_number_test.swf
	sh $(srcdir)/../generic-testrunner.sh -f20 $(top_builddir) get_frame_number_test.swf > $@
	chmod 755 $@

frame_label_test_SOURCES =	\
	frame_label_test.c	\
	$(NULL)
frame_label_test_LDADD = libgnashmingutils.la

frame_label_test.swf: frame_label_test
	./frame_label_test $(abs_mediadir)

frame_label_testrunner: $(srcdir)/../generic-testrunner.sh frame_label_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) frame_label_test.swf > $@
	chmod 755 $@

path_format_test_SOURCES =	\
	path_format_test.c	\
	$(NULL)
path_format_test_LDADD = libgnashmingutils.la

path_format_test.swf: path_format_test
	./path_format_test $(abs_mediadir)

path_format_testrunner: $(srcdir)/../generic-testrunner.sh path_format_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) path_format_test.swf > $@
	chmod 755 $@

callFunction_test_SOURCES =	\
	callFunction_test.c	\
	$(NULL)
callFunction_test_LDADD = libgnashmingutils.la

callFunction_test.swf: callFunction_test
	./callFunction_test $(abs_mediadir)

callFunction_testrunner: $(srcdir)/../generic-testrunner.sh callFunction_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) callFunction_test.swf > $@
	chmod 755 $@

duplicate_movie_clip_test_SOURCES =	\
	duplicate_movie_clip_test.c	\
	$(NULL)
duplicate_movie_clip_test_LDADD = libgnashmingutils.la

duplicate_movie_clip_test.swf: duplicate_movie_clip_test
	./duplicate_movie_clip_test $(abs_mediadir)

duplicate_movie_clip_testrunner: $(srcdir)/../generic-testrunner.sh duplicate_movie_clip_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) duplicate_movie_clip_test.swf > $@
	chmod 755 $@


duplicate_movie_clip_test2_SOURCES =	\
	duplicate_movie_clip_test2.c	\
	$(NULL)
duplicate_movie_clip_test2_LDADD = libgnashmingutils.la

duplicate_movie_clip_test2.swf: duplicate_movie_clip_test2
	./duplicate_movie_clip_test2 $(abs_mediadir)

duplicate_movie_clip_test2runner: $(srcdir)/../generic-testrunner.sh duplicate_movie_clip_test2.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) duplicate_movie_clip_test2.swf > $@
	chmod 755 $@


empty_clip_action_test_SOURCES =	\
	empty_clip_action_test.c	\
	$(NULL)
empty_clip_action_test_LDADD = libgnashmingutils.la

empty_clip_action_test.swf: empty_clip_action_test
	./empty_clip_action_test $(abs_mediadir)

empty_clip_action_testrunner: $(srcdir)/../generic-testrunner.sh empty_clip_action_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) empty_clip_action_test.swf > $@
	chmod 755 $@


event_handler_scope_test_SOURCES =	\
	event_handler_scope_test.c	\
	$(NULL)
event_handler_scope_test_LDADD = libgnashmingutils.la

event_handler_scope_test.swf: event_handler_scope_test
	./event_handler_scope_test $(abs_mediadir)

event_handler_scope_testrunner: $(srcdir)/../generic-testrunner.sh event_handler_scope_test.swf
	sh $(srcdir)/../generic-testrunner.sh -r5 $(top_builddir) event_handler_scope_test.swf > $@
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
	$(MAKESWF) $(MAKESWF_FLAGS) -DOUTPUT_VERSION=6 -v6 -i attachMovieTest.swf:redsquare -o $@ Dejagnu.swf $(srcdir)/attachImported.as
attachImported-Runner: $(srcdir)/../generic-testrunner.sh attachImported.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r5 -c "END_OF_TEST" $(top_builddir) attachImported.swf > $@
	chmod 755 $@

# This will only work with Ming-0.4.0beta2
# (support for prebuilt clips inclusion)
attachExtImported.swf: Dejagnu.swf $(srcdir)/attachExtImported.as attachImported.swf attachMovieTest.swf
	$(MAKESWF) $(MAKESWF_FLAGS) -DOUTPUT_VERSION=6 -v6 -i attachImported.swf:redsquare -o $@ Dejagnu.swf $(srcdir)/attachExtImported.as
attachExtImported-Runner: $(srcdir)/../generic-testrunner.sh attachExtImported.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -r5 -c "END_OF_TEST" $(top_builddir) attachExtImported.swf > $@
	chmod 755 $@

goto_frame_test_SOURCES =	\
	goto_frame_test.c	\
	$(NULL)
goto_frame_test_LDADD = libgnashmingutils.la

goto_frame_test.swf: goto_frame_test
	./goto_frame_test $(abs_mediadir)

goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh goto_frame_test.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) goto_frame_test.swf > $@
	chmod 755 $@

consecutive_goto_frame_test_SOURCES =	\
	consecutive_goto_frame_test.c	\
	$(NULL)
consecutive_goto_frame_test_LDADD = libgnashmingutils.la

consecutive_goto_frame_test.swf: consecutive_goto_frame_test
	./consecutive_goto_frame_test $(abs_mediadir)

consecutive_goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh consecutive_goto_frame_test.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) consecutive_goto_frame_test.swf > $@
	chmod 755 $@

multi_doactions_and_goto_frame_test_SOURCES =	\
	multi_doactions_and_goto_frame_test.c	\
	$(NULL)
multi_doactions_and_goto_frame_test_LDADD = libgnashmingutils.la

multi_doactions_and_goto_frame_test.swf: multi_doactions_and_goto_frame_test
	./multi_doactions_and_goto_frame_test $(abs_mediadir)

multi_doactions_and_goto_frame_testrunner: $(srcdir)/../generic-testrunner.sh multi_doactions_and_goto_frame_test.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) multi_doactions_and_goto_frame_test.swf > $@
	chmod 755 $@

reverse_execute_PlaceObject2_test1_SOURCES =	\
	reverse_execute_PlaceObject2_test1.c	\
	$(NULL)
reverse_execute_PlaceObject2_test1_LDADD = libgnashmingutils.la

reverse_execute_PlaceObject2_test1.swf: reverse_execute_PlaceObject2_test1
	./reverse_execute_PlaceObject2_test1 $(abs_mediadir)

reverse_execute_PlaceObject2_test1runner: $(srcdir)/../generic-testrunner.sh reverse_execute_PlaceObject2_test1.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) reverse_execute_PlaceObject2_test1.swf > $@
	chmod 755 $@

reverse_execute_PlaceObject2_test2_SOURCES =	\
	reverse_execute_PlaceObject2_test2.c	\
	$(NULL)
reverse_execute_PlaceObject2_test2_LDADD = libgnashmingutils.la

reverse_execute_PlaceObject2_test2.swf: reverse_execute_PlaceObject2_test2
	./reverse_execute_PlaceObject2_test2 $(abs_mediadir)

reverse_execute_PlaceObject2_test2runner: $(srcdir)/../generic-testrunner.sh reverse_execute_PlaceObject2_test2.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) reverse_execute_PlaceObject2_test2.swf > $@
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
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) BitmapDataDraw.swf > $@
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

ResolveEventsTest_SOURCES =	\
	ResolveEventsTest.c	\
	$(NULL)
ResolveEventsTest_LDADD = libgnashmingutils.la
ResolveEventsTest.swf: ResolveEventsTest
	./ResolveEventsTest $(abs_mediadir)
ResolveEventsTest_Runner_SOURCES = \
	ResolveEventsTest-Runner.cpp \
	$(NULL)
ResolveEventsTest_Runner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(AM_LDFLAGS) \
	$(NULL)
ResolveEventsTest_Runner_DEPENDENCIES = \
	$(top_builddir)/testsuite/libtestsuite.la \
	ResolveEventsTest.swf	\
	$(NULL)
ResolveEventsTest_Runner_CXXFLAGS = \
	-DTGTDIR='"$(abs_builddir)"' \
	$(NULL)

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
VarAndCharClashTest.swf: $(srcdir)/../media/green.swf Dejagnu.swf $(srcdir)/VarAndCharClashTest.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -o $@ Dejagnu.swf $(srcdir)/../media/green.swf $(srcdir)/VarAndCharClashTest.as
VarAndCharClashTest-Runner: $(srcdir)/../generic-testrunner.sh VarAndCharClashTest.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) VarAndCharClashTest.swf > $@
	chmod 755 $@

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
FlashVarsTest.swf: Dejagnu.swf FlashVarsTest.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -DOUTPUT_VERSION=6 -v6 -o $@ Dejagnu.swf $(srcdir)/FlashVarsTest.as
FlashVarsTest-Runner: $(srcdir)/../generic-testrunner.sh FlashVarsTest.swf Makefile
	echo "$(top_builddir)/gui/gnash -1 -r0 -v -P 'FlashVars=?a=a_in_fvars&v=v_in_fvars&MixCaseFvars=mixFvars&dejagnu.nested=chFVars&_root.fqv=fqFVars&complex={a:'1',b:2}' 'FlashVarsTest.swf?a=a_in_qstring&q=q_in_qstring&MixCaseQstr=mixQstr&dejagnu.nested=chQstr&_root.fqv=fqQstr'" > $@
	chmod 755 $@

# This will only work with Ming-0.4.0beta2 (support for prebuilt clips inclusion)
SharedObjectTest.swf: Dejagnu.swf SharedObjectTest.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -DOUTPUT_VERSION=6 -v6 -o $@ Dejagnu.swf $(srcdir)/SharedObjectTest.as

SharedObjectTestRunner: SharedObjectTestRunner.sh Makefile SharedObjectTest.swf
	sed -e 's#@@TOP_BUILDDIR@@#${abs_top_builddir}#' -e 's#@@BASEINPUTSOLDIR@@#${srcdir}/SharedObjectTest.sol#' $(srcdir)/SharedObjectTestRunner.sh > $@
	chmod +x $@

StageConfigTest.swf: StageConfigTest.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -o $@ -s 512x512 $(srcdir)/StageConfigTest.as

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

intervalTest.swf: $(srcdir)/../media/green.swf Dejagnu.swf intervalTest.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -o $@ Dejagnu.swf $(srcdir)/intervalTest.as
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

gotoFrame2Test.swf: $(srcdir)/../media/green.swf Dejagnu.swf gotoFrame2Test.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -o $@ -iDejagnu.swf:dejagnu $(srcdir)/../actionscript.all/dejagnu_so_init.as \
		$(srcdir)/gotoFrame2Test.as 

gotoFrame2Test-Runner: $(srcdir)/../generic-testrunner.sh gotoFrame2Test.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) gotoFrame2Test.swf > $@
	chmod 755 $@

LC-Receive.swf: LC-Receive.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -v 8 -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/LC-Receive.as

LC-Send.swf: LC-Send.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -v 8 -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/LC-Send.as

LCTestRunner: $(srcdir)/../simultaneous-testrunner.sh LC-Receive.swf LC-Send.swf
	sh $(srcdir)/../simultaneous-testrunner.sh -c "ENDOFTEST" $(top_builddir) LC-Receive.swf LC-Send.swf > $@
	chmod 755 $@

XMLSocketTest.swf: XMLSocketTest.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/XMLSocketTest.as

XMLSocketTester: XMLSocketTester.sh XMLSocketTest.swf
	sh $(srcdir)/XMLSocketTester.sh -c "ENDOFTEST" $(top_builddir) $(top_srcdir) $(PERL) \
		XMLSocketTest.swf > $@
	chmod 755 $@

GradientFillTest.swf: GradientFillTest.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -v 8 -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/GradientFillTest.as

GradientFillTestRunner: $(srcdir)/../generic-testrunner.sh GradientFillTest.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) GradientFillTest.swf > $@
	chmod 755 $@

DrawingApiTest.swf: DrawingApiTest.as 
	$(MAKESWF) $(MAKESWF_FLAGS) -r 1 -o $@  $(srcdir)/empty.as $(srcdir)/DrawingApiTest.as

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

PrototypeEventListeners.swf: PrototypeEventListeners.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) $(MAKESWF_FLAGS) -r12 -o $@ -v6 -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=6 Dejagnu.swf $(srcdir)/PrototypeEventListeners.as

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

remoting.swf: remoting.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) $(MAKESWF_FLAGS) -n network -r12 -o $@ -v7 -DHTTP_TESTSUITE='\"$(HTTP_TESTSUITE)\"' -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 Dejagnu.swf $(srcdir)/remoting.as $(srcdir)/../actionscript.all/dejagnu_so_fini.as

remotingTestRunner: $(srcdir)/../generic-testrunner.sh remoting.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) remoting.swf > $@
	chmod 755 $@

red5test.swf: red5test.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) $(MAKESWF_FLAGS) -n network -r12 -o $@ -v7 -DRED5_HOST='\"$(RED5_HOST)\"' \
		-DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 Dejagnu.swf $(srcdir)/red5test.as \
		$(srcdir)/../actionscript.all/dejagnu_so_fini.as

oflaDemo.swf: oflaDemo.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) $(MAKESWF_FLAGS) -n network -r12 -o $@ -v7 -DRED5_HOST='\"$(RED5_HOST)\"' \
		-DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 Dejagnu.swf $(srcdir)/oflaDemo.as \
		$(srcdir)/../actionscript.all/dejagnu_so_fini.as

oflaDemo-runner: $(srcdir)/../generic-testrunner.sh oflaDemo.swf
	sh $(srcdir)/../generic-testrunner.sh -c "ENDOFTEST" $(top_builddir) oflaDemo.swf > $@
	chmod 755 $@

red5test-runner: $(srcdir)/../generic-testrunner.sh red5test.swf
	sh $(srcdir)/../generic-testrunner.sh -c "ENDOFTEST" $(top_builddir) red5test.swf > $@
	chmod 755 $@

rtmpytest.swf: rtmpytest.as Dejagnu.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) $(MAKESWF_FLAGS) -n network -r12 -o $@ -v7 -DRTMPY_HOST='\"$(RTMPY_HOST)\"' \
		-DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 Dejagnu.swf $(srcdir)/rtmpytest.as \
		$(srcdir)/../actionscript.all/dejagnu_so_fini.as

rtmpytest-runner: $(srcdir)/../generic-testrunner.sh rtmpytest.swf
	sh $(srcdir)/../generic-testrunner.sh -c "ENDOFTEST" $(top_builddir) rtmpytest.swf > $@
	chmod 755 $@

case6.swf: case6.as Makefile ../actionscript.all/check.as
	$(MAKESWF) $(MAKESWF_FLAGS) -v6 -o $@ -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=6 -I$(srcdir)/../actionscript.all $(srcdir)/case6.as 

case7.swf: case7.as Makefile ../actionscript.all/check.as
	$(MAKESWF) $(MAKESWF_FLAGS) -v7 -o $@ -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=7 -I$(srcdir)/../actionscript.all $(srcdir)/case7.as 

caseTestRunner: $(srcdir)/../generic-testrunner.sh case6.swf case7.swf Makefile
	sh $(srcdir)/../generic-testrunner.sh -f10 -r10 -c '#total' $(top_builddir) case6.swf case7.swf > $@
	chmod 755 $@

DragDropTest.swf: DragDropTest.as Dejagnu.swf DragDropTestLoaded.swf Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) $(MAKESWF_FLAGS) -r12 -o $@ -v6 -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=6 Dejagnu.swf $(srcdir)/DragDropTest.as

DragDropTestLoaded.swf: DragDropTest.as Makefile ../actionscript.all/check.as ../actionscript.all/utils.as
	$(MAKESWF) $(MAKESWF_FLAGS) -r12 -o $@ -v6 -DUSE_DEJAGNU_MODULE -DOUTPUT_VERSION=6 -DLOADED_VERSION $(srcdir)/DragDropTest.as

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
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) static_vs_dynamic1.swf > $@
	chmod 755 $@

static_vs_dynamic2_SOURCES = \
	static_vs_dynamic2.c \
	$(NULL)

static_vs_dynamic2_LDADD = libgnashmingutils.la

static_vs_dynamic2.swf: static_vs_dynamic2
	./static_vs_dynamic2 $(srcdir)/../media

static_vs_dynamic2_testrunner: $(srcdir)/../generic-testrunner.sh static_vs_dynamic2.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) static_vs_dynamic2.swf > $@
	chmod 755 $@


getTimer_test_SOURCES = \
	getTimer_test.c \
	$(NULL)

getTimer_test_LDADD = libgnashmingutils.la

getTimer_test.swf: getTimer_test
	./getTimer_test $(srcdir)/../media

getTimer_testrunner: $(srcdir)/../generic-testrunner.sh getTimer_test.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) getTimer_test.swf > $@
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

instanceNameTest_SOURCES =	\
	instanceNameTest.c	\
	$(NULL)
instanceNameTest_LDADD = libgnashmingutils.la

instanceNameTest.swf: instanceNameTest
	./instanceNameTest $(abs_mediadir)

instanceNameTestRunner: $(srcdir)/../generic-testrunner.sh instanceNameTest.swf
	sh $(srcdir)/../generic-testrunner.sh $(top_builddir) instanceNameTest.swf > $@
	chmod 755 $@

Dejagnu_SOURCES = Dejagnu.c
Dejagnu_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	libgnashmingutils.la \
	$(AM_LDFLAGS) \
	$(NULL)

Dejagnu.swf: Dejagnu
	./Dejagnu $(abs_mediadir)

clean-local: 
	-rm *.swf media *.pp *unner

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	GradientFillTestRunner \
	BeginBitmapFillRunner \
	BitmapDataTestRunner \
	BitmapDataDrawRunner \
	DeviceFontTestRunner \
	EmbeddedFontTestRunner \
	DefineEditTextVariableNameTest-Runner \
	DefineEditTextVariableNameTest2-Runner \
	DefineTextTest-Runner \
	DefineEditTextTest-Runner \
	RollOverOutTest-Runner \
	SpriteButtonEventsTest-Runner \
	ResolveEventsTest-Runner \
	timeline_var_test-Runner \
	root_stop_testrunner \
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
	XMLSocketTester \
	LCTestRunner \
	DrawingApiTestRunner \
	TextSnapshotTest-Runner \
	reverse_execute_PlaceObject2_test1runner \
	reverse_execute_PlaceObject2_test2runner \
	DepthLimitsTestrunner \
	matrix_testrunner \
	frame_label_testrunner \
	path_format_testrunner \
	callFunction_testrunner \
	duplicate_movie_clip_testrunner \
	duplicate_movie_clip_test2runner \
	empty_clip_action_testrunner \
	event_handler_scope_testrunner \
	masks_testrunner \
	masks_test2runner \
	key_event_testrunner \
	static_vs_dynamic1_testrunner \
	static_vs_dynamic2_testrunner \
	getTimer_testrunner \
	Video-EmbedSquareTestRunner \
	NetStream-SquareTestRunner \
	shape_testrunner \
	morph_test1runner \
	runtime_vm_stack_testrunner \
	instanceNameTestRunner \
	$(NULL)

if MING_VERSION_0_4_3
TEST_CASES += \
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
	MixedVersion4And5Runner \
	intervalTestRunner \
	gotoFrame2Test-Runner \
	attachImported-Runner \
	attachExtImported-Runner
endif

if MAKESWF_SUPPORTS_PREBUILT_CLIPS
if ENABLE_HTTP_TESTSUITE
TEST_CASES += remotingTestRunner
endif
if ENABLE_RED5_TESTING
TEST_CASES += red5test-runner 
TEST_CASES += oflaDemo-runner 
endif
if ENABLE_RTMPY_TESTING
TEST_CASES += rtmpytest-runner
endif
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
	KeyEventOrderRunner \
	$(NULL)
endif

TEST_ENV = GNASH_GC_TRIGGER_THRESHOLD=0
TEST_ENV += GNASHRC=../gnashrc
TEST_ENV += GST_INSTALL_PLUGINS_HELPER=$(top_srcdir)/testsuite/gst_install_plugins_helper.sh


check-DEJAGNU: site-update $(check_PROGRAMS)
	runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $(TEST_ENV) $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    $(TEST_ENV) $(SHELL) $$i; \
	  done; \
	fi

site-update: site.exp
	@rm -fr site.exp.bak
	@cp site.exp site.exp.bak
	@sed -e '/testcases/d' site.exp.bak > site.exp
	@echo "# This is a list of the pre-compiled testcases" >> site.exp
	@echo "set testcases \"$(TEST_CASES)\"" >> site.exp

