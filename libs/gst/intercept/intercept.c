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

#include "intercept.h"
#include <glib.h>
#include <time.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <gst/gst.h>
#include <gst/gstpad.h>
#include "trace.h"
#include <configure-static.h>

#if __MACH__
# include <mach/mach_init.h>
# include <mach/thread_act.h>
# include <mach/mach_port.h>
# define THREAD thread_port_t
#else
# include <signal.h>
# include <time.h>
# define THREAD int
THREAD mach_thread_self() { return 0; }
#endif

#define LGI_ELEMENT_NAME(element) ((element) != NULL) ? GST_ELEMENT_NAME (element) : "0"
#define LGI_OBJECT_TYPE_NAME(element) ((element) != NULL) ? G_OBJECT_TYPE_NAME (element) : "0"

static guint64 get_cpu_time (THREAD thread) {
#if __MACH__
  mach_msg_type_number_t count = THREAD_EXTENDED_INFO_COUNT;
  thread_extended_info_data_t info;
  
  int kr = thread_info (thread, THREAD_EXTENDED_INFO, (thread_info_t) &info, &count);
  if (kr != KERN_SUCCESS) {
    return 0;
  }
  
  return (guint64) info.pth_user_time + info.pth_system_time;
#else
  struct timespec ts;
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts))
    return 0;
  
  return ts.tv_sec * GST_SECOND + ts.tv_nsec;
#endif
}

gpointer libgstreamer = NULL;

GstStateChangeReturn (*gst_element_change_state_orig) (GstElement *element, GstStateChange transition) = NULL;
GstFlowReturn (*gst_pad_push_orig) (GstPad *pad, GstBuffer *buffer) = NULL;
GstFlowReturn (*gst_pad_push_list_orig) (GstPad *pad, GstBufferList *list) = NULL;
GstFlowReturn (*gst_pad_pull_range_orig) (GstPad *pad, guint64 offset, guint size, GstBuffer **buffer) = NULL;
gboolean (*gst_pad_push_event_orig) (GstPad *pad, GstEvent *event) = NULL;
GstStateChangeReturn (*gst_element_set_state_orig) (GstElement *element, GstState state) = NULL;

void *
get_libgstreamer ()
{
  if (libgstreamer == NULL)
  {
    libgstreamer = dlopen (PREFIX "lib/" LIBGSTREAMER, RTLD_NOW);
  }
  
  trace_init();
  
  return libgstreamer;
}

gpointer trace_heir (GstElement *element)
{
  GstObject *parent = NULL;
  
  if (element == NULL)
    return NULL;
  
  for (parent = GST_OBJECT(element); GST_OBJECT_PARENT(parent) != NULL; parent = GST_OBJECT_PARENT(parent))
  {

  }
  
  return parent;
}

gpointer get_downstack_element (gpointer pad)
{
  gpointer element = pad;
  do
  {
    gpointer peer = GST_PAD_PEER (element);
    if (peer)
      element = GST_PAD_PARENT (peer);
    else
      return NULL;
  }
  while (!GST_IS_ELEMENT (element));
  
  return element;
}

GHashTable *pipeline_by_element = NULL;

void
dump_hierarchy_info_if_needed (GstPipeline *pipeline, GstElement *new_element)
{
  if (pipeline_by_element == NULL)
    pipeline_by_element = g_hash_table_new (g_direct_hash, g_direct_equal);
  else if (g_hash_table_lookup (pipeline_by_element, new_element))
    return;
  if (new_element)
    g_hash_table_insert (pipeline_by_element, new_element, pipeline);
  
  if (!g_hash_table_lookup (pipeline_by_element, pipeline)) {
    trace_add_entry (pipeline, g_strdup_printf ("element-discovered %p %s %s 0", pipeline, LGI_ELEMENT_NAME (pipeline), LGI_OBJECT_TYPE_NAME (pipeline)));
    g_hash_table_insert (pipeline_by_element, pipeline, pipeline);
  }
  
  if (pipeline == NULL)
    return;
  
  GstIterator *it = gst_bin_iterate_recurse (GST_BIN (pipeline));
  GValue item = G_VALUE_INIT;
  gboolean done = FALSE;
  while (!done) {
    switch (gst_iterator_next (it, &item)) {
      case GST_ITERATOR_OK:
        {
          GstElement *internal = g_value_get_object (&item);
          GstElement *parent = GST_ELEMENT_PARENT (internal);
          
          trace_add_entry (pipeline, g_strdup_printf ("element-discovered %p %s %s %p", internal, LGI_ELEMENT_NAME (internal), LGI_OBJECT_TYPE_NAME (internal), parent));
          g_value_reset (&item);
        }
        break;
      case GST_ITERATOR_RESYNC:
        gst_iterator_resync (it);
        break;
      case GST_ITERATOR_ERROR:
        done = TRUE;
        break;
      case GST_ITERATOR_DONE:
        done = TRUE;
        break;
    }
  }
  g_value_unset (&item);
  gst_iterator_free (it);
}

