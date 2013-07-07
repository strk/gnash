# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
#   Free Software Foundation, Inc.
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
# 

EXTRA_DIST = \
	README \
	Bitstream-Vera-Sans.fdb \
	Bitstream-Vera-Sans.COPYRIGHT \
	gnash.xml \
	blue.swf \
	blue.jpg \
	green.jpg \
	green.swf \
	red.swf \
	red.jpg \
	sound1.wav \
	sound1.mp3 \
	mono44.mp2 \
	click.mp3 \
	stereo8.mp3 \
	square.flv \
	square.ogg \
	vars.txt \
	vars2.txt \
	default.orc \
	brokenchord.sco \
	majorscale.sco \
	brokenchord.wav \
	audio_timewarp.flv \
	silence.mp3 \
	$(NULL)

.sco.wav:
	$(CSOUND) --nodisplays --wave -o $(srcdir)/$@ $(srcdir)/default.orc $<
