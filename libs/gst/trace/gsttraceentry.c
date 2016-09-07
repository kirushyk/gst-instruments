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
#include <config.h>
#include <string.h>

void
gst_trace_entry_init (GstTraceEntry *entry)
{
  g_assert (entry != NULL);
  entry->type = GST_TRACE_ENTRY_TYPE_UNKNOWN;
  entry->timestamp = GST_CLOCK_TIME_NONE;
  entry->pipeline = NULL;
  entry->thread_id = NULL;
}

GstTraceEntryType
gst_trace_entry_get_type (GstTraceEntry *entry)
{
  g_assert (entry != NULL);
  return entry->type;
}

void
gst_trace_entry_set_pipeline (GstTraceEntry *entry, GstPipeline *pipeline)
{
  g_assert (entry != NULL);
  entry->pipeline = pipeline;
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
gst_trace_entry_get_timestamp (GstTraceEntry *entry)
{
  g_assert (entry != NULL);
  return entry->timestamp;
}

void
gst_trace_entry_set_thread_id (GstTraceEntry *entry, gpointer thread_id)
{
  g_assert (entry != NULL);
  entry->thread_id = thread_id;
}

size_t
gst_trace_entry_get_size (GstTraceEntry *entry)
{
  g_return_val_if_fail (entry != NULL, 0);
  
  switch (entry->type) {
  case GST_TRACE_ENTRY_TYPE_ELEMENT_DISCOVERED:
    return sizeof (GstTraceElementDiscoveredEntry);
  case GST_TRACE_ENTRY_TYPE_ELEMENT_ENTERED:
    return sizeof (GstTraceElementEnteredEntry);
  case GST_TRACE_ENTRY_TYPE_ELEMENT_EXITED:
    return sizeof (GstTraceElementExitedEntry);
  case GST_TRACE_ENTRY_TYPE_DATA_SENT:
    return sizeof (GstTraceDataSentEntry);
  case GST_TRACE_ENTRY_TYPE_UNKNOWN:
  default:
    return 0;
  }
}

void
gst_trace_entry_dump_to_file (GstTraceEntry *entry, FILE *fd)
{
  char buffer[GST_TRACE_ENTRY_SIZE];
  memset (buffer, 0, GST_TRACE_ENTRY_SIZE);
  size_t size = gst_trace_entry_get_size (entry);
  fwrite (entry, size, 1, fd);
  fwrite (buffer, 1, 512 - size, fd);
}

void
gst_trace_element_discoved_entry_init (GstTraceElementDiscoveredEntry *entry)
{
  gst_trace_entry_init ((GstTraceEntry *)entry);
  ((GstTraceEntry *)entry)->type = GST_TRACE_ENTRY_TYPE_ELEMENT_DISCOVERED;
  entry->element_id = NULL;
  entry->element_name[0] = '\0';
  entry->element_type_name[0] = '\0';
  entry->parent_element_id = NULL;
}

GstTraceElementDiscoveredEntry *
gst_trace_element_discoved_entry_new (void)
{
  GstTraceElementDiscoveredEntry *entry = g_new0 (GstTraceElementDiscoveredEntry, 1);
  gst_trace_element_discoved_entry_init (entry);
  return entry;
}

void
gst_trace_element_discoved_entry_init_set_element (GstTraceElementDiscoveredEntry *entry,  GstElement *element)
{
  entry->element_id = element;
  g_strlcpy (entry->element_name, LGI_ELEMENT_NAME (element), GST_ELEMENT_NAME_LENGTH_MAX);
  g_strlcpy (entry->element_type_name, LGI_OBJECT_TYPE_NAME (element), GST_ELEMENT_TYPE_NAME_LENGTH_MAX);
  entry->parent_element_id = GST_ELEMENT_PARENT (element);
}

void
gst_trace_element_entered_entry_init (GstTraceElementEnteredEntry *entry)
{
  gst_trace_entry_init ((GstTraceEntry *)entry);
  ((GstTraceEntry *)entry)->type = GST_TRACE_ENTRY_TYPE_ELEMENT_ENTERED;
  entry->upstack_element_id = NULL;
  entry->downstack_element_id = NULL;
  entry->upstack_element_name[0] = '\0';
  entry->downstack_element_name[0] = '\0';
}

GstTraceElementEnteredEntry *
gst_trace_element_entered_entry_new (void)
{
  GstTraceElementEnteredEntry *entry = g_new0 (GstTraceElementEnteredEntry, 1);
  gst_trace_element_entered_entry_init (entry);
  return entry;
}

void
gst_trace_element_entered_entry_set_upstack_element (GstTraceElementEnteredEntry *entry, GstElement *element)
{
  entry->upstack_element_id = element;
  g_strlcpy (entry->upstack_element_name, LGI_ELEMENT_NAME (element), GST_ELEMENT_NAME_LENGTH_MAX);
}

void
gst_trace_element_entered_entry_set_downstack_element (GstTraceElementEnteredEntry *entry, GstElement *element)
{
  entry->downstack_element_id = element;
  g_strlcpy (entry->downstack_element_name, LGI_ELEMENT_NAME (element), GST_ELEMENT_NAME_LENGTH_MAX);
}

void
gst_trace_element_entered_exited_init (GstTraceElementExitedEntry *entry)
{
  gst_trace_entry_init ((GstTraceEntry *)entry);
  ((GstTraceEntry *)entry)->type = GST_TRACE_ENTRY_TYPE_ELEMENT_EXITED;
  entry->downstack_element_id = NULL;
  entry->downstack_element_name[0] = '\0';
  entry->duration = GST_CLOCK_TIME_NONE;
}

GstTraceElementExitedEntry *
gst_trace_element_exited_entry_new (void)
{
  GstTraceElementExitedEntry *entry = g_new0(GstTraceElementExitedEntry, 1);
  gst_trace_element_entered_exited_init (entry);
  return entry;
}

void
gst_trace_element_exited_entry_set_downstack_element (GstTraceElementExitedEntry *entry, GstElement *element)
{
  entry->downstack_element_id = element;
  g_strlcpy (entry->downstack_element_name, LGI_ELEMENT_NAME (element), GST_ELEMENT_NAME_LENGTH_MAX);
}

void
gst_trace_element_exited_entry_set_duration (GstTraceElementExitedEntry *entry, GstClockTime duration)
{
  entry->duration = duration;
}

void
gst_trace_data_sent_entry_init (GstTraceDataSentEntry *entry)
{
  gst_trace_entry_init ((GstTraceEntry *)entry);
  ((GstTraceEntry *)entry)->type = GST_TRACE_ENTRY_TYPE_DATA_SENT;
  entry->pad_mode = GST_PAD_MODE_NONE;
  entry->sender_element = NULL;
  entry->receiver_element = NULL;
  entry->sender_pad = NULL;
  entry->receiver_pad = NULL;
  entry->buffers_count = 0;
  entry->bytes_count = 0;
}

GstTraceDataSentEntry *
gst_trace_data_sent_entry_new (void)
{
  GstTraceDataSentEntry *entry = g_new0(GstTraceDataSentEntry, 1);
  gst_trace_data_sent_entry_init (entry);
  return entry;
}
