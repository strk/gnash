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

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


#include "dejagnu.as"

file = new FileIO();

var gtk = new GtkExt();

function hello (Gtk2, data)
{
    file.puts("hello");
    file.puts(data);
}

function delete (Gtk2, event, data)
{
    file.puts("delete");
    trace(data);
}

function destroy (Gtk2, data)
{
    file.puts("destroy");
    trace(data);
    gtk.main_quit();
}

// create a new window
var window = gtk.window_new();

// When the window is given the "delete_event" signal (this is given
// by the window manager, usually by the "close" option, or on the
// titlebar), we ask it to call the delete_event () function
// as defined above. The data passed to the callback
// function is NULL and is ignored in the callback function.
gtk.signal_connect(window, "delete_event", delete, 0);

// Here we connect the "destroy" event to a signal handler.  
// This event occurs when we call gtk_widget_destroy() on the window,
// or if we return FALSE in the "delete_event" callback.
gtk.signal_connect(window, "destroy_event", destroy, 0);

// Sets the border width of the window.
gtk.container_set_border_width (window, 5);

// Creates a new button with the label "Hello World".
var button = gtk.button_new_with_label ("Hello World");

// When the button receives the "clicked" signal, it will call the
// function hello() passing it NULL as its argument.  The hello()
// function is defined above.
gtk.signal_connect (button, "clicked", hello, 222);

// This will cause the window to be destroyed by calling
// gtk_widget_destroy(window) when "clicked".  Again, the destroy
// signal could come from here, or the window manager.
gtk.signal_connect_swapped (button, "clicked", gtk_widget_destroy, window);

// This packs the button into the window (a gtk container).
gtk.container_add (window, button);

// The final step is to display this newly created widget.
gtk.widget_show (button);

// and the window
gtk.widget_show (window);

// All GTK applications must have a gtk_main(). Control ends here
// and waits for an event to occur (like a key press or mouse event).

gtk.main ();

// totals();
