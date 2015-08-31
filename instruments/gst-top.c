#include <dlfcn.h>
#include <glib.h>
#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  g_set_prgname ("gst-top-1.0");
  g_set_application_name ("GStreamer Top Tool");
  
#if defined(__APPLE__)
  g_setenv ("DYLD_FORCE_FLAT_NAMESPACE", "", FALSE);
  g_setenv ("DYLD_INSERT_LIBRARIES", "/usr/local/lib/libgstintercept.dylib", TRUE);
#elif defined(G_OS_UNIX)
  g_setenv ("LD_PRELOAD", "/usr/local/lib/libgstintercept.so", TRUE);
#else
# error GStreamer API calls interception is not supported on this platform
#endif
  
  g_setenv ("GST_DEBUG_DUMP_TRACE_DIR", ".", TRUE);
  g_setenv ("GST_DEBUG_DUMP_TRACE_FILENAME", "gst-top", TRUE);
  
  gint status = 0;
  GError *error = NULL;
  g_spawn_sync (NULL, argv + 1, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &status, &error);
  
  if (error)
  {
    g_print ("%s\n", error->message);
  }
  else
  {
    system("/usr/local/bin/gst-report-1.0 gst-top.gsttrace");
  }
  
  return 0;
}
