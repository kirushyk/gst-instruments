#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

void parse_output(const char *filename);

int
main (int argc, char *argv[])
{
  const char dump_filename[] = "gst-top.intercept";
  
  g_set_prgname ("gst-top-1.0");
  g_set_application_name ("GStreamer Top Tool");
  
#if defined(__APPLE__)
  g_setenv ("DYLD_FORCE_FLAT_NAMESPACE", "", FALSE);
  g_setenv ("DYLD_INSERT_LIBRARIES", "/usr/local/lib/libgstintercept.dylib", TRUE);
#elif defined(G_OS_UNIX)
  g_setenv ("LD_PRELOAD", "libgstintercept.so", TRUE);
#else
# error GStreamer API calls interception is not supported on this platform
#endif
  
  setenv ("GST_INTERCEPT_OUTPUT_FILE", dump_filename, 1);
  
  gint status = 0;
  GError *error = NULL;
  g_spawn_sync (NULL, argv + 1, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &status, &error);
  
  if (error)
  {
    g_print ("%s\n", error->message);
  }
  else
  {
    parse_output(dump_filename);
  }
  
  return 0;
}

void
parse_output (const char *filename)
{
  FILE *input = fopen (filename, "rt");
  while (!feof (input))
  {
    gchar event_name[1000];
    if (fscanf (input, "%s", event_name) == 1)
    {
      g_print ("%s: ", event_name);
      if (g_ascii_strcasecmp (event_name, "element-entered") == 0)
      {
        gpointer task_id;
        gchar element_name[1000];
        gpointer element_id;
        if (fscanf (input, "%p %s %p\n", &task_id, element_name, &element_id) == 3)
          g_print ("%p %s %p\n", task_id, element_name, element_id);
        else
        {
          g_printerr ("couldn't parse event: %s\n", event_name);
          exit (2);
        }
      }
      else if (g_ascii_strcasecmp (event_name, "element-exited") == 0)
      {
        gpointer task_id;
        gchar element_name[1000];
        gpointer element_id;
        guint64 duration;
        if (fscanf (input, "%p %s %p %" G_GUINT64_FORMAT"\n", &task_id, element_name, &element_id, &duration) == 4)
          g_print ("%p %s %p %" G_GUINT64_FORMAT"\n", task_id, element_name, element_id, duration);
        else
        {
          g_printerr ("couldn't parse event: %s\n", event_name);
          exit (2);
        }
      }
      else
      {
        g_printerr ("couldn't recognize event: %s\n", event_name);
        exit (1);
      }
    }
  }
  fclose (input);
}


