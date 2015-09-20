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

#include "gst-trace-parser.h"
#include <stdio.h>


static GstElementHeadstone *
gst_graveyard_get_element (GstGraveyard *graveyard, gpointer element_id, gchar *element_name)
{
  GstElementHeadstone *element = g_hash_table_lookup (graveyard->elements, element_id);
  if (element)
  {
    if (element->name == NULL && element_name != NULL)
    {
      element->name = g_string_new (element_name);
    }
  }
  else
  {
    element = g_new0 (GstElementHeadstone, 1);
    element->bytes_sent = 0;
    element->bytes_received = 0;
    element->is_subtopstack = FALSE;
    element->identifier = element_id;
    element->total_time = 0;
    if (element_name)
      element->name = g_string_new (element_name);
    g_hash_table_insert (graveyard->elements, element_id, element);
  }
  return element;
}

gint
elements_compare_func (gconstpointer a, gconstpointer b)
{
  gint64 diff = (gint64)(*(GstElementHeadstone **)b)->total_time - (gint64)(*(GstElementHeadstone **)a)->total_time;
  if (diff > 0)
    return 1;
  else if (diff < 0)
    return -1;
  return 0;
}

void
for_each_task (gpointer key, gpointer value, gpointer user_data)
{
  GstGraveyard *graveyard = (GstGraveyard *)user_data;
  GstTaskHeadstone *task = value;
  if (task->upstack_element_identifier)
  {
    GstElementHeadstone *upstack_element = gst_graveyard_get_element (graveyard, task->upstack_element_identifier, task->name->str);
    
    upstack_element->total_time += task->total_upstack_time;
  }
}

void
for_each_element (gpointer key, gpointer value, gpointer user_data)
{
  GstGraveyard *graveyard = (GstGraveyard *)user_data;
  GstElementHeadstone *element = (GstElementHeadstone *)value;
  graveyard->total_time += element->total_time;
  g_array_append_val (graveyard->elements_sorted, value);
}

