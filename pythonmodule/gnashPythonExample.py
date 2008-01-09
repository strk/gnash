#!/usr/bin/python
#
# gnash.py: An example python script demonstrating Gnash python bindings.
# 
#   Copyright (C) 2008 Free Software Foundation, Inc.
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
#
#
#
# The Gnash python bindings are loosely based on functions in 
# testsuite/MovieTester.h
#
# They are designed to allow quick setup of the Gnash player while
# catering for different purposes. The module can be used to test
# Gnash from python, but also to extract information (and eventually
# thumbnails) from SWF files.
#
# The greatest limitation at present is the single movie: calling
# obj = gnash.Player() more than once results in an assertion
# failure as we try to initGnash() twice.

# The lovely gnash module is named:
import gnash

# Functions are accessed through the Player() class
player = gnash.Player()

# The initialization of the player is split into three stages. First,
# set the base URL:
player.setBaseURL("/home/benjamin/Download/SWF/SWF6/u2_has.swf")

# Then instruct Gnash to load the movie from the URL:
if player.createMovieDefinition():
    print "Movie successfully created."
else:
    print "Load failed."

# At this stage, you can query movie properties like so:
print "The frame rate of this movie is " + str(player.swfFrameRate()) + " FPS."
print "It has " + str(player.swfFrameCount()) + " frames altogether."
print "Loaded " +  str(player.swfBytesLoaded()) + " of " + str(player.swfBytesTotal()) + "reported bytes."
print "It is version " + str(player.swfVersion()) +"."
print "It is " + str(player.swfWidth()) + "x" + str(player.swfWidth()) + " pixels."
print "URL: " + player.swfURL() + "."


# The third stage completes initialization.
if player.initVM():
    print "VM initialized."
#else:
#  print "VM initialization failed."

print "Loaded " +  str(player.swfBytesLoaded()) + " of " + str(player.swfBytesTotal()) + " bytes reported."

if player.addRenderer("AGG_RGB565"):
    print "Renderer added.";

# You are now able to advance the movie.
print "We start at frame " + str(player.currentFrame()) + "."

for i in range(0,10):
    player.advance()

print "After 10 advances we are at frame " + str(player.currentFrame()) + " (You don't necessarily move to the next frame when you advance)."

# Render like this (don't expect to see anything...)
player.render()

# You can also make time pass in an instant:
player.advanceClock(199)

# or press a key
player.pressKey(65)






