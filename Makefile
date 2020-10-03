SUBDIRS = src
BUILDDIR = build
DATA_FILE=cal-home-widget.desktop
LIB=cal-home-widget.so
HILDON_WIDGET_LIB_DIR=$(shell pkg-config libhildondesktop-1 --variable=hildondesktoplibdir)
HILDON_WIDGET_DATA_DIR=$(shell pkg-config libhildondesktop-1 --variable=hildonhomedesktopentrydir)


all:    create_builddir subdirs


MAKE_BUILD_DIR:
	mkdir -p $(BUILDDIR)
	

install: all
	install -d $(DESTDIR)/$(HILDON_WIDGET_LIB_DIR)
	install $(BUILDDIR)/$(LIB) $(DESTDIR)/$(HILDON_WIDGET_LIB_DIR)
	install -d $(DESTDIR)/$(HILDON_WIDGET_DATA_DIR)
	install data/$(DATA_FILE) $(DESTDIR)/$(HILDON_WIDGET_DATA_DIR)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

create_builddir:
	mkdir -p build

.PHONY: all clean install $(SUBDIRS)

clean:
	rm -rf build
	for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean); done
