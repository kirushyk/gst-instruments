#include <glib.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  g_set_prgname ("gst-top-1.0");
  g_set_application_name ("GStreamer Top Tool");
  
#if defined(__APPLE__)
  g_setenv ("DYLD_FORCE_FLAT_NAMESPACE", "", FALSE);
  g_setenv ("DYLD_INSERT_LIBRARIES", "libgstintercept.dylib", TRUE);
#elif defined(G_OS_UNIX)
  g_setenv ("LD_PRELOAD", "libgstintercept.so", TRUE);
#endif
  
  setenv ("GST_DEBUG_DUMP_INTERCEPT_PATH", "gst-top.intercept", 1);
  
  gint status = 0;
  GError *error = NULL;
  g_spawn_sync (NULL, argv + 1, NULL, G_SPAWN_SEARCH_PATH , NULL, NULL, NULL, NULL, &status, &error);
  
  if (error)
  {
    g_print("%s\n", error->message);
  }
  else
  {
    g_print ("statistics here\n");
  }
  
  return 0;
}
