## Process this file with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = -Wno-portability

POFILES = AUTO


translations = cs de es fi fr it ja sv

# This is GNU make specific, so if you are building Gnash on a BSD
# machine, you probably want to install GNU make in additional to
# BSD make.
pofiles := $(patsubst %,%.po,$(translations))
gmofiles := $(patsubst %,%.gmo,$(translations))

MAINTAINERCLEANFILES = \
	gnash.pot 

CLEANFILES = \
	$(gmofiles) \
	POTFILES 

dirs =  $(top_srcdir)/libbase \
	$(top_srcdir)/backend \
	$(top_srcdir)/libamf \
	$(top_srcdir)/libmedia \
	$(top_srcdir)/libmedia/ffmpeg \
	$(top_srcdir)/libmedia/gst \
	$(top_srcdir)/libcore \
	$(top_srcdir)/libcore/vm \
	$(top_srcdir)/libcore/swf \
	$(top_srcdir)/libcore/parser \
	$(top_srcdir)/libcore/asobj \
	$(top_srcdir)/utilities \
	$(top_srcdir)/plugin \
	$(top_srcdir)/gui \
	$(top_srcdir)/extensions \
	$(top_srcdir)/extensions/dbus \
	$(top_srcdir)/extensions/dejagnu \
	$(top_srcdir)/extensions/fileio \
	$(top_srcdir)/extensions/lirc \
	$(top_srcdir)/extensions/gtk2 \
	$(top_srcdir)/extensions/metome \
	$(top_srcdir)/extensions/mysql
	
srcfiles := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))

EXTRA_DIST = $(pofiles) gnash.pot 

all: $(gmofiles)

POTFILES: $(srcfiles)
	@echo "Building POTFILES..."
	@rm -f POTFILES
	@for i in $(srcfiles) ; do \
	  echo "$$i" >> POTFILES;\
	done

if HAS_GETTEXT

gnash.pot: POTFILES
	$(XGETTEXT) --language=C++ --files-from=POTFILES -o $(srcdir)/gnash.pot \
	  --default-domain=gnash --force \
	  --flag=_:1:pass-c-format --keyword=_ \
	  --copyright-holder='Free Software Foundation, Inc.'

update-po: gnash.pot
	@for i in $(pofiles); do \
	  echo "Merging $$i..." ; \
	  $(MSGMERGE) --update $(srcdir)/$$i $(srcdir)/gnash.pot; \
	done

$(gmofiles): #update-po

SUFFIXES = .po .gmo
.po.gmo:
	$(GMSGFMT) --statistics --directory=$(srcdir) -o $*.gmo $(@:%.gmo=%.po) 

else # not HAS_GETTEXT

$(gmofiles): force
	@echo "WARNING: can't compile $@ translations w/out gettext"

update-po: force
	@echo "WARNING: can't update translation files (.po) w/out gettext"

endif # not HAS_GETTEXT

force:

# install-hook: rename the .gmo files to gnash.mo at installation time
install-data-hook: $(gmofiles)
	@for i in $(translations); do \
	  $(mkinstalldirs) "$(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/"; \
	  if test -f $$i.gmo; then \
	    $(INSTALL_DATA) $$i.gmo $(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/gnash.mo; \
	  fi; \
	done

uninstall-hook:
	for i in $(translations); do \
	  rm -f "$(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/gnash.mo"; \
	done

