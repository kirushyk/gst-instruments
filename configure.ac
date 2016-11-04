AC_INIT([gst-instruments], [0.2.3], [kirushyk@gmail.com])
AC_CONFIG_SRCDIR([tools/gst-top.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CC_STDC
LT_INIT

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PREREQ

PKG_CHECK_MODULES([GST], [gstreamer-1.0])

AC_CHECK_HEADERS(
  [fcntl.h \
   stdint.h \
   stdlib.h \
   string.h \
   sys/ioctl.h \
   unistd.h \
   mach/mach.h \
   sys/time.h]
)

AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap strerror strtoul clock_gettime])

AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_FILES(
  [Makefile \
   libs/gst/intercept/Makefile \
   plugins/tracers/Makefile \
   tools/Makefile \
   data/Makefile \
   ui/Makefile]
)

AC_DEFINE([GST_TOP_TRACE_FILENAME_BASE], ["gst-top"], [Trace filename base])
AC_DEFINE([GST_ELEMENT_TYPE_NAME_LENGTH_MAX], [128], [Max length of type name of GstElement])
AC_DEFINE([GST_ELEMENT_NAME_LENGTH_MAX], [128], [Max length of name of GstElement])
AC_DEFINE([GST_TRACE_ENTRY_SIZE], [512], [Fixed size of GstEntry, in bytes])
GST_API_VERSION=1.0
AC_SUBST(GST_API_VERSION)
AC_DEFINE_UNQUOTED(GST_API_VERSION, "$GST_API_VERSION", [GStreamer API Version])

AC_ARG_ENABLE([ui], AS_HELP_STRING([--disable-ui], [Disable Gtk+ UI]))

AC_SUBST([UI_DIR], [""])
echo "checking deps for Gtk+ UI..."
AS_IF([test "x$enable_ui" != "xno"], [
  echo "checking for vala..."
  AM_PROG_VALAC(, [
    AC_CHECK_PROG(DOT, dot, yes, no)
    AS_IF([test "x$DOT" != "xno"], [
      PKG_CHECK_MODULES([GTK], [gtk+-3.0], [
        enable_ui=yes
        AC_SUBST([UI_DIR], [ui])
      ], [
        enable_ui=no
        echo "Gtk+ 3.x not found, Gtk+ UI will not be built"
      ])
    ], [
      enable_ui=no
      echo "Graphviz not found, Gtk+ UI will not be built"
    ])
  ], [
    enable_ui=no
    echo "Vala not found, Gtk+ UI will not be built"
  ])
])

AC_OUTPUT

echo "

Configuration
	Version                    : ${VERSION}
	Source code location       : ${srcdir}
	Prefix                     : ${prefix}
	Compiler                   : ${CC}

	Gtk+ UI                    : ${enable_ui}
"
