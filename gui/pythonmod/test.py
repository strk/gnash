import sys

import gtk
import gnash

gtk.gdk.threads_init

w = gtk.Window()
w.connect('delete-event', lambda w, e: gtk.main_quit())

v = gnash.View()
v.props.uri = sys.argv[1]
w.add(v)
v.show()

w.show()

gtk.main()
