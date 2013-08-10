## Process this fillocatee with automake to generate Makefile.in
# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
#   Free Software Foundation, Inc.
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

infodir = ${prefix}/share/info
man1dir = ${mandir}/man1
docname = gnash
lang = C
omffile = gnash-C.omf

# htmldir=docdir, which isn't what we want, so comment the hardcoded
# docdir value for now
# docdir is used to install scrollkeeper files, which isn't supported
# anymore, so maybe it should be removed together with its corresponding
# hooks
# docdir = $(datadir)/$(PACKAGE)/doc/$(docname)/$(lang)
omf_dest_dir=$(datadir)/omf/$(PACKAGE)
scrollkeeper_localstate_dir = $(localstatedir)/scrollkeeper

# These are the top level files for each manual.
XML_MAIN = \
	gnashuser.xml \
	gnashref.xml

# These are the files needed to build the Gnash user
# manual.
COMMON = \
	introduction.xml \
	security.xml \
	legal.xml \
	app_authors.xml \
	doc_authors.xml \
	fdl-appendix.xml \
	bugreport.xml

USERMAN = \
	usermanual/installation.xml \
	usermanual/revisions.xml \
	usermanual/usage.xml \
	usermanual/glossary.xml \
	usermanual/specification.xml \
	usermanual/gnashrc.xml \
	usermanual/acknowledgments.xml \
	usermanual/conventions.xml

# These are the files needed to build the Gnash reference
# manual.
REFMAN = \
	refmanual/amf.xml \
	refmanual/as_value.xml \
	refmanual/building.xml \
	refmanual/code_dependencies.xml \
	refmanual/codestyle.xml \
	refmanual/configuration.xml \
	refmanual/cross_compiling.xml \
	refmanual/custompath_configuration.xml \
	refmanual/documentation_dependencies.xml \
	refmanual/feature_configuration.xml \
	refmanual/install.xml \
	refmanual/internals.xml \
	refmanual/logging.xml \
	refmanual/memory.xml \
	refmanual/new_as_class.xml \
	refmanual/object.xml \
	refmanual/plugin.xml \
	refmanual/revisions.xml \
	refmanual/rtmp.xml \
	refmanual/sources.xml \
	refmanual/testing_dependencies.xml \
	refmanual/extensions/extensions.xml \
	refmanual/extensions/fileext.xml \
	refmanual/extensions/gtkext.xml \
	refmanual/extensions/mysqlext.xml

EXTRA_DIST = \
	$(USERMAN) \
	$(REFMAN) \
	$(COMMON) \
	$(XML_MAIN) \
	$(MANSRC) \
	preformatted/gnashuser.html.in \
	preformatted/gprocessor.1.in \
	preformatted/cygnal.1.in \
	preformatted/gnash.1.in \
	preformatted/gnash_ref.info.in \
	preformatted/soldumper.1.in \
	preformatted/flvdumper.1.in \
	preformatted/gnashref.html.in \
	preformatted/gnash_user.info.in \
	preformatted/rtmpget.1.in \
	preformatted/findmicrophones.1.in \
	preformatted/findwebcams.1.in \
	fb-gnash.1 \
	sdl-gnash.1 \
	dump-gnash.1 \
	gtk-gnash.1 \
	qt4-gnash.1 \
	gnash-gtk-launcher.1 \
	gnash-qt-launcher.1
#	preformatted/dumpshm.1.in

#noinst_SCRIPT = gen-doc.sh
# We list the texi and info pages twice, as differing versions of the 
# docbook-utils transform the name at different steps in the process.
# This is an easy way to handle it either way, since we only need it as
# dependency anyway.
MANPAGES = \
	gnash.1 \
	gprocessor.1 \
	soldumper.1 \
	flvdumper.1 \
	cygnal.1 \
	rtmpget.1 \
	findmicrophones.1 \
	findwebcams.1
#	dumpshm.1

# These are just aliases for the default gnash man page.
EXTRAMANPAGES =

if BUILD_GTK_GUI
EXTRAMANPAGES += gtk-gnash.1 gnash-gtk-launcher.1
endif

if BUILD_QT4_GUI
EXTRAMANPAGES += qt4-gnash.1 gnash-qt-launcher.1
endif

if BUILD_SDL_GUI
EXTRAMANPAGES += sdl-gnash.1
endif

if BUILD_FB_GUI
EXTRAMANPAGES += fb-gnash.1
endif

if BUILD_DUMP_GUI
EXTRAMANPAGES += dump-gnash.1
endif

MANSRC = $(MANPAGES:.1=.man-xml)
INFOS  = gnash_user.info gnash_ref.info
HTMLS  = $(XML_MAIN:.xml=.html)
PDFS   = $(XML_MAIN:.xml=.pdf)
TEXIS  = gnash_user.texi gnashuser.texi gnash_ref.texi gnashref.texi

