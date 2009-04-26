import gtk
import gnash

gtk.gdk.threads_init

w = gtk.Window()

v = gnash.View()
v.props.uri = '/home/tomeu/Desktop/EatBoom.swf'
w.add(v)
v.show()

w.show()

gtk.main()
