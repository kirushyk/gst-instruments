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

#define TIMESTAMP_FITS(ts, min, max) (((ts) >= (from)) || ((from) == GST_CLOCK_TIME_NONE)) && \
  (((ts) <= (till)) || ((till) == GST_CLOCK_TIME_NONE))

#include <config.h>
#include "gsttraceentry.h"
#include "gstgraveyard.h"
#include "gstpadheadstone.h"
#include "gsttaskheadstone.h"
#include "gstelementheadstone.h"
#include <stdio.h>

static GstElementHeadstone *
gst_graveyard_get_element (GstGraveyard *graveyard, gpointer element_id, gchar *element_name)
{
  GstElementHeadstone *element = g_hash_table_lookup (graveyard->elements, element_id);
  if (element) {
    if (element->name == NULL && element_name != NULL) {
      element->name = g_string_new (element_name);
    }
  } else {
    element = g_new0 (GstElementHeadstone, 1);
    element->bytes_sent = 0;
    element->bytes_received = 0;
    element->pads = g_hash_table_new (g_direct_hash, g_direct_equal);
    element->is_subtopstack = FALSE;
    element->identifier = element_id;
    element->parent = NULL;
    element->children = NULL;
    element->nesting_level = 0;
    element->name = NULL;
    element->type_name = NULL;
    element->total_cpu_time = 0;
    if (element_name) {
      element->name = g_string_new (element_name);
    }
    g_hash_table_insert (graveyard->elements, element_id, element);
  }
  return element;
}

gint
element_headstone_compare (gconstpointer a, gconstpointer b)
{
  gint64 diff = (gint64)(*(GstElementHeadstone **)b)->total_cpu_time - (gint64)(*(GstElementHeadstone **)a)->total_cpu_time;
  if (diff > 0) {
    return 1;
  } else if (diff < 0) {
    return -1;
  }
  return 0;
}

void
for_each_task (gpointer key, gpointer value, gpointer user_data)
{
  GstGraveyard *graveyard = (GstGraveyard *)user_data;
  GstTaskHeadstone *task = value;
  if (task->upstack_element_identifier) {
    GstElementHeadstone *upstack_element = gst_graveyard_get_element (graveyard, task->upstack_element_identifier, task->name->str);
    upstack_element->total_cpu_time += task->total_upstack_time;
  }
}

void
for_each_element (gpointer key, gpointer value, gpointer user_data)
{
  GstGraveyard *graveyard = (GstGraveyard *)user_data;
  GstElementHeadstone *element = (GstElementHeadstone *)value;
  GstElementHeadstone *parent;
  graveyard->total_cpu_time += element->total_cpu_time;
  if (element->name == NULL) {
    element->name = g_string_new("?");
  }
  if (element->type_name == NULL) {
    element->type_name = g_string_new("?");
  }
  element->nesting_level = 0;
  for (parent = element->parent; parent != NULL; parent = parent->parent) {
    element->nesting_level++;
  }
  element->cpu_load = (float)element->total_cpu_time / (float)(graveyard->till - graveyard->from);
  g_array_append_val (graveyard->elements_sorted, value);
}

