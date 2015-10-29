bin_PROGRAMS = gst-top-1.0 gst-report-1.0

gst_top_1_0_SOURCES = gst-top.c
gst_top_1_0_CFLAGS = $(GST_CFLAGS) -DLIBDIR='"$(libdir)"' -DBINDIR='"$(bindir)"'
gst_top_1_0_LDADD = $(GST_LIBS)

gst_report_1_0_SOURCES = gst-report.c gst-trace-parser.c formatters.c
gst_report_1_0_CFLAGS = $(GST_CFLAGS) -DDATADIR='"$(datadir)"' 
gst_report_1_0_LDADD = $(GST_LIBS)
