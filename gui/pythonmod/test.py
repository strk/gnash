import gtk
import gnash

w = gtk.Window()

v = gnash.View()
w.add(v)
v.show()

v.load_movie('http://shell.sugarlabs.org/~tomeu/EatBoom.swf')
v.start()

w.show()

gtk.main()
