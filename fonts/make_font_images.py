#!/usr/bin/python

# Thatcher Ulrich <http://tulrich.com> 2004

# This source code has been donated to the Public Domain.  Do whatever
# you want with it.

# Make font test images

# Unfortunately there appears to be some bug in text output which
# causes some glyph descenders to get clipped.  It's too bad because
# the fancy C++ version of this functionality took about 5 times
# longer to write...


import Image, ImageFont, ImageDraw

WIDTH = 400
HEIGHT = 600
image = Image.new("L", (WIDTH, HEIGHT), 255)	# Are pixel values normalized?

font = ImageFont.truetype("Tuffy_Bold.otf", 40)
#font = ImageFont.truetype("/usr/share/fonts/truetype/Verdana.ttf", 40)

draw = ImageDraw.Draw(image)

vals = { 'y': 5 }

def stringout(str):
	"output a centered line of text into our image"
	(width, height) = draw.textsize(str, font=font)
	draw.text(((WIDTH - width)/2, vals['y']), str, font=font, fill=0)
	vals['y'] += height + 5
	
stringout("Tuffy Bold")
stringout("ABCDEFGHIJKLMNOPQRS")
stringout("TUVWXY abcdefghijklmo")
stringout("pqrstuvwxy 0123456789")
# stringout("\`\~!@#$%^&*()_+-=")
stringout("`@#$%^&*()_+-=")
stringout("~")
stringout(";':\"[]\\{}|,./<>?")

# Draw small text.
font = ImageFont.truetype("Tuffy_Bold.otf", 15)
#font = ImageFont.truetype("/usr/share/fonts/truetype/Verdana.ttf", 15)
stringout("Tuffy Bold")
stringout("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
stringout("abcdefghijklmnopqrstuvwxyz")
stringout("0123456789 \`\~!@#$%^&*()_+-=")
stringout("`@#$%^&*()_+-=")
stringout(";':\"[]\\{}|,./<>?")
stringout("Satan, oscillate my metallic sonatas!")

image.show()

# image.save("tuffy_bold_stuff.png")
