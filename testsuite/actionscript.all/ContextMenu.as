// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//


// Test case for ContextMenu ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


// This could be tested properly with any Gnash hosting application
// like gprocessor by checking that the instructions to the context menu
// match what the pp displays.

rcsid="ContextMenu.as";
#include "check.as"

#if OUTPUT_VERSION < 7

  check_equals(typeof(ContextMenu), 'function');
  totals(1);

#else

  check_equals(typeof(ContextMenu), 'function');

  check(ContextMenu.prototype.hasOwnProperty("copy"));  
  check(ContextMenu.prototype.hasOwnProperty("hideBuiltInItems"));  

  check(!ContextMenu.prototype.hasOwnProperty("builtInItems"));  
  check(!ContextMenu.prototype.hasOwnProperty("customItems"));
  check(!ContextMenu.prototype.hasOwnProperty("onSelect"));  

  var cm = new ContextMenu;
  
  check_equals (typeof(cm), 'object');
  check(cm instanceof ContextMenu);

  check(!cm.hasOwnProperty("copy"));  
  check(!cm.hasOwnProperty("hideBuiltInItems"));  

  check_equals(typeof(cm.copy), "function");
  check_equals(typeof(cm.hideBuiltInItems), "function");

  check(cm.hasOwnProperty("builtInItems"));  
  check(cm.hasOwnProperty("customItems"));
  check(cm.hasOwnProperty("onSelect"));  

  check_equals(typeof(cm.builtInItems), "object");  
  check(!cm.builtInItems instanceof Array);
  check_equals(typeof(cm.builtInItems.length), 'undefined');

  check_equals(typeof(cm.customItems), "object");
  check(cm.customItems instanceof Array);
  check_equals(typeof(cm.customItems.length), 'number');

  // There are no custom items by default.
  check_equals(cm.customItems.length, 0);

  check_equals(typeof(cm.onSelect), "undefined");
 
  // Check the built-in items. 
  o = cm.builtInItems;
  s = "";
  for (i in o) {
     check_equals(typeof(i), "string");
     check_equals(typeof(o[i]), "boolean");
     s += i + ",";
  }
  check_equals(s, "save,zoom,quality,play,loop,rewind,forward_back,print,");


  // Check that the hideBuiltInItems method isn't fussy.
  e = {};
  e.f = ContextMenu.prototype.hideBuiltInItems;
  e.f();
  s = "";
  for (i in e.builtInItems) {
      s += i + ":" + e.builtInItems[i] + ",";
  };
  check_equals(s, "save:false,zoom:false,quality:false,play:false,loop:false,rewind:false,forward_back:false,print:false,");

  e.a = "string";
  e.builtInItems.extraProp = "boo";

  // Check the copy method. The original object has the following members:
  // f : function
  // builtInItems : object
  // a : string
  //
  // The builtInItems has an extra property.

  e.copy = ContextMenu.prototype.copy;
  ee = e.copy();
  check(!ee.hasOwnProperty("a"));
  check(!ee.hasOwnProperty("f"));
  check(ee.hasOwnProperty("onSelect"));
  check(ee.hasOwnProperty("builtInItems"));
  check(ee.hasOwnProperty("customItems"));

  check_equals(typeof(ee.builtInItems), "object");
  check_equals(typeof(ee.builtInItems.extraProp), "string");
  check_equals(ee.builtInItems.extraProp, "boo");
  check(ee instanceof ContextMenu);

  // It will copy any expected properties, not only if they are objects, and
  // add those that aren't there. The customItems array is cloned; if the
  // original customItems member isn't an array, the new array is empty.
  f = {};
  f.builtInItems = 6;
  f.copy = ContextMenu.prototype.copy;
  ff = f.copy();
  check(ff.hasOwnProperty("builtInItems"));
  check(ff.hasOwnProperty("customItems"));
  check(ff.hasOwnProperty("onSelect"));
  check_equals(ff.builtInItems, 6);
  check_equals(typeof(ff.customItems), "object");
  check(ff.customItems instanceof Array);

  f.customItems = 88;
  ff = f.copy();
  check_equals(typeof(ff.customItems), "object");

  f.customItems = {};
  f.customItems.p = "hello";
  ff = f.copy();
  check_equals(ff.customItems.length, 0);
  check_equals(ff.customItems.p, undefined);

  f.customItems = new Array;
  f.customItems.push("hello");
  ff = f.copy();
  check_equals(ff.customItems.length, 1);
  check_equals(ff.customItems[0], undefined);

  h = function() {};

  cmi = new ContextMenuItem("hi", h);
  f.customItems.push(cmi);
  ff = f.copy();
  check_equals(f.customItems.length, 2);
  check_equals(f.customItems[1].caption, "hi");
  check_equals(ff.customItems.length, 2);
  check_equals(ff.customItems[1].caption, "hi");

  c = {};
  c.onSelect = h;
  c.caption = "moo";
  f.customItems.push(c);
  check_equals(f.customItems.length, 3);
  check_equals(f.customItems[2].caption, "moo");
  ff = f.copy();
  check_equals(ff.customItems.length, 3);
  check_equals(ff.customItems[2].caption, undefined);

  // Properties are only copied properly if instanceOf ContextMenuItem;
  // otherwise they are undefined.
  c.__proto__ = ContextMenuItem.prototype;
  check_equals(ff.customItems[2].caption, undefined);
  ff = f.copy();
  check_equals(ff.customItems.length, 3);
  check_equals(ff.customItems[2].caption, "moo");

  // If builtInItems is undefined, it is copied as undefined. If it is an
  // object, the new ContextMenu refers to the *same* object.
  f.builtInItems = undefined;
  ff = f.copy();
  check(ff.hasOwnProperty("builtInItems"));
  check_equals(typeof(ff.builtInItems), "undefined")
  
  delete f.builtInItems;
  ff = f.copy();
  check(ff.hasOwnProperty("builtInItems"));
  check_equals(typeof(ff.builtInItems), "undefined")

  o = {};
  o.g = 5;
  f.builtInItems = o;
  ff = f.copy();
  check_equals(ff.builtInItems.g, 5);
  o.g = "string";
  check_equals(ff.builtInItems.g, "string");

  // Test ContextMenuItem
  
  check_equals(typeof(ContextMenuItem), "function");

  check(ContextMenuItem.prototype.hasOwnProperty("copy"));

  check(!ContextMenuItem.prototype.hasOwnProperty("caption"));
  check(!ContextMenuItem.prototype.hasOwnProperty("enabled"));
  check(!ContextMenuItem.prototype.hasOwnProperty("separatorBefore"));
  check(!ContextMenuItem.prototype.hasOwnProperty("visible"));

  it = new ContextMenuItem();
  check_equals(typeof(it), "object");
  check(it instanceof ContextMenuItem);

  check(!it.hasOwnProperty("copy"));
  check(it.hasOwnProperty("caption"));
  check(it.hasOwnProperty("enabled"));
  check(it.hasOwnProperty("separatorBefore"));
  check(it.hasOwnProperty("visible"));
  check(it.hasOwnProperty("onSelect"));

  check_equals(typeof(it.caption), "undefined");
  check_equals(it.caption, undefined);
  check_equals(typeof(it.onSelect), "undefined");
  check_equals(it.onSelect, undefined);
  check_equals(typeof(it.enabled), "boolean");
  check_equals(it.enabled, true);
  check_equals(typeof(it.separatorBefore), "boolean");
  check_equals(it.separatorBefore, false);
  check_equals(typeof(it.visible), "boolean");
  check_equals(it.visible, true);

  f = function () { trace("f"); return "f"; };
  g = function () { trace("g"); return "g"; };
  

  it = new ContextMenuItem("name1", f);
  check_equals(typeof(it.caption), "string");
  check_equals(it.caption, "name1");
  check_equals(typeof(it.onSelect), "function");
  check_equals(it.onSelect(), "f");
  check_equals(typeof(it.enabled), "boolean");
  check_equals(it.enabled, true);
  check_equals(typeof(it.separatorBefore), "boolean");
  check_equals(it.separatorBefore, false);
  check_equals(typeof(it.visible), "boolean");
  check_equals(it.visible, true);

  // Add a test object to the ContextMenu
  cm.customItems.push(it);
  check_equals(cm.customItems.length, 1);
  _root.menu = cm;

  // An object is added to the menu if:
  // (a) it has both a caption and an onSelect member that is a function.
  // (b) it does not have a visible property that evaluates to false.
  //
  // The item is enabled unless the enabled property evaluates to false.
  o = {};
  o.caption = "fake item";
  o.onSelect = f;
  o.enabled = true;
  o.visible = true;
  cm.customItems.push(o);

  o1 = {};
  o1.caption = "fake item 2";
  cm.customItems.push(o1);
  o1.onSelect = f;

  // This isn't added because onSelect isn't a function.
  o2 = it.copy();
  check_equals(o2.caption, "name1");
  check_equals(o2.onSelect(), "f");
  o2.onSelect = 6;
  o2.caption = "name2";
  cm.customItems.push(o2);

  o3 = o2.copy();
  o3.onSelect = g;
  o3.caption = "name3";
  cm.customItems.push(o3);

  // If two objects have the same caption, only the first is added to the menu.
  o4 = it.copy();
  check_equals(o4.caption, "name1");
  o4.onSelect = g;
  cm.customItems.push(o4);

  //----------------------------------------------
  // Test onSelect
  //----------------------------------------------
  
  function callback() { }
  var contextMenuObj2 = new ContextMenu(callback);
  check_equals(typeof(contextMenuObj2.onSelect), 'function');
  check_equals(contextMenuObj2.onSelect, callback);
  function callback2() { }
  contextMenuObj2.onSelect = callback2;
  check_equals(typeof(contextMenuObj2.onSelect), 'function');
  check_equals(contextMenuObj2.onSelect, callback2);
  contextMenuObj2.onSelect = null;
  check_equals(typeof(contextMenuObj2.onSelect), 'null');
  contextMenuObj2.onSelect = undefined;
  check_equals(typeof(contextMenuObj2.onSelect), 'undefined');
  contextMenuObj2.onSelect = 4;
  check_equals(typeof(contextMenuObj2.onSelect), 'number');
  
  totals(123);

#endif
