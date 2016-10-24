lib_LTLIBRARIES = libgstintercept.la

libgstintercept_la_SOURCES = \
  gstintercept.c \
  ../trace/gsttrace.c \
  ../trace/gstgraveyard.c \
  ../trace/gsttraceentry.c \
  ../trace/gstpadheadstone.c \
  ../trace/gsttaskheadstone.c \
  ../trace/gstelementheadstone.c \
  ../trace/spycommon.c
libgstintercept_la_CFLAGS = $(GST_CFLAGS) -DLIBDIR='"$(libdir)"'
libgstintercept_la_LIBADD = $(GST_LIBS)
