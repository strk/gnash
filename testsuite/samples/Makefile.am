## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = dejagnu

CLEANFILES =  \
      gnash-dbg.log \
      site.exp.bak \
      testrun.* \
      GotoAndPlayTestRunner

AM_CPPFLAGS = -I.. \
        -I$(top_srcdir)		\
        -I$(top_srcdir)/libbase \
        -I$(top_srcdir)/libmedia \
        -I$(top_srcdir)/backend \
        -I$(top_srcdir)/server  \
        -I$(top_srcdir)/server/parser  \
        -I$(top_srcdir)/libgeometry \
        -I$(top_srcdir)/testsuite \
	$(BOOST_CFLAGS) \
	$(NULL)

GNASH_LIBS = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(top_builddir)/server/libgnashserver.la \
	$(top_builddir)/libbase/libgnashbase.la \
	$(top_builddir)/libamf/libgnashamf.la \
	$(NULL)

check_PROGRAMS = \
	clip_as_button2-TestRunner \
	subshapes-TestRunner \
	gotoFrameOnKeyEvent-TestRunner \
	$(NULL)

check_SCRIPTS = \
	GotoAndPlayTestRunner \
	$(NULL)

if  BUILD_OGL_RENDERER
if   BUILD_GTK_GUI
      GNASH_LIBS += $(top_builddir)/gui/libgnashgtk_gl.la $(GTK_LIBS) $(OPENGL_LIBS)
endif # BUILD_GTK_GUI
if   BUILD_AQUA_GUI
      GNASH_LIBS += $(top_builddir)/gui/libgnashaqua_gl.la $(AQUA_LIBS) $(OPENGL_LIBS)
endif # BUILD_AQUA_GUI
if   BUILD_SDL_GUI
      GNASH_LIBS += $(top_builddir)/gui/libgnashsdl_gl.la $(SDL_LIBS) $(OPENGL_LIBS)
endif # BUILD_SDL_GUI
if BUILD_KDE_GUI
     GNASH_LIBS += $(top_builddir)/gui/libgnashkde_gl.la $(KDE_LIBS) $(QT_LIBS) $(OPENGL_LIBS)
endif # BUILD_KDE_GUI
endif # BUILD_OPENGL_RENDERER

#
# Setup the GUIs that have Cairo support
#
if  BUILD_CAIRO_RENDERER
if   BUILD_GTK_GUI
      GNASH_LIBS += libgnashgtk_cairo.la $(GTK_LIBS) $(CAIRO_LIBS)
endif # BUILD_GTK_GUI
if   BUILD_SDL_GUI
      GNASH_LIBS += libgnashsdl_cairo.la $(SDL_LIBS) $(CAIRO_LIBS)
endif # BUILD_SDL
if   BUILD_FLTK_GUI
      GNASH_LIBS += libgnashfltk_cairo.la $(FLTK2_LIBS) $(CAIRO_LIBS) $(XFT_LIBS) $(X11_LIBS)
endif # BUILD_FLTK_GUI
endif # BUILD_CAIRO_RENDERER

#
# Setup the GUIs that have AntiGrain (AGG) support
#
if  BUILD_AGG_RENDERER
if   BUILD_GTK_GUI
      GNASH_LIBS += $(AGG_LIBS) $(GTK_LIBS)
endif # BUILD_GTK_GUI
if  BUILD_AQUA_GUI
     GNASH_LIBS += $(AQUA_LIBS) $(AGG_LIBS)
endif # BUILD_AQUA_GUI
if  BUILD_SDL_GUI
     GNASH_LIBS += $(SDL_LIBS) $(AGG_LIBS)
endif # BUILD_SDL_GUI
if  BUILD_FLTK_GUI
     GNASH_LIBS += $(FLTK2_LIBS) $(AGG_LIBS) $(XFT_LIBS) $(X11_LIBS)
endif # BUILD_FLTK_GUI
if  BUILD_KDE_GUI
     GNASH_LIBS += $(top_builddir)/gui/libgnashkde_agg.la $(KDE_LIBS) $(QT_LIBS) $(AGG_LIBS)
endif # BUILD_KDE_GUI
if  BUILD_RISCOS_GUI
     GNASH_LIBS += $(top_builddir)/gui/libgnashriscos_agg.la $(RISCOS_LIBS) $(AGG_LIBS) -lOSLib32
endif # BUILD_RISCOS_GUI
endif # BUILD_AGG_RENDERER

clip_as_button2_TestRunner_SOURCES = \
	clip_as_button2-TestRunner.cpp \
	$(NULL)
clip_as_button2_TestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(GNASH_LIBS) \
	$(NULL)
clip_as_button2_TestRunner_CXXFLAGS = \
	-DSRCDIR=\"$(srcdir)\" \
	$(NULL)
clip_as_button2_TestRunner_DEPENDENCIES = \
	clip_as_button2.swf \
	$(NULL)

subshapes_TestRunner_SOURCES = \
	subshapes-TestRunner.cpp \
	$(NULL)
subshapes_TestRunner_LDADD = \
	$(top_builddir)/testsuite/libtestsuite.la \
	$(GNASH_LIBS) \
	$(NULL)
subshapes_TestRunner_CXXFLAGS = \
	-DSRCDIR=\"$(srcdir)\" \
	$(NULL)
subshapes_TestRunner_DEPENDENCIES = \
	subshapes.swf \
	$(GNASH_LIBS) \
	$(NULL)

gotoFrameOnKeyEvent_TestRunner_SOURCES = \
	gotoFrameOnKeyEvent-TestRunner.cpp \
	$(NULL)
gotoFrameOnKeyEvent_TestRunner_LDADD = \
	$(GNASH_LIBS) \
	$(NULL)
gotoFrameOnKeyEvent_TestRunner_CXXFLAGS = \
	-DSRCDIR=\"$(srcdir)\" \
	$(NULL)
gotoFrameOnKeyEvent_TestRunner_DEPENDENCIES = \
	gotoFrameOnKeyEvent.swf \
	$(GNASH_LIBS) \
	$(NULL)

GotoAndPlayTestRunner: $(srcdir)/../generic-testrunner.sh $(srcdir)/GotoAndPlayTest.swf
	sh $< $(top_builddir) $(srcdir)/GotoAndPlayTest.swf > $@
	chmod 755 $@

clean-local: 

EXTRA_DIST = \
	clip_as_button2.swf \
	GotoAndPlayTest.swf \
	gotoFrameOnKeyEvent.swf \
	subshapes.swf \
	README \
	$(NULL)

TEST_DRIVERS = ../simple.exp
TEST_CASES = \
	$(check_PROGRAMS) \
	$(check_SCRIPTS)

check-DEJAGNU: site-update $(check_PROGRAMS)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
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

