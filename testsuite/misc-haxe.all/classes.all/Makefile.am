## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

EXTRA_DIST = DejaGnu.hx

TEST_DRIVERS = classes.exp

# These get there values from the Makefile fragments. Each fragment
# appends to these.
check_SCRIPTS =
HAXE_FLAGS = -cp $(srcdir) 

# These classes are in the flash namespace
include $(srcdir)/accessibility/accessibility.am
include $(srcdir)/display/display.am
include $(srcdir)/errors/errors.am
include $(srcdir)/events/events.am
include $(srcdir)/external/external.am
include $(srcdir)/geom/geom.am
include $(srcdir)/media/media.am
include $(srcdir)/net/net.am
include $(srcdir)/printing/printing.am
include $(srcdir)/system/system.am
include $(srcdir)/text/text.am
include $(srcdir)/ui/ui.am
include $(srcdir)/utils/utils.am
include $(srcdir)/xml/xml.am

#/usr/bin/haxe -swf-version 9 -cp /home/rob/projects/gnu/gnash-bzr/avm2/testsuite/as3/classes.all -cp /home/rob/projects/gnu/gnash-bzr/avm2/testsuite/as3/classes.all/media -swf Sound_as3.swf -main Sound_as3.hx
SUFFIXES = hx swf

# build multiple output formats from the same test case. exit with failure if
# the test case doesn't compile for v9, it's a test cases bug. Not all classes
# support all versions, so in those cases justy ignore the errors as it's less
# messy than testing for each test case to see if it supports that version.
.hx.swf:
	-@newname=`echo $(@F) | sed -e 's:_as:_asv9:'`; \
	$(HAXE) -swf-version 9 $(HAXE_FLAGS) -swf $${newname} -main $(@F:%.swf=%.hx)
	-@newname=`echo $(@F) | sed -e 's:_as:_asv8:'`; \
	$(HAXE) -swf-version 8  $(HAXE_FLAGS) -swf $${newname} -main $(@F:%.swf=%.hx)
	-@newname=`echo $(@F) | sed -e 's:_as:_asv7:'`; \
	$(HAXE) -swf-version 7  $(HAXE_FLAGS) -swf $${newname} -main $(@F:%.swf=%.hx)
	-@newname=`echo $(@F) | sed -e 's:_as:_asv6:'`; \
	$(HAXE) -swf-version 6  $(HAXE_FLAGS) -swf $${newname} -main $(@F:%.swf=%.hx)

CLEANFILES =  \
	gnash-dbg.log \
	site.exp.bak \
	*-runner \
	testrun.* \
	*.swf \
	PP_*.hx \
	*_testrunner \
	*_Runner

check-DEJAGNU: site.exp $(check_SCRIPTS)
	@runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../../gnashrc $$runtest $(RUNTESTFLAGS) $(TEST_DRIVERS); true; \
	else \
	  echo "WARNING: could not find \`runtest'" 1>&2; \
          for i in "$(TEST_CASES)"; do \
	    GNASH_GC_TRIGGER_THRESHOLD=0 GNASHRC=../../gnashrc $(SHELL) $$i; \
	  done; \
	fi

