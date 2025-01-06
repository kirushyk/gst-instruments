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

#define TIMESTAMP_FITS(ts, from, till)                                                                                 \
  (((ts) >= (from)) || ((from) == GST_CLOCK_TIME_NONE)) && (((ts) <= (till)) || ((till) == GST_CLOCK_TIME_NONE))

#include "gstgraveyard.h"
#include <config.h>
#include <stdio.h>
#include "gstelementheadstone.h"
#include "gstpadheadstone.h"
#include "gsttaskheadstone.h"
#include "gsttraceentry.h"

static GstElementHeadstone *
gst_graveyard_get_element (GstGraveyard *graveyard, gpointer element_id, gchar *element_name)
{
  GstElementHeadstone *element = g_hash_table_lookup (graveyard->elements, element_id);
  if (element) {
    if (element->name == NULL && element_name != NULL) {
      element->name = g_string_new (element_name);
    }
  } else {
    element                 = g_new0 (GstElementHeadstone, 1);
    element->bytes_sent     = 0;
    element->bytes_received = 0;
    element->pads           = g_hash_table_new (g_direct_hash, g_direct_equal);
    element->is_subtopstack = FALSE;
    element->identifier     = element_id;
    element->parent         = NULL;
    element->children       = NULL;
    element->nesting_level  = 0;
    element->name           = NULL;
    element->type_name      = NULL;
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
  gint64 diff =
      (gint64)(*(GstElementHeadstone **)b)->total_cpu_time - (gint64)(*(GstElementHeadstone **)a)->total_cpu_time;
  if (diff > 0) {
    return 1;
  } else if (diff < 0) {
    return -1;
  }
  return 0;
}

void
for_each_task (G_GNUC_UNUSED gpointer key, gpointer value, gpointer user_data)
{
  GstGraveyard     *graveyard = (GstGraveyard *)user_data;
  GstTaskHeadstone *task      = value;
  if (task->upstack_element_identifier) {
    GstElementHeadstone *upstack_element =
        gst_graveyard_get_element (graveyard, task->upstack_element_identifier, task->name->str);
    upstack_element->total_cpu_time += task->total_upstack_time;
  }
}

void
for_each_element (G_GNUC_UNUSED gpointer key, gpointer value, gpointer user_data)
{
  GstGraveyard        *graveyard = (GstGraveyard *)user_data;
  GstElementHeadstone *element   = (GstElementHeadstone *)value;
  GstElementHeadstone *parent;
  graveyard->total_cpu_time += element->total_cpu_time;
  if (element->name == NULL) {
    element->name = g_string_new ("?");
  }
  if (element->type_name == NULL) {
    element->type_name = g_string_new ("?");
  }
  element->nesting_level = 0;
  for (parent = element->parent; parent != NULL; parent = parent->parent) {
    element->nesting_level++;
  }
  element->cpu_load = (float)element->total_cpu_time / (float)(graveyard->till - graveyard->from);
  g_array_append_val (graveyard->elements_sorted, value);
}

ElementEnter *
gst_graveyard_pick_element_enter (GstGraveyard *graveyard, gpointer element_id, gpointer thread_id)
{
  for (GList *iterator = graveyard->enters; iterator != NULL; iterator = iterator->next) {
    ElementEnter *current = (ElementEnter *)iterator->data;
    if ((current->element_id == element_id) && (current->thread_id == thread_id)) {
      graveyard->enters = g_list_remove_link (graveyard->enters, iterator);
      return current;
    }
  }
  return NULL;
}

GstGraveyard *
gst_graveyard_new_from_trace (const char *filename, GstClockTime from, GstClockTime till, gboolean query_duration_only)
{
  FILE *input = fopen (filename, "rb");
  if (input == NULL) {
    return NULL;
  }

  GstGraveyard *graveyard = g_new0 (GstGraveyard, 1);
  graveyard->dsec         = 0;
  graveyard->enters       = NULL;

  graveyard->tasks    = g_hash_table_new (g_direct_hash, g_direct_equal);
  graveyard->elements = g_hash_table_new (g_direct_hash, g_direct_equal);

  while (!feof (input)) {
    gchar buffer[GST_TRACE_ENTRY_SIZE];

    if (fread (buffer, GST_TRACE_ENTRY_SIZE, 1, input) != 1) {
      break;
    }

    GstTraceEntry *entry = (GstTraceEntry *)buffer;

    GstClockTime event_timestamp = gst_trace_entry_get_timestamp (entry);
    if (event_timestamp != GST_CLOCK_TIME_NONE && event_timestamp > graveyard->duration) {
      graveyard->duration = event_timestamp;
    }

    if (query_duration_only) {
      continue;
    }

    switch (gst_trace_entry_get_type (entry)) {
    case GST_TRACE_ENTRY_TYPE_ELEMENT_DISCOVERED: {
      GstTraceElementDiscoveredEntry *ed_entry = (GstTraceElementDiscoveredEntry *)entry;
      GstElementHeadstone            *element =
          gst_graveyard_get_element (graveyard, ed_entry->element_id, ed_entry->element_name);
      if (ed_entry->parent_element_id) {
        GstElementHeadstone *parent = gst_graveyard_get_element (graveyard, ed_entry->parent_element_id, NULL);
        if (element->type_name == NULL) {
          element->type_name = g_string_new (ed_entry->element_type_name);
        }
        gst_element_headstone_add_child (parent, element);
      }
    } break;

    case GST_TRACE_ENTRY_TYPE_ELEMENT_ENTERED: {
      GstTraceElementEnteredEntry *ee_entry = (GstTraceElementEnteredEntry *)entry;
      GstElementHeadstone         *element =
          gst_graveyard_get_element (graveyard, ee_entry->downstack_element_id, ee_entry->downstack_element_name);

      g_assert_true (element != NULL);

      /// @todo: Extrude to procedure
      ElementEnter *element_enter = g_new0 (ElementEnter, 1);
      element_enter->element_id   = ee_entry->downstack_element_id;
      element_enter->thread_id    = ee_entry->entry.thread_id;
      element_enter->enter_time   = ee_entry->enter_time;
      graveyard->enters           = g_list_prepend (graveyard->enters, element_enter);

      GstTaskHeadstone *task = g_hash_table_lookup (graveyard->tasks, entry->thread_id);
      if (!task) {
        task                               = g_new0 (GstTaskHeadstone, 1);
        task->identifier                   = ee_entry->entry.thread_id;
        task->total_upstack_time           = 0;
        task->upstack_enter_timestamp      = 0;
        task->currently_in_upstack_element = TRUE;
        task->name                         = NULL;
        task->upstack_element_identifier   = NULL;
        g_hash_table_insert (graveyard->tasks, entry->thread_id, task);
      }

      g_assert_true (task != NULL);

      if (task->currently_in_upstack_element) {
        element->is_subtopstack      = TRUE;
        task->upstack_exit_timestamp = entry->timestamp;
        if ((task->upstack_enter_timestamp > 0) && TIMESTAMP_FITS (event_timestamp, from, till)) {
          task->total_upstack_time += task->upstack_exit_timestamp - task->upstack_enter_timestamp;
        }
        task->currently_in_upstack_element = FALSE;
        if (task->name == NULL) {
          task->upstack_element_identifier = ee_entry->upstack_element_id;
          task->name                       = g_string_new (ee_entry->upstack_element_name);
        }
      } else {
        element->is_subtopstack = FALSE;
      }
      if (TIMESTAMP_FITS (event_timestamp, from, till)) {
        task->total_downstack_time += task->current_downstack_time;
      }
      task->current_downstack_time = 0;
    } break;

    case GST_TRACE_ENTRY_TYPE_ELEMENT_EXITED: {
      GstTraceElementExitedEntry *ee_entry = (GstTraceElementExitedEntry *)entry;
      GstTaskHeadstone           *task     = g_hash_table_lookup (graveyard->tasks, entry->thread_id);
      if (!task) {
        /** @todo: Move lookup + new task allocation in single function */
        task                               = g_new0 (GstTaskHeadstone, 1);
        task->identifier                   = entry->thread_id;
        task->total_upstack_time           = 0;
        task->upstack_enter_timestamp      = 0;
        task->currently_in_upstack_element = TRUE;
        task->name                         = NULL;
        task->upstack_element_identifier   = NULL;
        g_hash_table_insert (graveyard->tasks, entry->thread_id, task);
      }
      GstElementHeadstone *element = gst_graveyard_get_element (graveyard, ee_entry->downstack_element_id, NULL);
      ElementEnter        *element_enter =
          gst_graveyard_pick_element_enter (graveyard, ee_entry->downstack_element_id, ee_entry->entry.thread_id);
      if (element_enter) {
        if (TIMESTAMP_FITS (event_timestamp, from, till)) {
          element->total_cpu_time += (ee_entry->exit_time - element_enter->enter_time) - task->current_downstack_time;
        }
        task->current_downstack_time = (ee_entry->exit_time - element_enter->enter_time);
        g_free (element_enter);
      } else {
        /// @note: We detected exit from downstack element but we have no clue when we entered it
      }
      if (element->is_subtopstack) {
        task->upstack_enter_timestamp      = entry->timestamp;
        task->currently_in_upstack_element = TRUE;
      } else {
        task->currently_in_upstack_element = FALSE;
      }
    } break;

    case GST_TRACE_ENTRY_TYPE_DATA_SENT: {
      graveyard->dsec++;
      // fprintf(stderr, "%d\n", graveyard->dsec);
      GstTraceDataSentEntry *ds_entry = (GstTraceDataSentEntry *)entry;

      GstElementHeadstone *element = gst_graveyard_get_element (graveyard, ds_entry->sender_element, NULL);

      if (TIMESTAMP_FITS (event_timestamp, from, till)) {
        element->bytes_sent += ds_entry->bytes_count;

        GstPadHeadstone *pad = g_hash_table_lookup (element->pads, ds_entry->sender_pad);
        if (!pad) {
          pad                 = g_new0 (GstPadHeadstone, 1);
          pad->identifier     = ds_entry->sender_pad;
          pad->parent_element = ds_entry->sender_element;
          pad->peer           = ds_entry->receiver_pad;
          pad->peer_element   = ds_entry->receiver_element;
          pad->mode           = ds_entry->pad_mode;
          pad->bytes          = 0;
          pad->direction      = GST_PAD_SRC;
          g_hash_table_insert (element->pads, ds_entry->sender_pad, pad);
        }
        pad->bytes += ds_entry->bytes_count;
      }

      element = gst_graveyard_get_element (graveyard, ds_entry->receiver_element, NULL);

      if (TIMESTAMP_FITS (event_timestamp, from, till)) {
        element->bytes_received += ds_entry->bytes_count;

        GstPadHeadstone *pad = g_hash_table_lookup (element->pads, ds_entry->receiver_pad);
        if (!pad) {
          pad                 = g_new0 (GstPadHeadstone, 1);
          pad->identifier     = ds_entry->receiver_pad;
          pad->parent_element = ds_entry->receiver_element;
          pad->peer           = ds_entry->sender_pad;
          pad->peer_element   = ds_entry->sender_element;
          pad->mode           = ds_entry->pad_mode;
          pad->bytes          = 0;
          pad->direction      = GST_PAD_SINK;
          g_hash_table_insert (element->pads, ds_entry->receiver_pad, pad);
        }
        pad->bytes += ds_entry->bytes_count;
      }
    } break;

    case GST_TRACE_ENTRY_TYPE_UNKNOWN:
    default:
      break;
    }
  }
  fclose (input);

  g_hash_table_foreach (graveyard->tasks, for_each_task, graveyard);

  gint elements_count        = g_hash_table_size (graveyard->elements);
  graveyard->elements_sorted = g_array_sized_new (FALSE, FALSE, sizeof (gpointer), elements_count);
  graveyard->from            = (from == GST_CLOCK_TIME_NONE) ? 0 : from;
  graveyard->till            = (till == GST_CLOCK_TIME_NONE) ? graveyard->duration : till;
  g_hash_table_foreach (graveyard->elements, for_each_element, graveyard);
  g_array_sort (graveyard->elements_sorted, element_headstone_compare);

  return graveyard;

  /* error:*/
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
