bin_PROGRAMS = \
  gst-top-@GST_API_VERSION@ \
  gst-report-@GST_API_VERSION@

gst_top_@GST_API_VERSION@_SOURCES = gst-top.c
gst_top_@GST_API_VERSION@_CFLAGS = $(GST_CFLAGS) -DLIBDIR='"$(libdir)"' -DBINDIR='"$(bindir)"'
gst_top_@GST_API_VERSION@_LDADD = $(GST_LIBS)

gst_report_@GST_API_VERSION@_SOURCES = \
  gst-report.c \
  formatters.c \
  ../libs/gst/trace/gstgraveyard.c \
  ../libs/gst/trace/gstpadheadstone.c \
  ../libs/gst/trace/gsttaskheadstone.c \
  ../libs/gst/trace/gstelementheadstone.c \
  ../libs/gst/trace/gsttrace.c \
  ../libs/gst/trace/gsttraceentry.c
gst_report_@GST_API_VERSION@_CFLAGS = $(GST_CFLAGS) -DDATADIR='"$(datadir)"'
gst_report_@GST_API_VERSION@_LDADD = $(GST_LIBS)