GstStateChangeReturn
gst_element_change_state (GstElement *element, GstStateChange transition)
{
  GstStateChangeReturn result;
  GstPipeline *pipeline = NULL;
  
  if (gst_element_change_state_orig == NULL)
  {
    gst_element_change_state_orig = dlsym (get_libgstreamer (), "gst_element_change_state");
    
    if (gst_element_change_state_orig == NULL)
    {
      GST_ERROR ("can not link to gst_element_change_state\n");
      return GST_FLOW_CUSTOM_ERROR;
    }
    else
    {
      GST_INFO ("gst_element_change_state linked: %p\n", gst_element_change_state_orig);
    }
  }
  
  THREAD thread = mach_thread_self ();
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  trace_add_entry (pipeline, g_strdup_printf ("element-entered %p gst_element_change_state 0 %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element), element, start));
  
  result = gst_element_change_state_orig (element, transition);
  
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element), element, end, duration));
  
  return result;
}

GstFlowReturn
gst_pad_push (GstPad *pad, GstBuffer *buffer)
{
  GstFlowReturn result;
  GstPipeline *pipeline = NULL;
  
  if (gst_pad_push_orig == NULL)
  {
    gst_pad_push_orig = dlsym (get_libgstreamer (), "gst_pad_push");
    
    if (gst_pad_push_orig == NULL)
    {
      GST_ERROR ("can not link to gst_pad_push\n");
      return GST_FLOW_CUSTOM_ERROR;
    }
    else
    {
      GST_INFO ("gst_pad_push linked: %p\n", gst_pad_push_orig);
    }
  }
  
  THREAD thread = mach_thread_self ();
  
  gpointer element_from = GST_PAD_PARENT (pad);
  gpointer element = get_downstack_element (pad);
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  trace_add_entry (pipeline, g_strdup_printf ("element-entered %p %s %p %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element_from), element_from, LGI_ELEMENT_NAME(element), element, start));
  
  dump_hierarchy_info_if_needed (pipeline, element);
  
  GstPad *peer = GST_PAD_PEER (pad);
  
  trace_add_entry (pipeline, g_strdup_printf ("data-sent s %p %p %p %p %d %" G_GUINT64_FORMAT, element_from, pad, element, peer, 1, gst_buffer_get_size (buffer)));
  result = gst_pad_push_orig (pad, buffer);
  
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element), element, end, duration));

  return result;
}

typedef struct ListInfo
{
  guint64 size;
  guint buffers_count;
} ListInfo;

gboolean
for_each_buffer (GstBuffer **buffer, guint idx, gpointer user_data)
{
  ListInfo *info = user_data;
  info->buffers_count++;
  info->size += gst_buffer_get_size(*buffer);
  return TRUE;
}

GstFlowReturn
gst_pad_push_list (GstPad *pad, GstBufferList *list)
{
  GstFlowReturn result;
  GstPipeline *pipeline = NULL;
  
  if (gst_pad_push_list_orig == NULL)
  {
    gst_pad_push_list_orig = dlsym (get_libgstreamer (), "gst_pad_push_list");
    
    if (gst_pad_push_list_orig == NULL) {
      GST_ERROR ("can not link to gst_pad_push_list\n");
      return GST_FLOW_CUSTOM_ERROR;
    } else {
      GST_INFO ("gst_pad_push_list linked: %p\n", gst_pad_push_orig);
    }
  }
  
  THREAD thread = mach_thread_self ();
  
  gpointer element_from = GST_PAD_PARENT (pad);
  gpointer element = get_downstack_element (pad);
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  trace_add_entry (pipeline, g_strdup_printf ("element-entered %p %s %p %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element_from), element_from, LGI_ELEMENT_NAME(element), element, start));
  
  dump_hierarchy_info_if_needed (pipeline, element);
  
  ListInfo list_info;
  gst_buffer_list_foreach (list, for_each_buffer, &list_info);
  GstPad *peer = GST_PAD_PEER (pad);
  
  trace_add_entry (pipeline, g_strdup_printf ("data-sent s %p %p %p %p %d %" G_GUINT64_FORMAT, element_from, pad, element, peer, list_info.buffers_count, list_info.size));
  
  result = gst_pad_push_list_orig (pad, list);
    
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element), element, end, duration));
  
  return result;
}

