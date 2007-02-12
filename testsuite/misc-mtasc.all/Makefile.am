
# these need some autoconf work...
CLASSPATH=/extra/mtasc/std
MTASC = mtasc

MTASC_FLAGS= -cp $(MTASC_CLASSPATH)

hello.swf: hello.as
	$(MTASC) $(MTASC_FLAGS) -swf $@ -main -header 800:600:20 $<

CLEANFILES = hello.swf
