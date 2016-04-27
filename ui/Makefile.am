bin_PROGRAMS = gst-instruments-@GST_API_VERSION@

gst_instruments_@GST_API_VERSION@_SOURCES = \
  main.vala \
  mainmenu.vala \
  timeline.vala \
  mainwindow.vala
gst_instruments_@GST_API_VERSION@_VALAFLAGS = --pkg gtk+-3.0 --pkg posix -X -g -X -lm -X -I./
gst_instruments_@GST_API_VERSION@_CFLAGS = $(GTK_CFLAGS)
gst_instruments_@GST_API_VERSION@_LDFLAGS = $(GTK_LIBS)
