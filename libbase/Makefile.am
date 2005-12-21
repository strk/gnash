## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

AUTOMAKE_OPTIONS = 

AM_CXXFLAGS = $(SDL_CFLAGS) $(INCLUDES)
AM_CPPFLAGS = -Wall
AM_LDFLAGS = $(OPENGL_LIBS) $(GLU_LIBS) $(SDL_LIBS)

lib_LTLIBRARIES = libbase.la

# These headers get installed
# include_HEADERS = log.h err.h proc.h serial.h xantrex.h outback.h

INCLUDES = -I.. $(srcdir) \
        $(LIBPNG_INCLUDE) \
        $(ZLIB_INCLUDE) \
        $(JPEGLIB_INCLUDE) \
        -I$(top_srcdir)

libbase_la_SOURCES = \
	config.cpp				\
	container.cpp				\
	demo.cpp				\
	dlmalloc.c				\
	file_util.cpp				\
	image.cpp				\
	image_filters.cpp			\
	jpeg.cpp				\
	membuf.cpp				\
	ogl.cpp					\
	png_helper.cpp				\
	postscript.cpp				\
	triangulate_float.cpp			\
	triangulate_sint32.cpp			\
	tu_file.cpp				\
	tu_file_SDL.cpp				\
	tu_random.cpp				\
	tu_timer.cpp				\
	tu_types.cpp				\
	utf8.cpp				\
	utility.cpp				\
	zlib_adapter.cpp

noinst_HEADERS = \
	config.h	\
	container.h	\
	cschema.h	\
	demo.h		\
	dlmalloc.h	\
	file_util.h	\
	grid_index.h	\
	image.h		\
	jpeg.h		\
	membuf.h	\
	ogl.h		\
	png_helper.h	\
	postscript.h	\
	smart_ptr.h	\
	triangulate.h	\
	triangulate_impl.h	\
	tu_config.h	\
	tu_file.h	\
	tu_math.h	\
	tu_opengl_includes.h	\
	tu_random.h	\
	tu_swap.h	\
	tu_timer.h	\
	tu_types.h	\
	utf8.h		\
	utility.h	\
	zlib_adapter.h

#libbase_la_LDFLAGS = -module -avoid-version -no-undefined
#libbase_la_LIBDADD =  #@DL_LIB@  # $(LIBLTDL)

# These are basically copied from the Makefile.in produced by Automake.
# Unfortunately the -local options for changing these don't work...
# so we have to redefine them since we want to install with the package
# name for our library and headers to avoid any problems. So the only
# thing changes is includedir to pkgincludedir and libdir to pkglibdir.
install-includeHEADERS:
	@$(NORMAL_INSTALL)
	test -z "$(pkgincludedir)" || $(mkdir_p) "$(DESTDIR)$(pkgincludedir)"
	@list='$(include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f=$(am__strip_dir) \
	  echo " $(includeHEADERS_INSTALL) '$$d$$p' '$(DESTDIR)$(pkgincludedir)/$$f'"; \
	  $(includeHEADERS_INSTALL) "$$d$$p" "$(DESTDIR)$(pkgincludedir)/$$f"; \
	done

uninstall-includeHEADERS: uninstall-libLTLIBRARIES
	@$(NORMAL_UNINSTALL)
	@list='$(include_HEADERS)'; for p in $$list; do \
	  f=$(am__strip_dir) \
	  echo " rm -f '$(DESTDIR)$(pkgincludedir)/$$f'"; \
	  rm -f "$(DESTDIR)$(pkgincludedir)/$$f"; \
	done

install-libLTLIBRARIES: $(lib_LTLIBRARIES)
	@$(NORMAL_INSTALL)
	test -z "$(pkglibdir)" || $(mkdir_p) "$(DESTDIR)$(pkglibdir)"
	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    f=$(am__strip_dir) \
	    echo " $(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) '$$p' '$(DESTDIR)$(pkglibdir)/$$f'"; \
	    $(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$(DESTDIR)$(pkglibdir)/$$f"; \
	  else :; fi; \
	done

uninstall-libLTLIBRARIES:
	@$(NORMAL_UNINSTALL)
	@set -x; list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  p=$(am__strip_dir) \
	  echo " $(LIBTOOL) --mode=uninstall rm -f '$(DESTDIR)$(pkglibdir)/$$p'"; \
	  $(LIBTOOL) --mode=uninstall rm -f "$(DESTDIR)$(pkglibdir)/$$p"; \
	done

# Rebuild with GCC 4.x Mudflap support
mudflap:
	@echo "Rebuilding with GCC Mudflap support"
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fmudflap" LDFLAGS="$(LDFLAGS) -lmudflap"

clean-hook:
	-rm -f core.*

