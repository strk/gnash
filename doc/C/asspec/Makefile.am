## Process this fillocatee with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
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

# Building a PDF using Apache FOP only works with fop-0.20.5 for now.
include $(top_srcdir)/doc/xmldocs.make

infodir = ${prefix}/share/info
figdir = ../images
docname = asspec
lang = C
omffile = asspec-C.omf
entities = \
	accessibility.xml \
	array.xml \
	boolean.xml \
	button.xml \
	camera.xml \
	color.xml \
	contextmenu.xml	\
	customactions.xml \
	date.xml \
	error.xml \
	function.xml \
	key.xml	\
	legal.xml \
	loadvars.xml \
	localconnection.xml \
	math.xml \
	microphone.xml \
	mouse.xml \
	movieclip.xml \
	moviecliploader.xml \
	netconnection.xml \
	netstream.xml \
	number.xml \
	selection.xml \
	sharedobject.xml \
	sound.xml \
	stage.xml \
	string.xml \
	system.xml \
	textfield.xml \
	textformat.xml \
	textsnapshot.xml \
	video.xml \
	xml.xml	\
	xmlnode.xml \
	xmlsocket.xml

xml_files = $(entities) $(docname).xml

html: asspec.html
pdf:  asspec.pdf
man:  asspec.1 
texi: asspec.texi
info: asspec.info
asspec.pdf asspec.html asspec.texi: $(xml_files)

SUFFIXES = .xml .html .texi .pdf .info .1 .fo

EXTRA_DIST = \
	$(xml_files) \
	$(omffile) \
	asspec.texi \
	asspec.info \
	asspec.html

if ENABLE_FOP
.xml.pdf:
	$(FOP) -xsl $(docbook_styles)/fo/docbook.xsl -xml $< -pdf $@
else
if ENABLE_XMLTEX
.xml.fo:
	$(XSLTPROC) --output $@ --nonet $(docbook_styles)/fo/docbook.xsl $<

.fo.pdf:
	$(PDFXMLTEX) -interaction=nonstopmode $<
else
.xml.pdf:
	touch $@
endif
endif


.xml.html:
if ENABLE_HTML
	$(XSLTPROC) --output $@ --nonet $(docbook_styles)/html/docbook.xsl $<
else
	touch $@
endif

#	basefile=$(notdir $(basename $<));
.xml.texi:
if ENABLE_TEXI
	basefile=`echo $< | sed -e  's,^.*/,,' -e 's,\.xml$$,,'`; \
	$(DB2X_XSLTPROC) -s texi $< --output $${basefile}.txml; \
	if test x"$(DB2X_VERSION)" = x"0.8.5" -o x"$(DB2X_VERSION)" = x"0.8.7"; then \
	  $(DB2X_TEXIXML) --encoding=us-ascii//TRANSLIT $${basefile}.txml; \
	  sed -e "s/setfilename $${basefile}_manual_v.*\.info/setfilename $${basefile}.info/" $${basefile}_manual_v*.texi > $${basefile}.texi ; \
	  $(RM) $${basefile}_manual_v*.texi; \
	else \
	  $(DB2X_TEXIXML) --encoding=us-ascii//TRANSLIT --to-stdout $${basefile}.txml > $${basefile}.texi ; \
	  $(RM) $${basefile}_manual_v*.texi; \
	fi
else
	touch $@
endif

.texi.info:
	-$(MAKEINFO) --force $<

asspec.1: asspec-man.xml
if ENABLE_MAN
	$(DB2X_XSLTPROC) -s man $(srcdir)/asspec-man.xml -o asspec.mxml
	$(DB2X_MANXML) asspec.mxml
	$(RM) asspec.mxml
else
	touch asspec.1
endif

alldocs: html pdf man info

lint:
	@xmllint $(srcdir)/asspec.xml

CLEANFILES = \
	asspec-C.omf.out \
	manpage.links  \
	manpage.refs  \
	asspec.tex  \
	asspec.fo  \
	asspec.log  \
        asspec.texi \
	asspec_manual_*.texi \
	asspec.txml \
	asspec.mxml \
	omf_timestamp

MAINTAINERCLEANFILES = \
        asspec.pdf       \
        asspec.1         \
        asspec.info      \
        asspec.html

install-info-hook: asspec.info
	test -d "$(DESTDIR)$(infodir)" || $(mkinstalldirs) "$(DESTDIR)$(infodir)"
	$(INSTALL_DATA) $< $(DESTDIR)$(infodir)/asspec.info 
	-if (install-info --version) >/dev/null 2>&1; then \
	  version=`install-info --version | grep -c 'GNU texinfo'`; \
	  if test $$version -eq 0; then \
	    install-info --menuentry="Asspec" --description="GNU Flash Player" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/asspec.info >/dev/null 2>&1; \
	  else \
	    install-info --entry="* Asspec: (asspec). GNU Flash Player" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/asspec.info >/dev/null 2>&1; \
	  fi; \
	fi

uninstall-info-hook:
	$(RM) $(DESTDIR)$(infodir)/asspec.info

