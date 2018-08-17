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

#ifndef __GST_TRACE_ENTRY_H__
#define __GST_TRACE_ENTRY_H__

#include <stdio.h>
#include <glib.h>
#include <gst/gst.h>
#include <config.h>

#define LGI_ELEMENT_NAME(element) ((element) != NULL) ? GST_ELEMENT_NAME (element) : "0"
#define LGI_OBJECT_TYPE_NAME(element) ((element) != NULL) ? G_OBJECT_TYPE_NAME (element) : "0"

typedef enum GstTraceEntryType
{
  GST_TRACE_ENTRY_TYPE_UNKNOWN,
  GST_TRACE_ENTRY_TYPE_ELEMENT_DISCOVERED,
  GST_TRACE_ENTRY_TYPE_ELEMENT_ENTERED,
  GST_TRACE_ENTRY_TYPE_ELEMENT_EXITED,
  GST_TRACE_ENTRY_TYPE_DATA_SENT
} GstTraceEntryType;

typedef struct GstTraceEntry GstTraceEntry;

/** @todo: Think about #pragma pack */
struct GstTraceEntry
{
  GstTraceEntryType type;
  GstClockTime timestamp;
  gpointer pipeline;
  gpointer thread_id;
};

typedef struct GstTraceElementDiscoveredEntry GstTraceElementDiscoveredEntry;

struct GstTraceElementDiscoveredEntry
{
  GstTraceEntry entry;
  gpointer element_id;
  gchar element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  gchar element_type_name[GST_ELEMENT_TYPE_NAME_LENGTH_MAX];
  gpointer parent_element_id;
};

typedef struct GstTraceElementEnteredEntry GstTraceElementEnteredEntry;

struct GstTraceElementEnteredEntry
{
  GstTraceEntry entry;
  gpointer upstack_element_id;
  gpointer downstack_element_id;
  gchar upstack_element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  gchar downstack_element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  GstClockTime enter_time;
};

typedef struct GstTraceElementExitedEntry GstTraceElementExitedEntry;

struct GstTraceElementExitedEntry
{
  GstTraceEntry entry;
  // gchar upstack_element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  gpointer downstack_element_id;
  gchar downstack_element_name[GST_ELEMENT_NAME_LENGTH_MAX];
  GstClockTime exit_time;
};

typedef struct GstTraceDataSentEntry GstTraceDataSentEntry;

struct GstTraceDataSentEntry
{
  GstTraceEntry entry;
  GstPadMode pad_mode;
  gpointer sender_element;
  gpointer receiver_element;
  gpointer sender_pad;
  gpointer receiver_pad;
  gint buffers_count;
  guint64 bytes_count;
};

void                gst_trace_entry_init                   (GstTraceEntry      *entry);

GstTraceEntryType   gst_trace_entry_get_type               (GstTraceEntry      *entry);

void                gst_trace_entry_set_pipeline           (GstTraceEntry      *entry,
                                                            GstPipeline        *pipeline);

gpointer            gst_trace_entry_get_pipeline           (GstTraceEntry      *entry);

void                gst_trace_entry_set_timestamp          (GstTraceEntry      *entry,
                                                            GstClockTime        timestamp);

GstClockTime        gst_trace_entry_get_timestamp          (GstTraceEntry      *entry);

void                gst_trace_entry_set_thread_id          (GstTraceEntry      *entry,
                                                            gpointer            thread_id);

void                gst_trace_entry_dump_to_file           (GstTraceEntry      *entry,
                                                            FILE               *fd);

GstTraceElementDiscoveredEntry * gst_trace_element_discoved_entry_new (void);

void                             gst_trace_element_discoved_entry_init_set_element (GstTraceElementDiscoveredEntry *entry,
                                                                                    GstElement    *element);

GstTraceElementEnteredEntry    * gst_trace_element_entered_entry_new (void);

void        gst_trace_element_entered_entry_set_upstack_element   (GstTraceElementEnteredEntry *entry,  GstElement *element);

void        gst_trace_element_entered_entry_set_downstack_element (GstTraceElementEnteredEntry *entry,  GstElement *element);

void                gst_trace_element_entered_entry_set_enter_time  (GstTraceElementEnteredEntry      *entry,
                                                                   GstClockTime        enter_time);

GstTraceElementExitedEntry    * gst_trace_element_exited_entry_new (void);

void        gst_trace_element_exited_entry_set_downstack_element (GstTraceElementExitedEntry *entry,  GstElement *element);

void                gst_trace_element_exited_entry_set_exit_time  (GstTraceElementExitedEntry      *entry,
                                                                  GstClockTime        exit_time);

GstTraceDataSentEntry * gst_trace_data_sent_entry_new (void);

#endif
