// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#include "check.as"

#if OUTPUT_VERSION < 6

 totals(0);

#else

 _root.createTextField("textfield1", 1, 10, 10, 100, 100);
 tf = _root.textfield1;

 // Text is not parsed as HTML unless the html property is true, regardless
 // of whether you set htmlText or Text.
 tf.htmlText = "<b>bold</b>";
 check_equals(tf.text, "<b>bold</b>");
 tf.text = "<i>italic</i>";
 check_equals(tf.text, "<i>italic</i>");

 // Changing the property afterwards makes no difference.
 tf.html = true;
 check_equals(tf.text, "<i>italic</i>");
 tf.text = "<b>bold</b>";

 // Only setting the htmlText property causes tag parsing.
 check_equals(tf.text, "<b>bold</b>");
 tf.htmlText = "<i>italic</i>";
 xcheck_equals(tf.text, "italic");
 
 tf.html = false;
 
 // The htmlText value is generated on-the-fly from the text's properties,
 // but only when the html property is true.
 tf.htmlText = "<font>font</font>";
 check_equals(tf.text, "<font>font</font>");

 // Here there is no html output.
 check_equals(tf.htmlText, "<font>font</font>");

 tf.html = true;

 // Here there is html output, even though the text has not changed.
 xcheck_equals(tf.htmlText, '<P ALIGN="LEFT"><FONT FACE="Times" SIZE="12" COLOR="#000000" LETTERSPACING="0" KERNING="0">&lt;font&gt;font&lt;/font&gt;</FONT></P>');

 // Check font color attribute.
 // The html property is still true now.

 tf.htmlText = '<font color="#00FF00">green</font>';
 xcheck_equals(tf.text, "green");
 // The TextField textColor remains black.
 check_equals(tf.textColor, 0);
 // The characters are green.
 format = tf.getTextFormat(1, 4);
 xcheck_equals(format.color, 0x00ff00);
 
 // This fails (no quotes)
 tf.htmlText = '<font color=#00FF00>green2</font>';
 xcheck_equals(tf.text, "");
 // The TextField textColor remains black.
 check_equals(tf.textColor, 0);

 // Lower case is fine.
 tf.htmlText = '<font color="#0000ff">blue</font>';
 xcheck_equals(tf.text, "blue");
 // The TextField textColor remains black.
 check_equals(tf.textColor, 0);
 format = tf.getTextFormat(1, 4);
 xcheck_equals(format.color, 0x0000ff);

// WARNING!! The disabled code crashes Gnash at the moment.
// When that's fixed, this code can be reenabled to get the totals() pass,
// and the failures in this section (only!) can be expected.

 // A color string that is this short doesn't change the color.
 tf.htmlText = '<font color="#ff">too short</font>';
 xcheck_equals(tf.text, "too short");
 format = tf.getTextFormat(1, 4);
 xcheck_equals(format.color, 0x0000ff);

 // When it's three characters it does change the color.
 tf.htmlText = '<font color="#ff0">a bit short</font>';
 xcheck_equals(tf.text, "a bit short");
 format = tf.getTextFormat(1, 4);
 xcheck_equals(format.color, 0x000ff0);
 
 // Without a hash it sets the color to black.
 tf.htmlText = '<font color="ff00ff">no hash</font>';
 xcheck_equals(tf.text, "no hash");
 format = tf.getTextFormat(1, 4);
 check_equals(format.color, 0);

 tf.htmlText = '<font color="hi">no hash 2</font>';
 xcheck_equals(tf.text, "no hash 2");
 format = tf.getTextFormat(1, 4);
 check_equals(format.color, 0);

 tf.htmlText = '<font color="">empty</font>';
 xcheck_equals(tf.text, "empty");
 format = tf.getTextFormat(1, 4);
 check_equals(format.color, 0);

 // Extra long strings are truncated, but the end counts, not the beginning.
 tf.htmlText = '<font color="#ff00ffffee">long</font>';
 xcheck_equals(tf.text, "long");
 format = tf.getTextFormat(1, 4);
 xcheck_equals(format.color, 0xffffee);

 // Strings containing non-hex characters ignore those characters.
 tf.htmlText = '<font color="#ff00gp">corrupt</font>';
 xcheck_equals(tf.text, "corrupt");
 format = tf.getTextFormat(1, 4);
 xcheck_equals(format.color, 0x00ff00);

 // Check empty face value (bug #32508)
 tf.htmlText = '<font face="">#32508</font>';
 xcheck_equals(tf.text, "#32508");

 totals(31);

#endif
