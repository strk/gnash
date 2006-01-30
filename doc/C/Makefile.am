## Process this fillocatee with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

# Building a PDF using Apache FOP only works with fop-0.20.5 for now.
include $(top_srcdir)/doc/xmldocs.make

STYLESDIR = @docbook_styles@
EXTRA_DIST += \
	gnash.html \
	gnash.pdf \
	gnash.1

figdir = images
docname = gnash
lang = C
omffile = gnash.omf
entities = legal.xml
all:
	@echo "Nothing to do in doc/C for target all"

alldocs: html pdf man info

html: gnash.html
pdf:  gnash.pdf
man:  gnash.1
texi: gnash.texi
info: gnash.info
gnash.pdf gnash.html gnash.texi gnash.info gnash.1: gnash.xml

SUFFIXES = .xml .html .texi .pdf .info .1

gnash.xml: sources.xml internals.xml

.xml.html:
	@echo "Making HTML from XML source..."
	@if test x"$(XSLTPROC)" != x; then \
	  $(XSLTPROC) --output $@ --nonet $(STYLESDIR)/html/docbook.xsl $< ; \
	else \
	  echo "You need to install the xsltproc package"; \
	fi

.xml.pdf:
	@echo "Making PDF from XML source..."
	@if test x"$(FOP)" != x; then \
	  $(FOP) -xsl $(STYLESDIR)/fo/docbook.xsl -xml $< -pdf $@; \
	else \
	  echo "You need to install the fop package"; \
	fi

.xml.texi:
	@echo "Making Texinfo from XML source..."
	@if test x"$(DB2X_XSLTPROC)" != x -a x"$(DB2X_TEXIXML)" != x; then \
	  $(DB2X_XSLTPROC) -s texi $(srcdir)/gnash.xml -o gnash.txml; \
	  $(DB2X_TEXIXML) gnash.txml; \
	fi
	-@$(RM) gnash.txml

.texi.info:
	@echo "Making INFO from Texinfo source..."
	@if test x"$(MAKEINFO)" != x; then \
	  $(MAKEINFO) --force gnash.texi; \
	else \
	  echo "You need to install the texinfo package"; \
	fi
	-@$(RM) gnash.texi

dist-hook: app-dist-hook

lint:
	@xmllint $(srcdir)/gnash.xml

CLEANFILES += \
	gnash.pdf  \
	gnash.rtf  \
	gnash.html \
	gnash.omf.out \
	manpage.links  \
	manpage.refs  \
	gnash.tex  \
	gnash.log  \
	gnash.info \
	gnash.texi \
	gnash.txml \
	gnash.1    \
	gnash.mxml \
	omf_timestamp

#	@xsltproc --output pgd.1 --nonet $(STYLESDIR)/manpages/docbook.xsl $(srcdir)/gnash-man.xml
gnash.1: gnash-man.xml
	@echo "Making man pages from XML source..."
	@if test x"$(DB2X_XSLTPROC)" != x -a x"$(DB2X_MANXML)" != x; then \
	  $(DB2X_XSLTPROC) -s man $(srcdir)/gnash-man.xml -o gnash.mxml;\
	  $(DB2X_MANXML) gnash.mxml;\
	fi
	-@$(RM) gnash.mxml


# FIXME: install man page
install-data-hook: install-data-hook-omf gnash.html gnash.pdf
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/doc/$(PACKAGE)-$(VERSION)/images;
	$(INSTALL_DATA) gnash.html $(DESTDIR)$(datadir)/doc/$(PACKAGE)-$(VERSION);
	$(INSTALL_DATA) gnash.pdf  $(DESTDIR)$(datadir)/doc/$(PACKAGE)-$(VERSION);
	for png in $(srcdir)/images/*.png; do \
	    if test -f $$png; then \
	      $(INSTALL_DATA) $$png $(DESTDIR)$(datadir)/doc/$(PACKAGE)-$(VERSION)/images; \
	    fi \
         done

uninstall-hook: 
	-rm -fr $(DESTDIR)$(datadir)/doc/$(PACKAGE)-$(VERSION)

# install GNOME help files, which are basically the html output
# install-ghelp: gnash.html
# 	$(mkinstalldirs) $(gnash_helpdir)/images
# 	$(INSTALL_DATA) gnash.html $(gnash_helpdir)
# 	$(INSTALL_DATA) images/*.png $(gnash_helpdir)/images

#	$(INSTALL_DATA) $(srcdir)/images/*.png $(gnash_helpdir)/images

install-doc:


