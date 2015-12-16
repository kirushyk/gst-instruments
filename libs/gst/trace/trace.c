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
#include "trace.h"
#include "../trace/entry.h"

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

GstClockTime current_monotonic_time()
{
#ifdef __MACH__ // Mach does not have clock_gettime, use clock_get_time
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

struct GstTrace
{
  GMutex        trace_mutex;
  GList        *trace_entries;
  GstClockTime  startup_time;
}

void
gst_element_dump_to_file (GstElement *element, const gchar *filename)
{
  GList *iterator;
  
  FILE *output = fopen (filename, "wt");
  if (output == NULL)
    return;
      
  g_mutex_lock (&trace_mutex);
  for (iterator = g_list_last (trace_entries); iterator != NULL; iterator = iterator->prev) {
    TraceEntry *entry = (TraceEntry *)iterator->data;
    if (entry) {
      if ((element == NULL) || ((gpointer)entry->pipeline == (gpointer)element)) {
        fprintf(output, "%" G_GUINT64_FORMAT " %s\n", entry->timestamp, entry->text);
        
        iterator->data = NULL;
        g_free(entry->text);
        g_free(entry);
      }
    }
  }
  
  /// @todo: Optimize removal
  trace_entries = g_list_remove_all(trace_entries, NULL);
  g_mutex_unlock (&trace_mutex);
  
  fclose (output);
}

GstTrace *
gst_trace_new (void)
{
  GstTrace *trace = g_new0 (GstTrace, 1);
  trace->startup_time = GST_CLOCK_TIME_NONE;
  trace->trace_entries = NULL;
  g_mutex_init (&trace->trace_mutex);
}

void
gst_trace_free (GstTrace *trace)
{
  g_mutex_lock (&trace->trace_mutex);
  trace->trace_entries = g_list_remove_all(trace->trace_entries, NULL);
  g_mutex_unlock (&trace->trace_mutex);
  g_mutex_clear (&trace->trace_mutex);
  g_free(trace);
}

void
trace_add_entry (GstPipeline *pipeline, gchar *text)
{
  GstClockTime current_time = current_monotonic_time ();
  if (GST_CLOCK_TIME_NONE == startup_time) {
    startup_time = current_time;
  }
  current_time -= startup_time;
  
  GstTraceEntry *trace_entry = g_new0 (GstTraceEntry, 1);
  trace_entry->pipeline = pipeline;
  trace_entry->timestamp = current_time;
  
  g_mutex_lock (&trace_mutex);
  trace_entries = g_list_prepend (trace_entries, entry);
  g_mutex_unlock (&trace_mutex);
}
