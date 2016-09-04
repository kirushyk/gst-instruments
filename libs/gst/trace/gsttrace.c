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

#include "gsttrace.h"
#include <sys/time.h>
#include <glib.h>
#include <stdio.h>
#include "../trace/gsttrace.h"
#include "../trace/gsttraceentry.h"

struct GstTrace
{
  GMutex        mutex;
  GList        *entries;
  GstClockTime  startup_time;
};

void
gst_trace_dump_pipeline_to_file (GstTrace *trace, GstPipeline *pipeline, const gchar *filename)
{
  GList *iterator;
  
  FILE *output = fopen (filename, "wb");
  if (output == NULL)
    return;
      
  g_mutex_lock (&trace->mutex);
  for (iterator = g_list_last (trace->entries); iterator != NULL; iterator = iterator->prev) {
    GstTraceEntry *entry = (GstTraceEntry *)iterator->data;
    if (entry) {
      if ((pipeline == NULL) || (pipeline == gst_trace_entry_get_pipeline (entry))) {
        gst_trace_entry_dump_to_file (entry, output);
      }
    }
  }
  g_mutex_unlock (&trace->mutex);
  
  fclose (output);
}

GstTrace *
gst_trace_new (void)
{
  GstTrace *trace = g_new0 (GstTrace, 1);
  trace->startup_time = GST_CLOCK_TIME_NONE;
  trace->entries = NULL;
  g_mutex_init (&trace->mutex);
  return trace;
}

void
gst_trace_free (GstTrace *trace)
{
  g_mutex_lock (&trace->mutex);
  trace->entries = g_list_remove_all (trace->entries, NULL);
  g_mutex_unlock (&trace->mutex);
  g_mutex_clear (&trace->mutex);
  g_free (trace);
}

void
gst_trace_add_entry (GstTrace *trace, GstPipeline *pipeline, GstTraceEntry *entry)
{
  if (trace == NULL) {
    return;
  }
  GstClockTime current_time = gst_trace_entry_get_timestamp (entry);
  if (trace->startup_time == GST_CLOCK_TIME_NONE) {
    trace->startup_time = current_time;
  }
  current_time -= trace->startup_time;
  g_mutex_lock (&trace->mutex);
  trace->entries = g_list_prepend (trace->entries, entry);
  g_mutex_unlock (&trace->mutex);
}
