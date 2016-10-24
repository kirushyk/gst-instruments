# todo: use AG_GST_SET_PLUGIN_DIR instead
plugindir="$(libdir)/gstreamer-$(GST_API_VERSION)"

plugin_LTLIBRARIES = libgstinstruments.la

libgstinstruments_la_SOURCES = gstinstruments.c
libgstinstruments_la_CFLAGS = $(GST_CFLAGS) -DLIBDIR='"$(libdir)"' -DGST_USE_UNSTABLE_API='1'
libgstinstruments_la_LIBADD = $(GST_LIBS)

