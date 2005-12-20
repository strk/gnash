// 
//   Copyright (C) 2005 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "log.h"
#include "textformat.h"

namespace gnash {  

  text_format::text_format() :
      _underline(false),
      _bold(false),
      _italic(false),
      _bullet(false),
      _block_indent(-1),
      _color(0),
      _indent(-1),
      _leading(-1),
      _left_margin(-1),
      _right_margin(-1),
      _point_size(-1),
      _tab_stops(-1),
      _target(-1)
{
  //log_msg("%s: \n", __FUNCTION__);
}

text_format::~text_format()
{
  // don't need to clean up anything
}

// Copy one text_format object to another.
text_format *
text_format::operator = (text_format &format)
{
  log_msg("%s: \n", __FUNCTION__);

  _underline = format._underline;
  _bold = format._bold;
  _italic = format._italic;
  _bullet = format._bullet;
  
  _align = format._align;
  _block_indent = format._block_indent;
  _color = format._color;
  _font = format._font;
  _indent = format._indent;
  _leading = format._leading;
  _left_margin = format._left_margin;
  _right_margin = format._right_margin;
  _point_size = format._point_size;
  _tab_stops = format._tab_stops;
  _target = format._target;
  _url = format._url;
  
  return this;
}

// In a paragraph, change the format of a range of characters.
void
text_format::setTextFormat (text_format &format)
{
  //log_msg("%s: \n", __FUNCTION__);
}

void
text_format::setTextFormat (int index, text_format &format)
{
  //log_msg("%s: \n", __FUNCTION__);
}

void
text_format::setTextFormat (int start, int end, text_format &format)
{
  //log_msg("%s: \n", __FUNCTION__);
}

#if 0
text_format &
text_format::getTextFormat ()
{
  log_msg("%s: \n", __FUNCTION__);
}

text_format &
text_format::getTextFormat (int index)
{
  log_msg("%s: \n", __FUNCTION__);
}

text_format &
text_format::getTextFormat (int start, int end)
{
  log_msg("%s: \n", __FUNCTION__);
}
#endif

void textformat_new(const fn_call& fn)
{
  //log_msg("%s: args=%d\n", __FUNCTION__, nargs);

  textformat_as_object*	text_obj = new textformat_as_object;
  log_msg("\tCreated New TextFormat object at %p. Not fully implmented yet!\n", text_obj);
  
  // tulrich: this looks like it's inserting a method into our
  // caller's env.  setTextFormat is a method on TextField.  So here
  // we're hoping our caller is a text field... scary.
  //
  // TODO we should handle setTextFormat as a method on TextField,
  // instead of doing this.
  fn.env->set_variable("setTextFormat", &textformat_setformat, 0);
  
  fn.result->set_as_object_interface(text_obj);
}


void textformat_setformat(const fn_call& fn)
{
  as_value	method;
  //log_msg("%s: args=%d at %p\n", __FUNCTION__, nargs, this_ptr);
#if 0
  // FIXME: these are only commented out to eliminate compilation warnings.
  textformat_as_object*	ptr = (textformat_as_object*) fn.this_ptr;	// tulrich: TODO fix this unsafe cast; see textformat_new().
  assert(ptr);
  double start = fn.arg(0).to_number();
  double end = fn.arg(1).to_number();
#endif
  textformat_as_object *obj = (textformat_as_object*) fn.arg(2).to_object();	// tulrich: TODO fix this unsafe cast.  (need cast_to_textformat())
  assert(obj);

  //log_msg("Change from %f for %f characters for object at %p\n", start, end, obj);

  // Check for the flags that could be set
  if (obj->get_member("underline", &method)) {
    //log_msg("Underline exists and is set to %d\n", method.to_bool());
    obj->obj.underlinedSet(method.to_bool());
  }
  
  if (obj->get_member("italic", &method)) {
    //log_msg("Italic exists and is set to %d\n", method.to_bool());
    obj->obj.italicedSet(method.to_bool());
  }
  
  if (obj->get_member("bold", &method)) {
    //log_msg("Bold exists and is set to %d\n", method.to_bool());
    obj->obj.boldSet(method.to_bool());
  }
  
  if (obj->get_member("bullet", &method)) {
    //log_msg("Bullet exists and is set to %d\n", method.to_bool());
    obj->obj.bulletSet(method.to_bool());
  }

  if (obj->get_member("color", &method)) {
    //log_msg("Color exists and is set to %f\n", method.to_number());
    obj->obj.colorSet((uint32)method.to_number());
  }

  if (obj->get_member("indent", &method)) {
    //log_msg("Indent exists and is set to %f\n", method.to_number());
    obj->obj.indentSet(float(method.to_number()));
  }

  if (obj->get_member("align", &method)) {
    //log_msg("Align exists and is set to %s\n", method.to_string());
    obj->obj.alignSet(method.to_tu_string());
  }

  if (obj->get_member("blockIndent", &method)) {
    //log_msg("BlockIndent exists and is set to %f\n", method.to_number());
    obj->obj.blockIndentSet(float(method.to_number()));
  }
  
  if (obj->get_member("leading", &method)) {
    //log_msg("Leading exists and is set to %f\n", method.to_number());
    obj->obj.leadingSet(float(method.to_number()));
  }
  
  if (obj->get_member("leftMargin", &method)) {
    //log_msg("LeftMargin exists and is set to %f\n", method.to_number());
    obj->obj.leftMarginSet(float(method.to_number()));
  }
  
  if (obj->get_member("RightMargin", &method)) {
    //log_msg("RightMargin exists and is set to %f\n", method.to_number());
    obj->obj.rightMarginSet(float(method.to_number()));
  }
  
  if (obj->get_member("size", &method)) {
    //log_msg("Size exists and is set to %f\n", method.to_number());
    obj->obj.sizeSet(float(method.to_number()));
  }
  
  //ptr->obj.setTextFormat(start, end, obj->obj);
  //result->set_bool(true);
}
#if 0
  void
  textformat_getformat(gnash::as_value* result, gnash::as_object_interface* this_ptr, gnash::as_environment* env, int nargs, int first_arg)
{
  log_msg("%s: args=%d unfinished implementation\n", __FUNCTION__, nargs);
  textformat_as_object*	ptr = (textformat_as_object*)this_ptr;
  assert(ptr);
  double start = env->bottom(first_arg).to_number();
  double end = env->bottom(first_arg-1).to_number();
  textformat_as_object *obj = (textformat_as_object *)env->bottom(first_arg-2).to_object();
  assert(obj);
    
  ptr->obj = ptr->obj.getTextFormat();
  result->set_bool(true);
}
#endif

} // end of gnash namespace
