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

#include "gsttraceentry.h"

typedef struct GstTraceEntry
{
  GstTraceEntryType type;
  GstClockTime timestamp;
  gpointer pipeline;
} GstTraceEntry;

typedef struct GstTraceElementDiscoveredEntry
{
  GstTraceEntry entry;
  gpointer element_id;
  gchar element_name[GST_ELEMENT_TYPE_NAME_LENGTH_MAX];
  gchar element_type_name[GST_ELEMENT_NAME_LENGTH_MAX];
  gpointer parent_element_id;
} GstTraceElementDiscoveredEntry;

typedef struct GstTraceElementEnteredEntry
{
  GstTraceEntry entry;
  gpointer thread_id;
  gpointer upperstack_element_id;
  gpointer downstack_element_id;
  gchar upperstack_element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  gchar downstack_element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  guint64 start;
} GstTraceElementEnteredEntry;

typedef struct GstTraceElementExitedEntry
{
  GstTraceEntry entry;
  gpointer thread_id;
  gchar upperstack_element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  gchar downstack_element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  guint64 end;
  guint64 duration;
} GstTraceElementExitedEntry;

void
gst_trace_entry_init (GstTraceEntry *entry)
{
  g_assert (entry != NULL);
  entry->type = GST_TRACE_ENTRY_TYPE_UNKNOWN;
  entry->timestamp = GST_CLOCK_TIME_NONE;
  entry->pipeline = NULL;
}

gpointer
gst_trace_entry_get_pipeline (GstTraceEntry *entry)
{
  g_assert (entry != NULL);
  return entry->pipeline;
}

void
gst_trace_entry_set_timestamp (GstTraceEntry *entry, GstClockTime timestamp)
{
  g_assert (entry != NULL);
  entry->timestamp = timestamp;
}

GstClockTime
gst_trace_entry_get_timestamp (GstTraceEntry *)
{
  g_assert (entry != NULL);
  return entry->timestamp;
}

void
gst_trace_entry_dump_to_fd (GstTraceEntry *entry, FILE *fd)
{
  
}

void
gst_trace_element_discoved_entry_init (GstTraceElementDiscoveredEntry *entry)
{
  gst_trace_entry_init ((GstTraceEntry *)entry);
  ((GstTraceEntry *)entry)->type = GST_TRACE_ENTRY_TYPE_UNKNOWN;
}

