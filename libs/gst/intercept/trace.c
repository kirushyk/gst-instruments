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

#include "trace.h"
#include <sys/time.h>
#include <glib.h>
#include <stdio.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

GstClockTime current_monotonic_time()
{
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  return mts.tv_sec * GST_SECOND + mts.tv_nsec;
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * GST_SECOND + ts.tv_nsec;
#endif
}

GMutex trace_mutex;

typedef struct TraceEntry {
  GstClockTime timestamp;
  GstPipeline *pipeline;
  gchar *text;
} TraceEntry;

GList *trace_entries = NULL;
GstClockTime startup_time = GST_CLOCK_TIME_NONE;

void
gst_element_dump_to_file (GstElement *element, const gchar *filename)
{
  GList *iterator;
  g_mutex_lock (&trace_mutex);
  
  FILE *output = fopen (filename, "wt");
  
  for (iterator = g_list_last (trace_entries); iterator != NULL; iterator = iterator->prev) {
    TraceEntry *entry = (TraceEntry *)iterator->data;
    if (entry) {
      if ((element == NULL) || (GST_ELEMENT (entry->pipeline) == element)) {
        fprintf(output, "%" G_GUINT64_FORMAT " %s\n", entry->timestamp, entry->text);
        
        iterator->data = NULL;
        g_free(entry->text);
        g_free(entry);
      }
    }
  }
  
  /// @todo: Optimize removal
  trace_entries = g_list_remove_all(trace_entries, NULL);
  
  fclose (output);
  
  g_mutex_unlock (&trace_mutex);
}

void trace_init (void)
{
  g_mutex_init (&trace_mutex);
}

void
trace_add_entry (GstPipeline *pipeline, gchar *text)
{
  GstClockTime current_time = current_monotonic_time ();
  if (GST_CLOCK_TIME_NONE == startup_time) {
    startup_time = current_time;
  }
  current_time -= startup_time;
  
  TraceEntry *entry = g_new0 (TraceEntry, 1);
  entry->pipeline = pipeline;
  entry->timestamp = current_time;
  entry->text = text;
  
  g_mutex_lock (&trace_mutex);
  trace_entries = g_list_prepend (trace_entries, entry);
  g_mutex_unlock (&trace_mutex);
}
