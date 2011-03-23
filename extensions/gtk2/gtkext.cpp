// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <map>
#include <iostream>
#include <string>
#include <cstdio>
#include <boost/algorithm/string/case_conv.hpp>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include "VM.h"
#include "fn_call.h"
#include "log.h"
#include "fn_call.h"
#include "as_object.h"
#include "as_function.h"
#include "debugger.h"
#include "gtkext.h"
#include "as_function.h"

using namespace std;

namespace gnash
{

#define dbglogfile cerr		// FIXME: use log_*()

// prototypes for the callbacks required by Gnash
as_value gtkext_window_new(const fn_call& fn);
as_value gtkext_signal_connect(const fn_call& fn);
as_value gtkext_container_set_border_width(const fn_call& fn);
as_value gtkext_button_new_with_label(const fn_call& fn);
as_value gtkext_signal_connect_swapped(const fn_call& fn);
as_value gtkext_container_add(const fn_call& fn);
as_value gtkext_widget_show(const fn_call& fn);
as_value gtkext_main(const fn_call& fn);

// Sigh... We can't store the callbacks for the events in the GtkExt
// class object because that data is inaccessible to a C symbol based
// callback. 
static map<string, as_value> callbacks;

void dump_callbacks(map<string, as_value> &calls)
{
//    GNASH_REPORT_FUNCTION;
    map<string, as_value>::const_iterator it;
    dbglogfile << "# of callbacks is: " << calls.size() << endl;
    for (it=calls.begin(); it != calls.end(); it++) {
	string name = (*it).first;
	as_value as = (*it).second;
	dbglogfile << "Event \"" << name.c_str() << "\" has AS function" << as.to_string() << endl;
    }
	
}

static void
generic_callback(GtkWidget * /*widget*/, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
//    g_print ("Hello World - %d\n", *(int *)data;
    const char *event = (const char *)data;

    as_value handler = callbacks[event];
    as_function *as_func = handler.to_function();

//	start the debugger when this callback is activated.
// 	debugger.enabled(true);
// 	debugger.console();

    // FIXME: Delete events don't seem to pass in data in a form we
    // can access it. So for now we just hack in a quit, since we know
    // we're done, we hope...
    if (*event == 0) {
	gtk_main_quit();
	return;
    } else {
	cerr << "event is: \"" << event << "\"" << endl;
    }
    
    as_value val;
    as_environment env(getVM(*as_func));

    fn_call::Args args;
    args += handler, event, handler;

    as_object obj = val.to_object(*getGlobal(fn));

    // Call the AS function defined in the source file using this extension
    (*as_func)(fn_call(&obj, &env, args));
}

static void
attachInterface(as_object *obj)
{
//    GNASH_REPORT_FUNCTION;

    obj->init_member("window_new", gl->createFunction(gtkext_window_new));
    obj->init_member("signal_connect", gl->createFunction(gtkext_signal_connect));
    obj->init_member("container_set_border_width", gl->createFunction(gtkext_container_set_border_width));
    obj->init_member("button_new_with_label", gl->createFunction(gtkext_button_new_with_label));
    obj->init_member("signal_connect_swapped", gl->createFunction(gtkext_signal_connect_swapped));
    obj->init_member("container_add", gl->createFunction(gtkext_container_add));
    obj->init_member("widget_show", gl->createFunction(gtkext_widget_show));
    obj->init_member("main", gl->createFunction(gtkext_main));
}

static as_object*
getInterface()
{
//    GNASH_REPORT_FUNCTION;
    static boost::intrusive_ptr<as_object> o;
    if (o == NULL) {
	o = new as_object();
    }
    return o.get();
}

static as_value
gtkext_ctor(const fn_call& /*fn*/)
{
//    GNASH_REPORT_FUNCTION;
    GtkExt *obj = new GtkExt();

    attachInterface(obj);
    return as_value(obj); // will keep alive
}


GtkExt::GtkExt()
{
//    GNASH_REPORT_FUNCTION;
    int argc = 0;
    char **argv;
    gtk_init (&argc, &argv);
}

GtkExt::~GtkExt()
{
//    GNASH_REPORT_FUNCTION;
}

void
GtkExt::window_new()
{
    GNASH_REPORT_FUNCTION;
//    std::auto_ptr<Gui> ggg = player.getGuiHandle();
//    player.getGuiHandle();
//    gui.getWindow();
    _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
}

// void
// GtkExt::signal_connect()
// {
//     GNASH_REPORT_FUNCTION;
// }

// void gtk_container_set_border_width (GtkContainer *container, guint border_width);
void
GtkExt::container_set_border_width(int width)
{
//    GNASH_REPORT_FUNCTION;
    if (_window) {
	gtk_container_set_border_width (GTK_CONTAINER (_window), width);
    }
}

// GtkWidget *gtk_button_new_with_label (const gchar *label);
GtkWidget *
GtkExt::button_new_with_label(const char *label)
{
//    GNASH_REPORT_FUNCTION;
    _window = gtk_button_new_with_label (label);
    return _window;
}

void
GtkExt::main()
{
//    GNASH_REPORT_FUNCTION;
}

// this callback takes no arguments
as_value gtkext_window_new(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<GtkExt> ptr = ensureType<GtkExt>(fn.this_ptr);

    GtkExt *obj = new GtkExt;
    obj->window_new();
    return as_value(obj);
}

// this callback takes 4 arguments, we only need two of them
// g_signal_connect (instance, detailed_signal, c_handler, data)
as_value gtkext_signal_connect(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<GtkExt> ptr = ensureType<GtkExt>(fn.this_ptr);

    if (fn.nargs > 0) {
	GtkExt *window = dynamic_cast<GtkExt *>(fn.arg(0).to_object(*getGlobal(fn)).get());
	string name = fn.arg(1).to_string();
	as_value func = fn.arg(2).to_function();
	//int data = fn.arg(3).to_int();

	dbglogfile << "Adding callback " << func.to_string()
		   << " for event \"" << name << "\"" << endl;
 	callbacks[name] = func;
 	g_signal_connect (G_OBJECT (window->getWindow()), name.c_str(),
			  G_CALLBACK (generic_callback), (void *)name.c_str());
    }
    return as_value();
}

// this callback takes 2 arguments
// void gtk_container_set_border_width (GtkContainer *container, guint border_width);
as_value gtkext_container_set_border_width(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<GtkExt> ptr = ensureType<GtkExt>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	GtkExt *window = dynamic_cast<GtkExt *>(fn.arg(0).to_object(*getGlobal(fn)).get());
	int width = fn.arg(1).to_int();
	window->container_set_border_width(width);
	dbglogfile << "set container border width to " << width << " !" << endl;
    }
    return as_value();
}