GstGraveyard *
gst_graveyard_new_from_trace (const char *filename, GstClockTime from, GstClockTime till, gboolean query_duration_only)
{
  FILE *input = fopen (filename,  "rb");
  if (input == NULL) {
    return NULL;
  }
  
  GstGraveyard *graveyard = g_new0(GstGraveyard, 1);
  
  graveyard->tasks = g_hash_table_new (g_direct_hash, g_direct_equal);
  graveyard->elements = g_hash_table_new (g_direct_hash, g_direct_equal);
  
  while (!feof (input)) {
    gchar buffer[GST_TRACE_ENTRY_SIZE];
    
    if (fread(buffer, GST_TRACE_ENTRY_SIZE, 1, input) != 1) {
      break;
    }
    
    GstTraceEntry *entry = (GstTraceEntry *)buffer;
    
    GstClockTime event_timestamp = gst_trace_entry_get_timestamp(entry);
    if (event_timestamp > graveyard->duration) {
      graveyard->duration = event_timestamp;
    }
    
    if (query_duration_only) {
      continue;
    }
    
    switch (gst_trace_entry_get_type (entry)) {
      case GST_TRACE_ENTRY_TYPE_ELEMENT_DISCOVERED:
        {
          GstTraceElementDiscoveredEntry *ed_entry = (GstTraceElementDiscoveredEntry *)entry;
          GstElementHeadstone *element = gst_graveyard_get_element (graveyard, ed_entry->element_id, ed_entry->element_name);
          if (ed_entry->parent_element_id) {
            GstElementHeadstone *parent = gst_graveyard_get_element (graveyard, ed_entry->parent_element_id, NULL);
            if (element->type_name == NULL) {
              element->type_name = g_string_new (ed_entry->element_type_name);
            }
            gst_element_headstone_add_child (parent, element);
          }
        }
        break;
      case GST_TRACE_ENTRY_TYPE_ELEMENT_ENTERED:
        {
          GstTraceElementEnteredEntry *ee_entry = (GstTraceElementEnteredEntry *)entry;
          GstElementHeadstone *element = gst_graveyard_get_element (graveyard, ee_entry->downstack_element_id, ee_entry->downstack_element_name);
          
          g_assert_true (element != NULL);
          
          GstTaskHeadstone *task = g_hash_table_lookup (graveyard->tasks, entry->thread_id);
          if (!task) {
            task = g_new0 (GstTaskHeadstone, 1);
            task->identifier = ee_entry->entry.thread_id;
            task->total_upstack_time = 0;
            task->upstack_enter_timestamp = 0;
            task->currently_in_upstack_element = TRUE;
            task->name = NULL;
            task->upstack_element_identifier = NULL;
            g_hash_table_insert (graveyard->tasks, entry->thread_id, task);
          }
          
          g_assert_true (task != NULL);
          
          if (task->currently_in_upstack_element) {
            element->is_subtopstack = TRUE;
            task->upstack_exit_timestamp = entry->timestamp;
            if ((task->upstack_enter_timestamp > 0) && TIMESTAMP_FITS (event_timestamp, from, till)) {
              task->total_upstack_time += task->upstack_exit_timestamp - task->upstack_enter_timestamp;
            }
            task->currently_in_upstack_element = FALSE;
            if (task->name == NULL) {
              task->upstack_element_identifier = ee_entry->upstack_element_id;
              task->name = g_string_new (ee_entry->upstack_element_name);
            }
          } else {
            element->is_subtopstack = FALSE;
          }
          if (TIMESTAMP_FITS (event_timestamp, from, till)) {
            task->total_downstack_time += task->current_downstack_time;
          }
          task->current_downstack_time = 0;
        }
        break;
      case GST_TRACE_ENTRY_TYPE_ELEMENT_EXITED:
        {
          GstTraceElementExitedEntry *ee_entry = (GstTraceElementExitedEntry *)entry;
          GstTaskHeadstone *task = g_hash_table_lookup (graveyard->tasks, entry->thread_id);
          if (!task) {
            g_print ("couldn't find task %p\n", entry->thread_id);
            goto error;
          }
          GstElementHeadstone *element = g_hash_table_lookup (graveyard->elements, ee_entry->downstack_element_id);
          if (!element) {
            g_print ("couldn't find element %p: %s\n", ee_entry->downstack_element_id, ee_entry->downstack_element_name);
            goto error;
          }
          if (TIMESTAMP_FITS (event_timestamp, from, till)) {
            element->total_cpu_time += ee_entry->duration - task->current_downstack_time;
          }
          task->current_downstack_time = ee_entry->duration;
          if (element->is_subtopstack) {
            task->upstack_enter_timestamp = entry->timestamp;
            task->currently_in_upstack_element = TRUE;
          } else {
            task->currently_in_upstack_element = FALSE;
          }
        }
        break;
      case GST_TRACE_ENTRY_TYPE_DATA_SENT:
        break;
      case GST_TRACE_ENTRY_TYPE_UNKNOWN:
      default:
        break;
    }
    
    /*else if (g_ascii_strcasecmp (event_name, "data-sent") == 0)
    {
      gchar mode[2];
      GstPadMode pad_mode;
      gpointer element_from;
      gpointer element_to;
      gpointer pad_from;
      gpointer pad_to;
      gint buffers_count;
      guint64 size;
      if (fscanf (input, "%1s %p %p %p %p %d %" G_GUINT64_FORMAT "\n", mode, &element_from, &pad_from, &element_to, &pad_to, &buffers_count, &size) == 7) {
        GstElementHeadstone *element = gst_graveyard_get_element (graveyard, element_from, NULL);
        pad_mode = mode[0] == 'l' ? GST_PAD_MODE_PULL : GST_PAD_MODE_PUSH;
        
        if (TIMESTAMP_FITS (event_timestamp, from, till)) {
          element->bytes_sent += size;
          
          GstPadHeadstone *pad = g_hash_table_lookup (element->pads, pad_from);
          if (!pad) {
            pad = g_new0 (GstPadHeadstone, 1);
            pad->identifier = pad_from;
            pad->parent_element = element_from;
            pad->peer = pad_to;
            pad->peer_element = element_to;
            pad->mode = pad_mode;
            pad->bytes = 0;
            pad->direction = GST_PAD_SRC;
            g_hash_table_insert (element->pads, pad_from, pad);
          }
          pad->bytes += size;
        }
        
        element = gst_graveyard_get_element (graveyard, element_to, NULL);
        
        if (TIMESTAMP_FITS (event_timestamp, from, till)) {
          element->bytes_received += size;
          
          GstPadHeadstone *pad = g_hash_table_lookup (element->pads, pad_to);
          if (!pad) {
            pad = g_new0 (GstPadHeadstone, 1);
            pad->identifier = pad_to;
            pad->parent_element = element_to;
            pad->peer = pad_from;
            pad->peer_element = element_from;
            pad->mode = pad_mode;
            pad->bytes = 0;
            pad->direction = GST_PAD_SINK;
            g_hash_table_insert (element->pads, pad_to, pad);
          }
          pad->bytes += size;
        }
      } else {
        g_print ("couldn't parse event: %s\n", event_name);
        goto error;
      }
    }*/
  }
  fclose (input);
  
  g_hash_table_foreach (graveyard->tasks, for_each_task, graveyard);
  
  gint elements_count = g_hash_table_size (graveyard->elements);
  graveyard->elements_sorted = g_array_sized_new (FALSE, FALSE, sizeof (gpointer), elements_count);
  graveyard->from = (from == GST_CLOCK_TIME_NONE) ? 0 : from;
  graveyard->till = (till == GST_CLOCK_TIME_NONE) ? graveyard->duration : till;
  g_hash_table_foreach (graveyard->elements, for_each_element, graveyard);
  g_array_sort (graveyard->elements_sorted, element_headstone_compare);
  
  return graveyard;
  
error:
  gst_graveyard_free (graveyard);
  return NULL;
}

void
gst_graveyard_free (GstGraveyard *graveyard)
{
  g_array_free (graveyard->elements_sorted, TRUE);
  g_hash_table_destroy (graveyard->elements);
  g_hash_table_destroy (graveyard->tasks);
  g_free (graveyard);
}
