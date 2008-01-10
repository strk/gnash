#!/usr/bin/python
#
# gnashPythonExample.py: An python script demonstrating Gnash bindings.
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
# Do not rely on this interface remaining stable!
#
# The greatest limitation at present is the single movie: calling
# obj = gnash.Player() more than once results in an assertion
# failure as we try to initGnash() twice.

# The lovely gnash module is named:
import gnash
import sys

# Functions are accessed through the Player() class
player = gnash.Player()

# The initialization of the player is split into three stages. First,
# set the base URL:
player.setBaseURL("../../testsuite/movies.all/gravity.swf")

# Then instruct Gnash to load the movie from the URL:
if player.loadMovie():
    print "Movie successfully created."
else:
    print "Load of movie failed."
    sys.exit()

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
else:
    print "VM initialization failed."
    sys.exit()


print "Loaded " +  str(player.swfBytesLoaded()) + " of " + str(player.swfBytesTotal()) + " bytes reported."

# This initializes the named renderer. "Cairo", "OpenGL" and various
# AGG types are possible. Asking for a non-existent renderer results
# in a runtime exception.
if player.addRenderer("AGG_RGB565"):
    print "Renderer added."

# Once the movie is loaded, you can advance to the next frame of the movie.
# This may not be the directly following frame, or even a different one at all.
# You do not need to add a renderer to advance through the movie.
print "We start at frame " + str(player.currentFrame()) + "."

# Advance 10 times.
for i in range(0,10):
    player.advance()
    print player.currentFrame()

print "After 10 advances we are at frame " + str(player.currentFrame()) + " (You don't necessarily move to the next frame when you advance)."

# Render like this (don't expect to see anything...)
player.render()

# You can also make time pass in an instant:
player.advanceClock(199)

# or press a key
player.pressKey(65)