// Creates a new button with the label "Hello World".
// GtkWidget *gtk_button_new_with_label (const gchar *label);
as_value gtkext_button_new_with_label(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<GtkExt> ptr = ensureType<GtkExt>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string label = fn.arg(0).to_string();
	GtkExt *obj = new GtkExt;
	obj->button_new_with_label(label.c_str());
	return as_value(obj);
    }
    return as_value();
}

// g_signal_connect_swapped(instance, detailed_signal, c_handler, data)
//
// Connects a GCallback function to a signal for a particular object.
//
// The instance on which the signal is emitted and data will be swapped when calling the handler.
// instance : 	the instance to connect to.
// detailed_signal : 	a string of the form "signal-name::detail".
// c_handler : 	the GCallback to connect.
// data : 	data to pass to c_handler calls.
// Returns : 	the handler id
as_value gtkext_signal_connect_swapped(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<GtkExt> ptr = ensureType<GtkExt>(fn.this_ptr);

    if (fn.nargs > 0) {
	GtkExt *parent = dynamic_cast<GtkExt *>(fn.arg(0).to_object(*getGlobal(fn)).get());
	string name = (fn.arg(1).to_string());
	GtkExt *child = dynamic_cast<GtkExt *>(fn.arg(3).to_object(*getGlobal(fn)).get());
// currently unused
//	as_value *callback = dynamic_cast<as_value *>(fn.arg(2).to_object(*getGlobal(fn)));

	// FIXME: This seems to cause an Gobject warning
	g_signal_connect_swapped (G_OBJECT (child->getWindow()), name.c_str(),
				  G_CALLBACK (gtk_widget_destroy),
				  G_OBJECT (parent->getWindow()));
    }
    return as_value();
}

// this takes two arguments
as_value gtkext_container_add(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<GtkExt> ptr = ensureType<GtkExt>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	GtkExt *parent = dynamic_cast<GtkExt *>(fn.arg(0).to_object(*getGlobal(fn)).get());
	GtkExt *child = dynamic_cast<GtkExt *>(fn.arg(1).to_object(*getGlobal(fn)).get());
	gtk_container_add (GTK_CONTAINER (parent->getWindow()), child->getWindow());
	return as_value(true);
    }
    return as_value(false);
}

as_value gtkext_widget_show(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<GtkExt> ptr = ensureType<GtkExt>(fn.this_ptr);
     
    if (fn.nargs > 0) {
	GtkExt *window = dynamic_cast<GtkExt *>(fn.arg(0).to_object(*getGlobal(fn)).get());
	gtk_widget_show(window->getWindow());
    }
    return as_value();
}

// gtk_main takes no arguments.
as_value gtkext_main(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<GtkExt> ptr = ensureType<GtkExt>(fn.this_ptr);

    gtk_main();
    return as_value();
}

std::auto_ptr<as_object>
init_gtkext_instance()
{
    return std::auto_ptr<as_object>(new GtkExt());
}

extern "C" {
    void
    gtkext_class_init(as_object &obj)
    {
//	GNASH_REPORT_FUNCTION;
	// This is going to be the global "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;
	if (cl == NULL) {
        as_object* proto = getInterface();
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&gtkext_ctor, proto);
 	    // replicate all interface to class, to be able to access
 	    // all methods as static functions
 	    attachInterface(cl.get());
	}
	
	obj.init_member("GtkExt", cl.get());
    }
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
