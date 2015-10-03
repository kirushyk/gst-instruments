DEPS = gstreamer-1.0
CFLAGS = $(shell pkg-config --cflags $(DEPS)) -I../
LIBS = $(shell pkg-config --libs $(DEPS))

all: gst-top-1.0 gst-report-1.0

gst-top-1.0: gst-top.c
	gcc -o gst-top-1.0 $(LIBS) $(CFLAGS) gst-top.c

gst-report-1.0: gst-report.c gst-trace-parser.c formatters.c
	gcc -o gst-report-1.0 $(LIBS) $(CFLAGS) gst-trace-parser.c gst-report.c formatters.c

PREFIX = /usr/local/

install: all
	cp gst-top-1.0 $(PREFIX)/bin/
	cp gst-report-1.0 $(PREFIX)/bin/

uninstall:
	rm -f $(PREFIX)/bin/gst-top-1.0
	rm -f $(PREFIX)/bin/gst-report-1.0

clean:
	rm -f gst-top-1.0
	rm -f gst-report-1.0

