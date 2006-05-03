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

# Linking Gnash statically or dynamically with other modules is making a
# combined work based on Gnash. Thus, the terms and conditions of the GNU
# General Public License cover the whole combination.
#
# As a special exception, the copyright holders of Gnash give you
# permission to combine Gnash with free software programs or libraries
# that are released under the GNU LGPL and with code included in any
# release of Talkback distributed by the Mozilla Foundation. You may
# copy and distribute such a system following the terms of the GNU GPL
# for all but the LGPL-covered parts and Talkback, and following the
# LGPL for the LGPL-covered parts.
#
# Note that people who make modified versions of Gnash are not obligated
# to grant this special exception for their modified versions; it is their
# choice whether to do so. The GNU General Public License gives permission
# to release a modified version without this exception; this exception
# also makes it possible to release a modified version which carries
# forward this exception.
#

# Building a PDF using Apache FOP only works with fop-0.20.5 for now.
include $(top_srcdir)/doc/xmldocs.make

STYLESDIR = @docbook_styles@

infodir = ${prefix}/share/info
figdir = images
docname = gnash
lang = C
omffile = gnash-C.omf
entities = \
	sources.xml		\
	internals.xml		\
	amf.xml			\
	accessibility.xml	\
	appendix.xml		\
	array.xml		\
	boolean.xml		\
	button.xml		\
	camera.xml		\
	color.xml		\
	contextmenu.xml		\
	customactions.xml	\
	date.xml		\
	error.xml		\
	fdl-appendix.xml	\
	function.xml		\
	key.xml			\
	legal.xml		\
	loadvars.xml		\
	localconnection.xml	\
	logging.xml		\
	math.xml		\
	microphone.xml		\
	mouse.xml		\
	movieclip.xml		\
	moviecliploader.xml	\
	netconnection.xml	\
	netstream.xml		\
	number.xml		\
	object.xml		\
	opcodes.xml		\
	plugin.xml		\
	selection.xml		\
	sharedobject.xml	\
	sound.xml		\
	stage.xml		\
	string.xml		\
	system.xml		\
	textfield.xml		\
	textformat.xml		\
	textsnapshot.xml	\
	video.xml		\
	xml.xml			\
	xmlnode.xml		\
	xmlsocket.xml

xml_files = $(entities) $(docname).xml

#noinst_SCRIPT = gen-doc.sh
dist_man_MANS = gnash.1

html: gnash.html
pdf:  gnash.pdf
man:  gnash.1
texi: gnash.texi
info: gnash.info
gnash.pdf gnash.html gnash.texi: $(xml_files)

SUFFIXES = .xml .html .texi .pdf .info .1 .fo

EXTRA_DIST = \
	$(xml_files)		\
	$(omffile)		\
	gnash.texi              \
	gnash-man.xml           \
	gnash.info              \
	gnash.html              \
	gnash.1

if ENABLE_FOP
.xml.pdf:
	$(FOP) -xsl $(STYLESDIR)/fo/docbook.xsl -xml $< -pdf $@
	touch $@
else
.xml.fo:
if ENABLE_XMLTEX
	$(XSLTPROC) --output $@ --nonet $(STYLESDIR)/fo/docbook.xsl $<
else
	touch $@
endif
.fo.pdf:
if ENABLE_XMLTEX
	$(PDFXMLTEX) $<
else
	touch $@
endif
endif


.xml.html:
if ENABLE_HTML
	$(XSLTPROC) --output $@ --nonet $(STYLESDIR)/html/docbook.xsl $<
else
	touch $@
endif

#	basefile=$(notdir $(basename $<));
.xml.texi:
if ENABLE_TEXI
	basefile=`echo $< | sed -e  's,^.*/,,' -e 's,\.xml$$,,'`; \
	$(DB2X_XSLTPROC) -s texi $< --output $${basefile}.txml; \
	if test x"$(DB2X_VERSION)" = x"0.8.5"; then \
	  $(DB2X_TEXIXML) $${basefile}.txml; \
	  sed -e "s/setfilename $${basefile}_manual_v.*\.info/setfilename $${basefile}.info/" $${basefile}_manual_v*.texi > $${basefile}.texi ; \
	  $(RM) $${basefile}_manual_v*.texi; \
	else \
	  $(DB2X_TEXIXML) --to-stdout $${basefile}.txml > $${basefile}.texi ; \
	  $(RM) $${basefile}_manual_v*.texi; \
	fi
else
	touch $@
endif

.texi.info:
	-$(MAKEINFO) --force $<

gnash.1: gnash-man.xml
if ENABLE_MAN
	$(DB2X_XSLTPROC) -s man $(srcdir)/gnash-man.xml -o gnash.mxml
	$(DB2X_MANXML) gnash.mxml
	$(RM) gnash.mxml
else
	touch gnash.1
endif

alldocs: html pdf man info

lint:
	@xmllint $(srcdir)/gnash.xml

CLEANFILES = \
	gnash-C.omf.out \
	manpage.links  \
	manpage.refs  \
	gnash.tex  \
	gnash.log  \
        gnash.texi \
	gnash_manual_*.texi \
	gnash.txml \
	gnash.mxml \
	omf_timestamp

MAINTAINERCLEANFILES = \
        gnash.pdf       \
        gnash.1         \
        gnash.info      \
        gnash.html

install-info-hook: gnash.info
	test -d "$(DESTDIR)$(infodir)" || $(mkinstalldirs) "$(DESTDIR)$(infodir)"
	$(INSTALL_DATA) $< $(DESTDIR)$(infodir)/gnash.info 
	if (install-info --version) >/dev/null 2>&1; then \
	  version=`install-info --version | grep -c 'GNU texinfo'`; \
	  if test $$version -eq 0; then \
	    install-info --menuentry="Gnash" --description="GNU Flash Player" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash.info >/dev/null 2>&1; \
	  else \
	    install-info --entry="* Gnash: (gnash). GNU Flash Player" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash.info >/dev/null 2>&1; \
	  fi; \
	fi

uninstall-info-hook:
	$(RM) $(DESTDIR)$(infodir)/gnash.info

