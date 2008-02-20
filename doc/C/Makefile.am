## Process this fillocatee with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

SUBDIRS = # asspec
infodir = ${prefix}/share/info
figdir = images
docname = gnash
lang = C
omffile = gnash-C.omf
entities = \
	internals.xml \
	amf.xml	\
	appendix.xml \
	debugger.xml \
	extensions.xml \
	fdl-appendix.xml \
	fileext.xml \
	gtkext.xml \
	mysqlext.xml \
	object.xml \
	opcodes.xml \
	plugin.xml \
	rtmp.xml \
	app_authors.xml	\
	conformance.xml	\
	gpl-appendix.xml \
	legal.xml \
	logging.xml \
	actionscript/main.xml \
	actionscript/as_value.xml \
	actionscript/new_as_class.xml \
	asspec/accessibility.xml \
	asspec/array.xml \
	asspec/asspec.xml \
	asspec/boolean.xml \
	asspec/button.xml \
	asspec/camera.xml \
	asspec/color.xml \
	asspec/contextmenu.xml	\
	asspec/customactions.xml \
	asspec/date.xml	\
	asspec/error.xml \
	asspec/fdl-appendix.xml	\
	asspec/function.xml \
	asspec/key.xml \
	asspec/legal.xml \
	asspec/loadvars.xml \
	asspec/localconnection.xml \
	asspec/math.xml	\
	asspec/microphone.xml \
	asspec/mouse.xml \
	asspec/movieclip.xml \
	asspec/moviecliploader.xml \
	asspec/netconnection.xml \
	asspec/netstream.xml \
	asspec/number.xml \
	asspec/selection.xml \
	asspec/sharedobject.xml	\
	asspec/sound.xml \
	asspec/stage.xml \
	asspec/string.xml \
	asspec/system.xml \
	asspec/textfield.xml \
	asspec/textformat.xml	\
	asspec/textsnapshot.xml	\
	asspec/video.xml \
	asspec/xml.xml \
	asspec/xmlnode.xml \
	asspec/xmlsocket.xml	\
	usermanual/installation/code_dependencies.xml \
	usermanual/installation/configuration.xml \
	usermanual/installation/custompath_configuration.xml \
	usermanual/installation/documentation_dependencies.xml	\
	usermanual/installation/feature_configuration.xml \
	usermanual/installation/packagemanager.xml \
	usermanual/installation/sources.xml \
	usermanual/installation/testing_dependencies.xml \
	usermanual/installation/cross_compiling.xml	\
	usermanual/installation/building.xml \
	usermanual/installation/install.xml \
	usermanual/doc_authors.xml \
	usermanual/installation.xml \
	usermanual/introduction.xml \
	usermanual/revisions.xml \
	usermanual/usage.xml \
	usermanual/usage/gnashrc.xml \
	usermanual/bugreport.xml \
	usermanual/acknowledgments.xml \
	usermanual/conventions.xml \
	$(NULL)

xml_files = $(docname).xml actionscript.xml

#noinst_SCRIPT = gen-doc.sh
dist_man_MANS = gnash.1 gprocessor.1 dumpshm.1 soldumper.1 cygnal.1
MANSRC = $(dist_man_MANS:.1=.man-xml)

# dist_html_DATA = gnash.html actionscript.html
# dist_info_DATA = gnash.info actionscript.info

html: links gnash.html actionscript.html
pdf:  links gnash.pdf actionscript.pdf
man:  $(dist_man_MANS)
texi: gnash.texi actionscript.texi
info: links gnash.info actionscript.info
gnash.pdf gnash.html gnash.texi: links $(xml_files) $(entities)
actionscript.pdf actionscript.html actionscript.texi: actionscript.xml actionscript/main.xml actionscript/as_value.xml actionscript/new_as_class.xml

SUFFIXES = .xml .html .texi .pdf .info .1 .fo .man-xml

EXTRA_DIST = \
	$(xml_files) \
	$(entities) \
	$(omffile) \
	$(dist_man_MANS) \
	$(MANSRC) \
	actionscript.info \
	gnash.info \
	actionscript.html \
	gnash.html

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
	@$(RM) $*.txml
