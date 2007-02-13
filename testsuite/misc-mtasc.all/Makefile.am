## Process this fill with automake to generate Makefile.in
# 
# Copyright (C) 2007 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
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

if ENABLE_MTASC
if ENABLE_MING

DEJAGNU_SO_PATH = ../Dejagnu.swf
DEJAGNU_SO_FINI = $(srcdir)/../actionscript.all/dejagnu_so_fini.as
MTASC_FLAGS= -cp $(MTASC_CLASSPATH) -cp $(srcdir) -v -header 800:600:20

ASTESTS =			\
	hello.as		\
 	$(NULL)


.as.swf:
	$(MTASC) $(MTASC_FLAGS) -swf mtasc-$@ -main $<
	$(MAKESWF) -o $@ $(DEJAGNU_SO_PATH) mtasc-$@  $(DEJAGNU_SO_FINI)

CLEANFILES = *.swf

endif
endif
