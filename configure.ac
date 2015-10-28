#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([gst-instruments], [0.1], [kirushyk@gmail.com])
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
AM_PROG_VALAC
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

AC_DEFINE([GST_TOP_TRACE_FILENAME_BASE], ["gst-top"], [Base trace filename])

AC_ARG_ENABLE([ui], AS_HELP_STRING([--disable-ui], [Disable feature ui]))

AS_IF([test "x$enable_ui" != "xno"], [
	PKG_CHECK_MODULES([GTK], [gtk+-3.0])
	AC_SUBST([UI_DIR], [ui])],
	[ AC_SUBST([UI_DIR], [""]) ]
	 )

AC_OUTPUT
