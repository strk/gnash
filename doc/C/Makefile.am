## Process this fillocatee with automake to generate Makefile.in
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

SUFFIXES = .xml .html .texi .1

.xml.html:
	@echo "Making HTML from XML source..."
	@xsltproc --output $@ --nonet $(STYLESDIR)/html/docbook.xsl $<

.xml.pdf:
	@echo "Making PDF from XML source..."
	@fop.sh -xsl $(STYLESDIR)/fo/docbook.xsl -xml $< -pdf $@

.xml.texi:
	db2x_xsltproc -s texi $(srcdir)/gnash.xml -o gnash.txml
	db2x_texixml gnash.txml
	mv gnash_manual_v*.texi gnash.texi
	-@$(RM) gnash.txml

.texi.info:
	makeinfo --force gnash.texi
	mv gnash_manual_v*.info gnash.info
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

gnash.1: gnash-man.xml
	@echo "Making man pages from XML source..."
#	@xsltproc --output pgd.1 --nonet $(STYLESDIR)/manpages/docbook.xsl $(srcdir)/gnash-man.xml
	db2x_xsltproc -s man $(srcdir)/gnash-man.xml -o gnash.mxml
	db2x_manxml gnash.mxml
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


