/* GStreamer Instruments
 * Copyright (C) 2015 Kyrylo Polezhaiev <kirushyk@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify i t under the terms of the GNU Library General Public
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

#include <glib.h>
#include <stdlib.h>
#include <config.h>

gboolean insert_libraries = FALSE;

gint
main (gint argc, gchar *argv[])
{
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
    g_setenv ("LD_PRELOAD", LIBDIR "/libgstintercept.so.0", TRUE);
#else
# error GStreamer API calls interception is not supported on this platform
#endif
  } else {
    /** @todo: Check tracer module presence */
    
  }
  
  g_setenv ("GST_DEBUG_DUMP_TRACE_DIR", ".", TRUE);
  g_setenv ("GST_DEBUG_DUMP_TRACE_FILENAME", GST_TOP_TRACE_FILENAME_BASE, TRUE);
  
  gint status = 0;
  GError *error = NULL;
  g_spawn_sync (NULL, argv + 1, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &status, &error);
  
  if (error) {
    g_critical ("%s", error->message);
  } else if (status != EXIT_SUCCESS) {
    g_warning ("%s exited with code %d", argv[1], status);
  } else {
    system (BINDIR "/gst-report-1.0 " GST_TOP_TRACE_FILENAME_BASE ".gsttrace");
  }
    
  return EXIT_SUCCESS;
}
