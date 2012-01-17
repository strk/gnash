// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef __GTKEXT_PLUGIN_H__
#define __GTKEXT_PLUGIN_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <memory> // for auto_ptr
#include "as_object.h"

#include <cstdio>
#include <string>
#include <map>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

namespace gnash
{

class GtkExt : public as_object {
public:
    GtkExt();
    ~GtkExt();

    typedef void (*gtk_callback_t)(GtkWidget *widget, gpointer data);

    // Gtk2 API
    void window_new();
    void signal_connect();
    void container_set_border_width(int width);
    GtkWidget *button_new_with_label(const char *label);
    void signal_connect_swapped();
    void container_add();
    void widget_show();
    void main();

    // internal methods
    GtkWidget *getWindow() { return _window; };
    void setWindow(GtkWidget *x) { _window = x; };
//     void addCallback(std::string &event, as_value *func) { _callbacks[event] = func; };
//     as_value *getCallback(std::string &event) { return _callbacks[event]; };
//     std::map<std::string, as_value *>	_callbacks;
    
private:
    GtkWidget			*_window;
};

extern "C" {
    void gtkext_class_init(as_object &obj);  
    /// Return an  instance
}

std::auto_ptr<as_object> init_gtkext_instance();

} // end of gnash namespace

// __GTKEXT_PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
