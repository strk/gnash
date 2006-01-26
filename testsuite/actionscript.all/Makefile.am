LDFLAGS += $MING_LDFLAGS 

if ENABLE_MING

# moviecliploader_test_ming.swf definebitsjpeg2.swf 
GENERATED = xtrace.swf

swf: $(GENERATED)

xtrace.swf: xtrace.as 
	$(MAKESWF) -o $@ $<

moviecliploader_test: moviecliploader_test.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $<

moviecliploader_test_ming.swf: moviecliploader_test
	./moviecliploader_test

definebitsjpeg2: definebitsjpeg2.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $<

definebitsjpeg2.swf: definebitsjpeg2
	./definebitsjpeg2

clean:
	rm -f $(GENERATED)

endif # ENABLE_MING
