POFILES = AUTO


CLEANFILES = \
	*.gmo \
	gnash.pot \
	POTFILES \
	update-po

translations = es fr de it cs

pofiles := $(patsubst %,%.po,$(translations))
gmofiles := $(patsubst %,%.gmo,$(translations))

dirs =  $(top_srcdir)/libbase \
	$(top_srcdir)/backend \
	$(top_srcdir)/server \
	$(top_srcdir)/server/vm \
	$(top_srcdir)/server/swf \
	$(top_srcdir)/server/parser \
	$(top_srcdir)/server/asobjs \
	$(top_srcdir)/libgeometry \
	$(top_srcdir)/utilities \
	$(top_srcdir)/gui 
srcfiles := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))

EXTRA_DIST = $(gmofiles)

all: $(gmofiles)

if HAS_GETTEXT
POTFILES: $(srcfiles)
	@echo "Building POTFILES..."
	@rm -f POTFILES
	@for i in $(srcfiles) ; do \
	  echo "$$i" >> POTFILES;\
	done

gnash.pot: POTFILES
	$(XGETTEXT) --language=C++ --files-from=POTFILES -o gnash.pot \
	  --default-domain=gnash --force \
	  --flag=_:1:pass-c-format --keyword=_ \
	  --copyright-holder='Free Software Foundation, Inc.'

update-po: gnash.pot
	@for i in $(pofiles); do \
	  echo "Merging $$i..." ; \
	  $(MSGMERGE) --update $(srcdir)/$$i gnash.pot; \
	done
	touch update-po

$(gmofiles): #update-po

SUFFIXES = .po .gmo
.po.gmo:
	msgfmt --statistics --directory=$(srcdir) -o $*.gmo $(@:%.gmo=%.po) 
else
$(gmofiles): force
	@for i in $(gmofiles); do \
	  ln -fs $(srcdir)/$$i.in $$i; \
	done
endif

force:

# install-hook: rename the es.gmo files to gnash.mo at installtion time
install-data-hook: $(gmofiles)
	@for i in $(translations); do \
	  $(mkinstalldirs) "$(DESTDIR)$(datadir)/locale/$$i"; \
	  if test -f $$i.gmo; then \
	    $(INSTALL_DATA) $$i.gmo $(DESTDIR)$(datadir)/locale/$$i/gnash.mo; \
	  fi; \
	done
