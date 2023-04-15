OBJS=cal-home-widget.o cal-home-load-and-store.o cal-home-calendar-util.o cal-home-settings-dlg.o
BUILDDIR=../build
LIB=$(BUILDDIR)/cal-home-widget.so
PKG_FLAGS_C=$(shell pkg-config hildon-1 libhildondesktop-1 calendar-backend libtime gconf-2.0 --cflags) -fPIC
PKG_FLAGS_LIB=$(shell pkg-config hildon-1 libhildondesktop-1 calendar-backend libtime gconf-2.0 --libs)
CCFLAGS=-shared
CC=g++

all:$(LIB)


$(LIB):$(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) $(PKG_FLAGS_LIB) -o $(LIB)

.c.o:
	$(CC) $(PKG_FLAGS_C) -c $< -o $@

.PHONY: clean all

clean:
	rm -f $(OBJS)