gboolean
gst_pad_push_event (GstPad *pad, GstEvent *event)
{
  gboolean result;
  GstPipeline *pipeline = NULL;
  
  if (gst_pad_push_event_orig == NULL)
  {
    gst_pad_push_event_orig = dlsym (get_libgstreamer (), "gst_pad_push_event");
    
    if (gst_pad_push_event_orig == NULL) {
      GST_ERROR ("can not link to gst_pad_push_event\n");
      return FALSE;
    } else {
      GST_INFO ("gst_pad_push_event linked: %p\n", gst_pad_push_event_orig);
    }
  }
  
  THREAD thread = mach_thread_self ();
  
  gpointer element_from = GST_PAD_PARENT (pad);
  gpointer element = get_downstack_element (pad);
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  if (element_from && element) {
    trace_add_entry (pipeline, g_strdup_printf ("element-entered %p %s %p %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element_from), element_from, LGI_ELEMENT_NAME(element), element, start));
  }
  
  result = gst_pad_push_event_orig (pad, event);
  
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  if (element_from && element) {
    trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element), element, end, duration));
  }
  
  return result;
}

GstFlowReturn
gst_pad_pull_range (GstPad *pad, guint64 offset, guint size, GstBuffer **buffer)
{
  GstFlowReturn result;
  GstPipeline *pipeline = NULL;
  
  if (gst_pad_pull_range_orig == NULL) {
    gst_pad_pull_range_orig = dlsym (get_libgstreamer (), "gst_pad_pull_range");

    if (gst_pad_pull_range_orig == NULL) {
      GST_ERROR ("can not link to gst_pad_pull_range\n");
      return GST_FLOW_CUSTOM_ERROR;
    } else {
      GST_INFO ("gst_pad_pull_range linked: %p\n", gst_pad_pull_range_orig);
    }
  }
  
  THREAD thread = mach_thread_self();
  
  gpointer element_from = GST_PAD_PARENT (pad);
  gpointer element = get_downstack_element (pad);
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  trace_add_entry (pipeline, g_strdup_printf ("element-entered %p %s %p %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element_from), element_from, LGI_ELEMENT_NAME(element), element, start));
  
  dump_hierarchy_info_if_needed (pipeline, element);
  
  result = gst_pad_pull_range_orig (pad, offset, size, buffer);
  
  if (*buffer) {
    GstPad *peer = GST_PAD_PEER (pad);
    trace_add_entry (pipeline, g_strdup_printf ("data-sent l %p %p %p %p %d %" G_GUINT64_FORMAT, element, peer, element_from, pad, 1, gst_buffer_get_size (*buffer)));
  }
  
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element), element, end, duration));

  return result;
}

GstStateChangeReturn
gst_element_set_state (GstElement *element, GstState state)
{
  GstStateChangeReturn result;
  
  if (gst_element_set_state_orig == NULL)
  {
    gst_element_set_state_orig = dlsym (get_libgstreamer (), "gst_element_set_state");
    
    if (gst_element_set_state_orig == NULL) {
      GST_ERROR ("can not link to gst_element_set_state\n");
      return GST_FLOW_CUSTOM_ERROR;
    } else {
      GST_INFO ("gst_element_set_state linked: %p\n", gst_element_set_state_orig);
    }
  }
  
  result = gst_element_set_state_orig (element, state);
  
  switch (state)
  {
  case GST_STATE_NULL:
    if (GST_IS_PIPELINE (element)) {
      const gchar *path = g_getenv ("GST_DEBUG_DUMP_TRACE_DIR");
      const gchar *name = g_getenv ("GST_DEBUG_DUMP_TRACE_FILENAME");
      gchar *filename = g_strdup_printf ("%s/%s.gsttrace", path ? path : ".", name ? name : GST_OBJECT_NAME (element));
      gst_element_dump_to_file (element, filename);
      g_free (filename);
    }
    break;

  default:
    break;

  }
  
  return result;
}