else
	@if test -d $(top_srcdir)/packaging/doc; then \
	  $(LN_S) -f $(top_srcdir)/packaging/doc/$@.in ./$@; \
	else \
	  touch $@; \
	fi
endif

#	basefile=$(notdir $(basename $<));
.xml.texi:
if ENABLE_TEXI
	-if test x$(DB2X_TEXI) != x; then \
	  $(DB2X_TEXI) --encoding=us-ascii//TRANSLIT $< ; \
	else \
	  basefile="$*"; \
          $(DB2X_XSLTPROC) -s texi $< --output $${basefile}.txml; \
	  if test x"$(DB2X_VERSION)" = x"0.8.5" -o x"$(DB2X_VERSION)" = x"0.8.7"; then \
	    $(DB2X_TEXIXML) --encoding=us-ascii//TRANSLIT $${basefile}.txml; \
	    sed -e "s/setfilename $${basefile}_.*\.info/setfilename $${basefile}.info/" $${basefile}_*.texi > $${basefile}.texi ; \
	  else \
	    $(DB2X_TEXIXML) --encoding=us-ascii//TRANSLIT --to-stdout $${basefile}.txml > $${basefile}.texi ; \
	  fi; \
	fi
endif

.texi.info:
if ENABLE_TEXI
	-$(MAKEINFO) --force $<
	$(RM) *.texi
else
	@if test -d $(top_srcdir)/packaging/doc; then \
	  $(LN_S) -f $(top_srcdir)/packaging/doc/$@.in ./$@; \
	else \
	  touch $@; \
	fi
endif

.man-xml.1:
if ENABLE_MAN
	-@if test x$(DB2X_MAN) != x; then \
	  $(DB2X_MAN) $?; \
        else \
	  $(DB2X_XSLTPROC) -s man $? -o $*.mxml; \
	  $(DB2X_MANXML) $*.mxml; \
	  $(RM) $*.mxml; \
	fi
else
	@if test -d $(top_srcdir)/packaging/doc; then \
	  $(LN_S) -f $(top_srcdir)/packaging/doc/$@.in ./$@; \
	else \
	  touch $@; \
	fi
endif

alldocs: links html pdf man info

# When we build in a seperate build tree, we need links to the images
# subdirectory to have them be included in the final output.
images:
	@if test ! -d images; then \
	   $(LN_S) -f $(srcdir)/images images; \
	fi

# When we build in a seperate build tree, we need links to these
# additional Docbook source subdirectoriers to have them be included
# in the final output without error.
links: images
	@if test ! -d asspec; then \
	   $(LN_S) -f $(srcdir)/asspec; \
	fi
	@if test ! -d usermanual; then \
	   $(LN_S) -f $(srcdir)/usermanual; \
	fi
	@if test ! -d actionscript; then \
	   $(LN_S) -f $(srcdir)/actionscript; \
	fi

lint:
	@xmllint $(srcdir)/gnash.xml

CLEANFILES = \
	gnash-C.omf.out \
	manpage.links  \
	manpage.refs  \
	gnash.tex  \
	gnash.fo  \
	gnash.log  \
        *.texi \
	*.txml \
	*.mxml \
	omf_timestamp

MAINTAINERCLEANFILES = \
        gnash.pdf \
	$(MANS) \
        gnash.info \
        gnash.html \
	actionscript.html \
	actionscript.info

install-info-hook: gnash.info
	@test -d "$(DESTDIR)$(infodir)" || $(mkinstalldirs) "$(DESTDIR)$(infodir)"
	$(INSTALL_DATA) $< $(DESTDIR)$(infodir)/gnash.info 
	-if (install-info --version) >/dev/null 2>&1; then \
	  version=`install-info --version | grep -c 'GNU texinfo'`; \
	  if test $$version -eq 0; then \
	    install-info --menuentry="Gnash" --description="GNU Flash Player" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash.info >/dev/null 2>&1; \
	  else \
	    install-info --entry="* Gnash: (gnash). GNU Flash Player" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash.info >/dev/null 2>&1; \
	  fi; \
	fi

uninstall-info-hook:
	$(RM) $(DESTDIR)$(infodir)/gnash.info

.PHONY: images
