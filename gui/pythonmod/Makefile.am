pkgpythondir = $(pyexecdir)/gtk-2.0
pkgpyexecdir = $(pyexecdir)/gtk-2.0

GNASH_INCLUDES = \
	-I$(top_srcdir)/gui \
	-I$(top_srcdir)/libbase	\
	-I$(top_srcdir)/backend	\
	-I$(top_srcdir)/libcore	\
	-I$(top_srcdir)/libcore/swf \
	-I$(top_srcdir)/libcore/vm \
	-I$(top_srcdir)/libcore/parser \
	-I$(top_srcdir)/libmedia \
	-I$(top_srcdir)/libmedia/gst \
	-I$(top_srcdir)/libmedia/ffmpeg \
	-I$(top_srcdir)/libsound

INCLUDES = \
	$(PYTHON_CFLAGS) \
	$(PYGTK_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(GNASH_INCLUDES)

pkgpyexec_LTLIBRARIES = gnash.la

GNASH_LIBS = \
	$(top_builddir)/libamf/libgnashamf.la \
	$(top_builddir)/libnet/libgnashnet.la \
	$(top_builddir)/libmedia/libgnashmedia.la \
	$(top_builddir)/libsound/libgnashsound.la \
	$(top_builddir)/libcore/libgnashcore.la \
	$(top_builddir)/backend/libgnashcairo.la \
	$(top_builddir)/libbase/libgnashbase.la

gnash_la_LDFLAGS = -module -avoid-version initgnash -export-dynamic #-no-undefined
gnash_la_LIBADD = \
	$(GNASH_LIBS) \
	$(GTK2_LIBS) \
	$(DEPS_LIBS)

gnash_la_SOURCES = \
	gnash.c	\
	gnashmodule.c \
	gnash-view.cpp\
	$(top_srcdir)/gui/gtk_canvas.cpp \
	$(top_srcdir)/gui/gtk_glue_cairo.cpp

gnash.c: gnash.defs gnash.override

CLEANFILES = gnash.c
EXTRA_DIST = gnash.override gnash.defs

.defs.c:
	(cd $(srcdir)\
	 && $(PYGOBJECT_CODEGEN) \
	    --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	    --register $(PYGTK_DEFSDIR)/gtk-types.defs \
	    --override $*.override \
	    --prefix py$* $*.defs) > gen-$*.c \
	&& cp gen-$*.c $*.c \
	&& rm -f gen-$*.c
