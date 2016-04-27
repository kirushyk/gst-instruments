AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libgstintercept.la

libgstintercept_la_SOURCES = gstintercept.c \
  ../trace/gstelementheadstone.c \
  ../trace/gstpadheadstone.c \
  ../trace/gsttaskheadstone.c \
  ../trace/gstgraveyard.c \
  ../trace/gsttrace.c \
  ../trace/gsttraceentry.c
libgstintercept_la_CFLAGS = $(GST_CFLAGS) -DLIBDIR='"$(libdir)"'
libgstintercept_la_LIBADD = $(GST_LIBS)
