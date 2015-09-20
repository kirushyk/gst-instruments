#define GST_TOP_TRACE_FILENAME_BASE "gst-top"
#define PREFIX "/usr/local/"
#if __MACH__
# define LIBGSTREAMER "libgstreamer-1.0.dylib"
#else
# define LIBGSTREAMER "libgstreamer-1.0.so"
#endif