GstGraveyard *
gst_graveyard_new_from_trace (const char *filename, GstClockTime from, GstClockTime till)
{
  FILE *input = fopen (filename,  "rt");
  if (input == NULL)
    return NULL;
  
  GstGraveyard *graveyard = g_new0(GstGraveyard, 1);
  
  graveyard->tasks = g_hash_table_new (g_direct_hash, g_direct_equal);
  graveyard->elements = g_hash_table_new (g_direct_hash, g_direct_equal);
  
  while (!feof (input))
  {
    GstClockTime event_timestamp;
    gchar event_name[1000];
    if (fscanf (input, "%" G_GUINT64_FORMAT " %s", &event_timestamp, event_name) == 2)
    {
      if (g_ascii_strcasecmp (event_name, "element-discovered") == 0)
      {
        gpointer element_id;
        gchar element_name[1000];
        gchar element_type_name[1000];
        if (fscanf (input, "%p %s %s\n", &element_id, element_name, element_type_name) == 3)
        {
          
        }
      }
      else if (g_ascii_strcasecmp (event_name, "element-entered") == 0)
      {
        gpointer task_id;
        gchar from_element_name[1000];
        gpointer from_element_id;
        gchar element_name[1000];
        gpointer element_id;
        guint64 thread_time;
        if (fscanf (input, "%p %s %p %s %p %" G_GUINT64_FORMAT "\n", &task_id, from_element_name, &from_element_id, element_name, &element_id, &thread_time) == 6)
        {
          GstElementHeadstone *element = gst_graveyard_get_element (graveyard, element_id, element_name);
          
          g_assert_true (element != NULL);
          
          GstTaskHeadstone *task = g_hash_table_lookup (graveyard->tasks, task_id);
          if (!task)
          {
            task = g_new0 (GstTaskHeadstone, 1);
            task->identifier = task_id;
            task->total_upstack_time = 0;
            task->upstack_enter_timestamp = 0;
            task->currently_in_upstack_element = TRUE;
            task->name = NULL;
            task->upstack_element_identifier = NULL;
            g_hash_table_insert (graveyard->tasks, task_id, task);
          }
          
          g_assert_true (task != NULL);
          
          if (task->currently_in_upstack_element)
          {
            element->is_subtopstack = TRUE;
            task->upstack_exit_timestamp = thread_time;
            if ((task->upstack_enter_timestamp > 0) && TIMESTAMP_FITS (event_timestamp, from, till))
              task->total_upstack_time += task->upstack_exit_timestamp - task->upstack_enter_timestamp;
            task->currently_in_upstack_element = FALSE;
            if (task->name == NULL)
            {
              task->upstack_element_identifier = from_element_id;
              task->name = g_string_new (from_element_name);
            }
          }
          else
          {
            element->is_subtopstack = FALSE;
          }
          task->total_downstack_time += task->current_downstack_time;
          task->current_downstack_time = 0;
        }
        else
        {
          g_print ("couldn't parse event: %s\n", event_name);
          goto error;
        }
      }
      else if (g_ascii_strcasecmp (event_name, "data-sent") == 0)
      {
        gpointer element_from;
        gpointer element_to;
        gint buffers_count;
        guint64 size;
        if (fscanf (input, "%p %p %d %" G_GUINT64_FORMAT "\n", &element_from, &element_to, &buffers_count, &size) == 4)
        {
          GstElementHeadstone *element = gst_graveyard_get_element(graveyard, element_from, NULL);
          
          if (TIMESTAMP_FITS (event_timestamp, from, till))
          {
            element->bytes_sent += size;
          }
          
          element = gst_graveyard_get_element (graveyard, element_to, NULL);
          
          if (TIMESTAMP_FITS (event_timestamp, from, till))
          {
            element->bytes_received += size;
          }
        }
      }
      else if (g_ascii_strcasecmp (event_name, "element-exited") == 0)
      {
        gpointer task_id;
        gchar element_name[1000];
        gpointer element_id;
        guint64 thread_time;
        guint64 duration;
        if (fscanf (input, "%p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT "\n", &task_id, element_name, &element_id, &thread_time, &duration) == 5)
        {
          GstTaskHeadstone *task = g_hash_table_lookup (graveyard->tasks, task_id);
          if (!task)
          {
            g_print ("couldn't find task %p\n", task_id);
            goto error;
          }
          GstElementHeadstone *element = g_hash_table_lookup (graveyard->elements, element_id);
          if (!element)
          {
            g_print ("couldn't find element %p: %s\n", element_id, element_name);
            goto error;
          }
          if (TIMESTAMP_FITS (event_timestamp, from, till))
            element->total_time += duration - task->current_downstack_time;
          task->current_downstack_time = duration;
          if (element->is_subtopstack)
          {
            task->upstack_enter_timestamp = thread_time;
            task->currently_in_upstack_element = TRUE;
          }
          else
          {
            task->currently_in_upstack_element = FALSE;
          }
        }
        else
        {
          g_print ("couldn't parse event: %s\n", event_name);
          goto error;
        }
      }
      else if (g_ascii_strcasecmp (event_name, "task") == 0)
      {
        gpointer task_id;
        if (fscanf (input, "%p\n", &task_id) != 1)
        {
          g_print ("couldn't parse task\n");
          goto error;
        }
      }
      else
      {
        g_print ("couldn't recognize event: %s\n", event_name);
        goto error;
      }
    }
  }
  fclose (input);
  
  g_hash_table_foreach (graveyard->tasks, for_each_task, graveyard);
  
  gint elements_count = g_hash_table_size (graveyard->elements);
  graveyard->elements_sorted = g_array_sized_new (FALSE, FALSE, sizeof (gpointer), elements_count);
  g_hash_table_foreach (graveyard->elements, for_each_element, graveyard);
  g_array_sort (graveyard->elements_sorted, elements_compare_func);
  
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