all-local: info html man

gnashuser.html gnashuser.pdf gnash_user.info: gnashuser.xml $(USERMAN) $(COMMON)
gnashref.html gnashref.pdf gnash_ref.info: gnashref.xml $(REFMAN) $(COMMON)

# Convienience rules for developers of these manuals. By default
# all output formats are built if the Docbook tools are installed
# and working correctly.
html: $(HTMLS)
pdf:  $(PDFS)
man:  $(MANPAGES) $(EXTRAMANPAGES)
info: $(INFOS)
if DOCBOOK
texi: $(TEXIS)
endif

#
# Gnash uses suffix rules to build all the various supported
# output formats. This section contains all the code that does
# the document convesion from docbook to whatever. Note that
# all of these are conditional, as the tools may not be
# installed on all systems, and on some systems, getting a fully
# working Docbook tools system can problematic. In this case 
# preformatted versions are in the top level packaging/doc
# directory, so we just link to them so they can be installed.
# While it's a bit of a pain in the neck for developers to
# keep the preformatted versions up to date, it's much easier
# for end users because the documentation in xml, html, and info
# formats is always available.
#
SUFFIXES = .xml .html .texi .pdf .info .1 .fo .man-xml .in

if ENABLE_FOP
.xml.pdf:
	-$(FOP) -xsl $(docbook_styles)/fo/docbook.xsl -xml $< -pdf $@
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
	-$(XSLTPROC) --output $@ --nonet $(docbook_styles)/html/docbook.xsl $<
	@$(RM) $*.txml
else
	@if test -d $(srcdir)/preformatted; then \
	  echo "WARNING: Linking to preformatted version of $@, it could be out of date."; \
	  $(LN_S) -f $(srcdir)/preformatted/$@.in ./$@; \
	else \
	  touch $@; \
	fi
endif

# The first test seems to be the default for Fedora/Redhat/Centos based distributions,
# which use a wrapper script for jade.
# Ubuntu/Debian uses a real perl script that actually does the real work
.xml.texi:
if ENABLE_TEXI
	-if test x$(DB2X_TEXI) != x; then \
	  out=`echo $* | sed -e 's:gnash:gnash_:'`; \
	  $(DB2X_TEXI) --info --encoding=us-ascii//TRANSLIT --string-param
directory-description="Gnash" --string-param output-file=$${out} $<; \
	else \
	  basefile="$*"; \
          $(DB2X_XSLTPROC) -s texi $< --output $${basefile}.txml; \
	  $(DB2X_TEXIXML) --info --encoding=us-ascii//TRANSLIT $${basefile}.txml ; \
	  rm $${basefile}.txml; \
	fi
endif

.texi.info:
	touch $@

.man-xml.1:
if ENABLE_MAN
	-if test x$(DB2X_MAN) != x; then \
	  $(DB2X_MAN) $?; \
        else \
	  $(DB2X_XSLTPROC) -s man $? -o $*.mxml; \
	  $(DB2X_MANXML) $*.mxml; \
	  $(RM) $*.mxml; \
	fi
else
	@if test -d $(srcdir)/preformatted; then \
	  echo "WARNING: Linking to preformatted version of $@, it could be out of date."; \
	  $(LN_S) -f $(srcdir)/preformatted/$@.in ./$@; \
	else \
	  touch $@; \
	fi
endif

# As these man page alias files are very small, we just copy them
# into the build rather than symbolically link them like we used to.
$(EXTRAMANPAGES): gnash.1
	@if ! test -e $@; then \
	  cp $(srcdir)/$@ .; \
	fi

gprocessor.1: $(srcdir)/gprocessor.man-xml
#dumpshm.1: $(srcdir)/dumpshm.man-xml
soldumper.1: $(srcdir)/soldumper.man-xml
flvdumper.1: $(srcdir)/flvdumper.man-xml
cygnal.1: $(srcdir)/cygnal.man-xml
findmicrophones.1: $(srcdir)/findmicrophones.man-xml
findwebcams.1: $(srcdir)/findwebcams.man-xml

if DOCBOOK
gnash_user.info: gnashuser.texi
gnash_ref.info: gnashref.texi
gnashuser.texi gnash_user.texi: images gnashuser.xml
gnashref.texi gnash_ref.texi: gnashref.xml
else
gnash_user.info: gnashuser.xml
	-@if test -d $(srcdir)/preformatted; then \
	  echo "WARNING: Linking to preformatted version of $@, it could be out of date."; \
	  $(LN_S) -f $(srcdir)/preformatted/gnash_user.info.in ./gnash_user.info; \
	fi
