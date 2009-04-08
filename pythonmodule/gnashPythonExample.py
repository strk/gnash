#!/usr/bin/python
#
# gnashPythonExample.py: An python script demonstrating Gnash bindings.
# 
#   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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

uri = "../../testsuite/movies.all/gravity.swf"

if len(sys.argv) > 1:
	uri = sys.argv[1]

input = open(uri)

# The initialization of the player is split into two stages.
# First, load the movie from the URL (currently only local files):
if player.loadMovie(input):
    print "Movie successfully created."
else:
    print "Load of movie failed."
    sys.exit()

# At this stage, you can query movie properties like so:
print "The frame rate of this movie is " + str(player.swfFrameRate()) + " FPS."
print "It has " + str(player.swfFrameCount()) + " frames altogether."
print "Loaded " +  str(player.swfBytesLoaded()) + " of " + str(player.swfBytesTotal()) + " reported bytes."
print "It is version " + str(player.swfVersion()) +"."
print "It is " + str(player.swfWidth()) + "x" + str(player.swfWidth()) + " pixels."
print "URL: " + player.swfURL() + "."


print "Loaded " +  str(player.swfBytesLoaded()) + " of " + str(player.swfBytesTotal()) + " bytes reported."

# This initializes the named renderer. "Cairo", "OpenGL" and various
# AGG types are possible. Returns False if the renderer does not exist
# or otherwise fails.
if player.setRenderer("AGG_RGB565"):
    print "Renderer added."

# Once the movie is loaded, you can advance to the next frame of the movie.
# This may not be the directly following frame, or even a different one at all.
# You do not need to add a renderer to advance through the movie.
print "We start at frame " + str(player.currentFrame()) + "."

# Advance 10 times.
for i in range(0,10):
    player.advance()
    print "Frame: " + str(player.currentFrame())

print "After 10 advances we are at frame " + str(player.currentFrame()) + " (You don't necessarily move to the next frame when you advance)."

# Render like this (don't expect to see anything):
player.render(True)
# By passing 'True', you can force the renderer to redraw
# the entire window. False redraws only the invalidated bounds 
# calculated by Gnash.

# Turn verbosity on to send debug messages to stdout and the logfile.
player.setVerbosity(2)
# This can't be turned off at the moment.

# You can also make time pass in an instant:
player.advanceClock(199)

# or press a key
player.pressKey(65)

# Move the pointer to the specified co-ordinates. Returns true if the
# action requires a redraw.
if player.movePointer(10,20):
    player.render(False)

# Click the mouse at the current pointer position. True if the action
# requires a redraw.
if player.mouseClick():
    player.render(False)


char = player.getCharacterByTarget('_root')
print "_root has depth: "
print char.depth()

