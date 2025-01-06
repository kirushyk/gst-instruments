/* GStreamer Instruments
 * Copyright (C) 2015 Kyrylo Polezhaiev <kirushyk@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <signal.h>
#include <stdlib.h>
#include <glib.h>

gboolean   insert_libraries = TRUE;
GMainLoop *main_loop        = NULL;
GPid       child_pid        = 0;

static void
on_sigint (int signo)
{
  if (signo == SIGINT && child_pid) {
    kill (child_pid, SIGINT);
  }
}

static void
on_child_exit (GPid pid, gint status, gpointer user_data)
{
  const gchar *program_name = (const gchar *)user_data;
  if (status == EXIT_SUCCESS) {
    system (BINDIR "/gst-report-1.0 " GST_TOP_TRACE_FILENAME_BASE ".gsttrace");
  } else {
    g_warning ("%s (%" G_PID_FORMAT ") exited with code %d", program_name, pid, status);
  }
  g_main_loop_quit (main_loop);
}

gint
main (gint argc, gchar *argv[])
{
  /** @todo: Use -- to separate gst-top options from program and its arguments */
  if (argc < 2) {
    g_print ("Usage: %s PROG [ARGS]\n", argv[0]);
    return EXIT_FAILURE;
  }

  g_set_prgname ("gst-top-" GST_API_VERSION);
  g_set_application_name ("GStreamer Top Tool");

  /** @todo: Fix configuration vulnerability */
  system ("rm -f " GST_TOP_TRACE_FILENAME_BASE ".gsttrace");

  if (insert_libraries) {
    /** @todo: Check library presence */
#if defined(__MACH__)
    g_setenv ("DYLD_FORCE_FLAT_NAMESPACE", "1", FALSE);
    g_setenv ("DYLD_INSERT_LIBRARIES", LIBDIR "/libgstintercept.dylib", TRUE);
#elif defined(G_OS_UNIX)
    g_setenv ("LD_PRELOAD", LIBDIR "/libgstintercept.so", TRUE);
#else
#  error GStreamer API calls interception is not supported on this platform
#endif
  } else {
    /** @todo: Check tracer module presence */
    /** @todo: Read GST_TRACERS and append it with our tracer instead of just re-set it */
    g_setenv ("GST_PLUGIN_PATH", GST_PLUGIN_PATH, TRUE);
    g_setenv ("GST_TRACERS", "instruments", TRUE);
  }

  g_setenv ("GST_DEBUG_DUMP_TRACE_DIR", ".", TRUE);
  g_setenv ("GST_DEBUG_DUMP_TRACE_FILENAME", GST_TOP_TRACE_FILENAME_BASE, TRUE);

  GError *error = NULL;
  g_spawn_async (NULL, argv + 1, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &child_pid, &error);
  if (error == NULL) {
    main_loop = g_main_loop_new (NULL, FALSE);
    g_child_watch_add (child_pid, on_child_exit, argv[1]);
    if (signal (SIGINT, on_sigint) == SIG_ERR) {
      g_critical ("An error occurred while setting a signal handler.\n");
      return EXIT_FAILURE;
    }
    g_main_loop_run (main_loop);
    g_main_loop_unref (main_loop);
  } else {
    g_critical ("%s", error->message);
  }
  g_spawn_close_pid (child_pid);

  return EXIT_SUCCESS;
}
