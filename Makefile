ifeq ($(strip $(DESTDIR)),)
DESTDIR = /usr
endif

CFLAGS += -Wall

SRC = net_speeder.c
TARGET = net_speeder

all:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -lpcap -lnet -o $(TARGET)

clean:
	test ! -f $(TARGET) || rm $(TARGET)

install:
	install -m 0755 -p -D net_speeder $(DESTDIR)/bin/net_speeder

uninstall:
	test ! -f $(DESTDIR)/bin/net_speeder || rm $(DESTDIR)/bin/net_speeder