gnash_ref.info: gnashref.xml
	-@if test -d $(srcdir)/preformatted; then \
	  echo "WARNING: Linking to preformatted version of $@, it could be out of date."; \
	  $(LN_S) -f $(srcdir)/preformatted/gnash_ref.info.in ./gnash_ref.info; \
	fi
gnashref.html: gnashref.xml
	-@if test -d $(srcdir)/preformatted; then \
	  echo "WARNING: Linking to preformatted version of $@, it could be out of date."; \
	  $(LN_S) -f $(srcdir)/preformatted/gnashref.html.in ./gnashref.html; \
	fi
gnashuser.html: gnashref.xml
	-@if test -d $(srcdir)/preformatted; then \
	  echo "WARNING: Linking to preformatted version of $@, it could be out of date."; \
	  $(LN_S) -f $(srcdir)/preformatted/gnashuser.html.in ./gnashuser.html; \
	fi
endif

lint-user:
	@xmllint $(srcdir)/gnashuser.xml

lint-ref:
	@xmllint $(srcdir)/gnashref.xml

# This target adds the images to the dist file, which is
# produced by the dist-bzip2, dist-gzip, or snapshot targets.
# distdir is a built in target for Automake.
dist-hook:
	test -d "$(distdir)/images" || $(mkinstalldirs) "$(distdir)/images"
	-@for file in $(srcdir)/images/*.png $(srcdir)/images/*.txt; do \
	  basefile="`basename $${file}`"; \
	  if test ! -e $(distdir)/images/$${basefile}; then \
	    $(INSTALL_DATA) $$file $(distdir)/images/; \
	  fi; \
	done

# When we build in a seperate build tree, we need links to the images
# subdirectory to have them be included in the final output.
images:
	-@if test ! -e images; then \
	  $(LN_S) -f $(srcdir)/images images; \
	fi;

#
# Installing documentation can be a complex process, as we build
# multiple output formats. This section contains all of the 
# Makefile targets that are used to install or uninstall the
# documentation.
#
install-html-hook: $(HTMLS)
	-@test -d "$(DESTDIR)$(htmldir)" || $(mkinstalldirs) "$(DESTDIR)$(htmldir)"
	-$(INSTALL_DATA) gnashuser.html $(DESTDIR)$(htmldir)
	-$(INSTALL_DATA) gnashref.html $(DESTDIR)$(htmldir)
	@test -d "$(DESTDIR)$(htmldir)/images" || $(mkinstalldirs) "$(DESTDIR)$(htmldir)/images"
	-for file in $(srcdir)/images/*.png; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/images/$$basefile; \
	done

# There are two versions of install-info, with unfortunately different options.
# Rather than test the version at configure time, we do it at runtime instead
# so we can switch versions at installation time.
install-info-hook: $(INFOS)
if ENABLE_INFO
	-@test -d "$(DESTDIR)$(infodir)" || $(mkinstalldirs) "$(DESTDIR)$(infodir)"
	-for i in *.info; do \
	  $(INSTALL_DATA) "$$i" $(DESTDIR)$(infodir)/"$$i"; \
	done; \
	type="`file $(INSTALL_INFO) | grep -ic "perl " 2>&1`"; \
	if test $${type} -gt 0; then \
	  $(INSTALL_INFO) --menuentry="\"Gnash User Manual\"" --quiet --description="\"Gnash User Manual\"" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash_user.info; \
	  $(INSTALL_INFO) --menuentry="\"Gnash Reference Manual\"" --quiet --description="\"Gnash Reference Manual\"" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash_ref.info; \
        else \
	  $(INSTALL_INFO) --entry="\"Gnash User Manual: (gnash_user)	Gnash User Manual\"" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash_user.info; \
	  $(INSTALL_INFO) --entry="\"Gnash Reference Manual (gnash_ref)	Gnash Reference Manual\"" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash_ref.info; \
	fi
else
	echo "Using preformtted output file,"
	-$(LN_S) -f $(srcdir)/preformatted/gnash_user.info.in gnash_user.info
	-$(LN_S) -f $(srcdir)/preformatted/gnash_ref.info.in gnash_ref.info
        -$(INSTALL_INFO) --entry="\"Gnash User Manual: (gnash_user)	Gnash User Manual\"" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash_user.info
	-$(INSTALL_INFO) --entry="\"Gnash Reference Manual (gnash_ref)	Gnash Reference Manual\"" --info-dir=$(DESTDIR)$(infodir) $(DESTDIR)$(infodir)/gnash_ref.info
endif

uninstall-info-hook:
if ENABLE_INFO
	-$(INSTALL_INFO) --dir-file=$(DESTDIR)$(infodir)/dir --remove $(DESTDIR)$(infodir)/gnash_user.info
	-$(INSTALL_INFO)  --dir-file=$(DESTDIR)$(infodir)/dir --remove $(DESTDIR)$(infodir)/gnash_ref.info
endif
	-$(RM) $(DESTDIR)$(infodir)/gnash_user.info
	-$(RM) $(DESTDIR)$(infodir)/gnash_ref.info

uninstall-html-hook:
	-$(RM) $(DESTDIR)$(htmldir)/gnashuser.html
	-$(RM) $(DESTDIR)$(htmldir)/gnashref.html
	-$(RM) $(DESTDIR)$(htmldir)/images/*png

# install-data-hook: install-data-hook-omf
# uninstall-local: uninstall-local-doc uninstall-local-omf

# gtk-gnash.1, qt4-gnash.1, fb-gnash.1, sdl-gnash.1,gnash-gtk-launcher.1 and gnash-qt-launcher.1 are special,
# as they  are created by the --solinks option to the docbook2 tools for creating man pages.
install-man1: $(MANPAGES) $(EXTRAMANPAGES)
	-test -d "$(DESTDIR)$(man1dir)/" || $(mkinstalldirs) "$(DESTDIR)$(man1dir)/"
	-for file in $(MANPAGES) $(EXTRAMANPAGES); do \
	   $(INSTALL_DATA) $$file $(DESTDIR)$(man1dir)/$$file; \
	done

install-data-hook: install-man1 install-info-hook install-html-hook 
uninstall-hook: uninstall-info-hook uninstall-html-hook
	-for file in $(MANPAGES) $(EXTRAMANPAGES); do \
	   $(RM) $(DESTDIR)$(man1dir)/$$file; \
	done

#
# GNOME Help support, which currently isn't supported
# by the Gnash GTK GUI.
#

# if GHELP
# all-local: omf
# INSTALL_DATA_HOOK += omf install-data-omf-hook
# UNINSTALL_HOOK += uninstall-omf-hook
# endif

omf: omf_timestamp

omf_timestamp: $(omffile)
	for file in $(omffile); do \
	    $(SCROLLINSTALL) $(docdir)/$(docname).xml $(srcdir)/$$file $$file.out; \
	done
	touch omf_timestamp

install-data-omf-hook:
	$(mkinstalldirs) $(DESTDIR)$(omf_dest_dir)
	for file in $(omffile); do \
	  $(INSTALL_DATA) $$file.out $(DESTDIR)$(omf_dest_dir)/$$file; \
	done
	-$(SCROLLUPDATE) -v -o $(DESTDIR)$(omf_dest_dir)

uninstall-omf-hook:
	-for file in $(omffile); do \
	    $(RM) $(DESTDIR)$(omf_dest_dir)/$$file; \
	done
	-rmdir $(DESTDIR)$(omf_dest_dir)
	-$(SCROLLUPDATE) -v 

# All but gnash.1 aren't generated, but are just copied from the source
# tree. If building in thew source directory, we don't want to remove
# these, but when building in a separate directory, we do want to remove them.
MAN_ALIASES = \
	`if test x"$(srcdir)" != x"."; then \
	   echo $(EXTRAMANPAGES); \
	fi`

IMAGES = \
	`if test x"$(srcdir)" != x"."; then \
	   echo "images"; \
	fi`

CLEANFILES = \
        $(PDFS) \
	$(MANPAGES) \
	$(MAN_ALIASES) \
	$(INFOS) \
        $(HTMLS) \
	$(TEXIS) \
	gnash-C.omf.out \
	manpage.links  \
	manpage.refs  \
	gnash.fo  \
	gnash.log  \
	gnashuser.txml \
	gnashuser.mxml \
	gnashref.txml \
	gnashref.mxml \
	omf_timestamp \
	$(IMAGES)

# This updates the copies of the manuals that are stored preformatted
# so not everyone needs to get Docbook working well enough to produce
# the documentation. This should only be used by developers after
# making changes to a documentation source file.
if DOCBOOK
update-preformats:
	-@for type in 1 html info; do \
	  for i in *.$${type}; do \
	    name=$${i/\.$${type}/.$${type}.in}; \
	    diff=`diff --brief $$i $(srcdir)/preformatted/$${name}`; \
	    if test ! x"$${diff}" = x; then \
	      echo "Updating the preformatted/$${name}"; \
	      cp $$i $(srcdir)/preformatted/$${name}; \
	    fi; \
	  done; \
	done
endif

# Always execute these targets as they have no real dependencies.
.PHONY: images install-info-hook uninstall-info-hook install-html-hook uninstall-html-hook uninstall-data-hook install-data-hook images install-man1 update-preformats $(EXTRAMANPAGES)

