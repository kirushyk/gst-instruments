#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([gst-instruments], [0.2.0], [kirushyk@gmail.com])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

LT_INIT

AC_CONFIG_MACRO_DIR([m4])

#found by autoscan

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PREREQ

# Checks for libraries.
PKG_CHECK_MODULES([GST], [gstreamer-1.0])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h mach/mach.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap strerror strtoul clock_gettime])

AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile libs/gst/intercept/Makefile instruments/Makefile data/Makefile ui/Makefile])

AC_DEFINE([GST_TOP_TRACE_FILENAME_BASE], ["gst-top"], [Trace filename base])
AC_DEFINE([GST_ELEMENT_TYPE_NAME_LENGTH_MAX], [128], [Max length of type name of GstElement])
AC_DEFINE([GST_ELEMENT_NAME_LENGTH_MAX], [128], [Max length of name of GstElement])
GST_API_VERSION=1.0
AC_SUBST(GST_API_VERSION)
AC_DEFINE_UNQUOTED(GST_API_VERSION, "$GST_API_VERSION", [GStreamer API Version])

AC_ARG_ENABLE([ui], AS_HELP_STRING([--disable-ui], [Disable feature ui]))

AS_IF([test "x$enable_ui" != "xno"], [
	AM_PROG_VALAC
	AC_CHECK_PROG(DOT, dot, yes)
	PKG_CHECK_MODULES([GTK], [gtk+-3.0])
	AC_SUBST([UI_DIR], [ui])],
	[ AC_SUBST([UI_DIR], [""]) ]
	 )

AC_OUTPUT

echo "

Configuration
	Source code location       : ${srcdir}
	Prefix                     : ${prefix}
	Compiler                   : ${CC}
"
