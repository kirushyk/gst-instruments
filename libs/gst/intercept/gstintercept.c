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

#include "gstintercept.h"
#include <glib.h>
#include <time.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <gst/gst.h>
#include <gst/gstpad.h>
#include "../trace/gsttrace.h"
#include <config.h>
#include "../trace/spycommon.h"

#if __MACH__
#else
# define lgi_pad_push gst_pad_push
# define lgi_pad_push_list gst_pad_push_list
# define lgi_pad_push_event gst_pad_push_event
# define lgi_pad_pull_range gst_pad_pull_range
# define lgi_element_set_state gst_element_set_state
# define lgi_element_change_state gst_element_change_state
# include <signal.h>
# include <time.h>
#endif

gpointer libgstreamer = NULL;

GstTrace *current_trace = NULL;

GstStateChangeReturn (*gst_element_change_state_orig) (GstElement *element, GstStateChange transition) = NULL;
GstFlowReturn (*gst_pad_push_orig) (GstPad *pad, GstBuffer *buffer) = NULL;
GstFlowReturn (*gst_pad_push_list_orig) (GstPad *pad, GstBufferList *list) = NULL;
GstFlowReturn (*gst_pad_pull_range_orig) (GstPad *pad, guint64 offset, guint size, GstBuffer **buffer) = NULL;
gboolean (*gst_pad_push_event_orig) (GstPad *pad, GstEvent *event) = NULL;
GstStateChangeReturn (*gst_element_set_state_orig) (GstElement *element, GstState state) = NULL;

void *
get_libgstreamer ()
{
  if (libgstreamer == NULL) {
    libgstreamer = dlopen (LIBGSTREAMER, RTLD_NOW);
  }
  
  return libgstreamer;
}

void
optional_init ()
{
  if (current_trace == NULL) {
    current_trace = gst_trace_new ();
  }
  
#ifdef __MACH__
  
  gst_element_change_state_orig = gst_element_change_state;
  gst_pad_push_orig = gst_pad_push;
  gst_pad_push_list_orig = gst_pad_push_list;
  gst_pad_pull_range_orig = gst_pad_pull_range;
  gst_pad_push_event_orig = gst_pad_push_event;
  gst_element_set_state_orig = gst_element_set_state;
  
#else
  
  gst_element_change_state_orig = dlsym (get_libgstreamer (), "gst_element_change_state");
  
  if (gst_element_change_state_orig == NULL) {
    GST_ERROR ("can not link to gst_element_change_state\n");
    return;
  } else {
    GST_INFO ("gst_element_change_state linked: %p\n", gst_element_change_state_orig);
  }
  
  gst_pad_push_orig = dlsym (get_libgstreamer (), "gst_pad_push");
  
  if (gst_pad_push_orig == NULL) {
    GST_ERROR ("can not link to gst_pad_push\n");
    return;
  } else {
    GST_INFO ("gst_pad_push linked: %p\n", gst_pad_push_orig);
  }
  gst_pad_push_list_orig = dlsym (get_libgstreamer (), "gst_pad_push_list");
  
  if (gst_pad_push_list_orig == NULL) {
    GST_ERROR ("can not link to gst_pad_push_list\n");
    return;
  } else {
    GST_INFO ("gst_pad_push_list linked: %p\n", gst_pad_push_orig);
  }
  
  gst_pad_push_event_orig = dlsym (get_libgstreamer (), "gst_pad_push_event");
  
  if (gst_pad_push_event_orig == NULL) {
    GST_ERROR ("can not link to gst_pad_push_event\n");
    return;
  } else {
    GST_INFO ("gst_pad_push_event linked: %p\n", gst_pad_push_event_orig);
  }
  
  gst_pad_pull_range_orig = dlsym (get_libgstreamer (), "gst_pad_pull_range");
  
  if (gst_pad_pull_range_orig == NULL) {
    GST_ERROR ("can not link to gst_pad_pull_range\n");
    return;
  } else {
    GST_INFO ("gst_pad_pull_range linked: %p\n", gst_pad_pull_range_orig);
  }
  
  gst_element_set_state_orig = dlsym (get_libgstreamer (), "gst_element_set_state");
  
  if (gst_element_set_state_orig == NULL) {
    GST_ERROR ("can not link to gst_element_set_state\n");
    return;
  } else {
    GST_INFO ("gst_element_set_state linked: %p\n", gst_element_set_state_orig);
  }
  
#endif
  
}

GstStateChangeReturn
lgi_element_change_state (GstElement *element, GstStateChange transition)
{
  GstStateChangeReturn result;
  GstPipeline *pipeline = NULL;
  
  optional_init ();
  
  THREAD thread = mach_thread_self ();
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  {
    GstTraceElementEnteredEntry *entry = gst_trace_element_entered_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_entered_entry_set_upstack_element (entry, NULL);
    gst_trace_element_entered_entry_set_downstack_element (entry, element);
    gst_trace_element_entered_entry_set_enter_time (entry, start);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  result = gst_element_change_state_orig (element, transition);
  
  guint64 end = get_cpu_time (thread);
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  {
    GstTraceElementExitedEntry *entry = gst_trace_element_exited_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_exited_entry_set_downstack_element (entry, element);
    gst_trace_element_exited_entry_set_exit_time (entry, end);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  return result;
}

GstFlowReturn
lgi_pad_push (GstPad *sender_pad, GstBuffer *buffer)
{
  GstFlowReturn result;
  
  optional_init ();
  
  THREAD thread = mach_thread_self ();
  
  GstPad *receiver_pad = GST_PAD_PEER (sender_pad);
  receiver_pad = get_source_pad (receiver_pad);
  
  sender_pad = get_source_pad (sender_pad);
  
  GstElement *sender_element = GST_PAD_PARENT (sender_pad);
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstPipeline *pipeline = trace_heir (sender_element);
  
  guint64 start = get_cpu_time (thread);
  {
    GstTraceElementEnteredEntry *entry = gst_trace_element_entered_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_entered_entry_set_upstack_element (entry, sender_element);
    gst_trace_element_entered_entry_set_downstack_element (entry, receiver_element);
    gst_trace_element_entered_entry_set_enter_time (entry, start);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  dump_hierarchy_info_if_needed (current_trace, pipeline, receiver_element);
  
  GstTraceDataSentEntry *entry = gst_trace_data_sent_entry_new ();
  gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
  gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
  gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
  entry->pad_mode = GST_PAD_MODE_PUSH;
  entry->sender_element = sender_element;
  entry->receiver_element = receiver_element;
  entry->sender_pad = sender_pad;
  entry->receiver_pad = receiver_pad;
  entry->buffers_count = 1;
  entry->bytes_count = gst_buffer_get_size (buffer);
  gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  
  result = gst_pad_push_orig (sender_pad, buffer);
  
  guint64 end = get_cpu_time (thread);
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  {
    GstTraceElementExitedEntry *entry = gst_trace_element_exited_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_exited_entry_set_downstack_element (entry, receiver_element);
    gst_trace_element_exited_entry_set_exit_time (entry, end);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }

  return result;
}

GstFlowReturn
lgi_pad_push_list (GstPad *sender_pad, GstBufferList *list)
{
  GstFlowReturn result;
  
  optional_init ();
  
  THREAD thread = mach_thread_self ();
  
  GstPad *receiver_pad = GST_PAD_PEER (sender_pad);
  receiver_pad = get_source_pad (receiver_pad);
  sender_pad = get_source_pad (sender_pad);
  GstElement *sender_element = GST_PAD_PARENT (sender_pad);
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstPipeline *pipeline = trace_heir (sender_element);
  
  guint64 start = get_cpu_time (thread);
  {
    GstTraceElementEnteredEntry *entry = gst_trace_element_entered_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_entered_entry_set_upstack_element (entry, sender_element);
    gst_trace_element_entered_entry_set_downstack_element (entry, receiver_element);
    gst_trace_element_entered_entry_set_enter_time (entry, start);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  dump_hierarchy_info_if_needed (current_trace, pipeline, receiver_element);
  
  ListInfo list_info;
  gst_buffer_list_foreach (list, for_each_buffer, &list_info);
  
  GstTraceDataSentEntry *entry = gst_trace_data_sent_entry_new ();
  gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
  gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
  gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
  entry->pad_mode = GST_PAD_MODE_PUSH;
  entry->sender_element = sender_element;
  entry->receiver_element = receiver_element;
  entry->sender_pad = sender_pad;

  entry->receiver_pad = receiver_pad;  entry->buffers_count = list_info.buffers_count;
  entry->bytes_count = list_info.size;
  gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  
  result = gst_pad_push_list_orig (sender_pad, list);
    
  guint64 end = get_cpu_time (thread);
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  {
    GstTraceElementExitedEntry *entry = gst_trace_element_exited_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_exited_entry_set_downstack_element (entry, receiver_element);
    gst_trace_element_exited_entry_set_exit_time (entry, end);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  return result;
}

gboolean
lgi_pad_push_event (GstPad *sender_pad, GstEvent *event)
{
  gboolean result;
  GstPipeline *pipeline = NULL;
  
  optional_init ();
  
  THREAD thread = mach_thread_self ();
  
  gpointer receiver_element = NULL;
  GstPad *receiver_pad = GST_PAD_PEER (sender_pad);
  if (receiver_pad) {
    receiver_pad = get_source_pad (receiver_pad);
    if (receiver_pad) {
      receiver_element = GST_PAD_PARENT (receiver_pad);
    }
  }
  gpointer sender_element = GST_PAD_PARENT (sender_pad);
  
  pipeline = trace_heir (receiver_element);
  
  guint64 start = get_cpu_time (thread);
  
  if (sender_element && receiver_element) {
    GstTraceElementEnteredEntry *entry = gst_trace_element_entered_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_entered_entry_set_upstack_element (entry, NULL);
    gst_trace_element_entered_entry_set_downstack_element (entry, receiver_element);
    gst_trace_element_entered_entry_set_enter_time (entry, start);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  result = gst_pad_push_event_orig (sender_pad, event);
  
  guint64 end = get_cpu_time (thread);
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  if (sender_element && receiver_element) {
    GstTraceElementExitedEntry *entry = gst_trace_element_exited_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_exited_entry_set_downstack_element (entry, receiver_element);
    gst_trace_element_exited_entry_set_exit_time (entry, end);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  return result;
}

GstFlowReturn
lgi_pad_pull_range (GstPad *receiver_pad, guint64 offset, guint size, GstBuffer **buffer)
{
  GstFlowReturn result;
  GstPipeline *pipeline = NULL;
  
  optional_init ();
  
  THREAD thread = mach_thread_self ();
  
  GstPad *sender_pad = GST_PAD_PEER (receiver_pad);
  sender_pad = get_source_pad (sender_pad);
  
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstElement *sender_element = GST_PAD_PARENT (sender_pad);
  
  pipeline = trace_heir (sender_element);
  
  guint64 start = get_cpu_time (thread);
  {
    GstTraceElementEnteredEntry *entry = gst_trace_element_entered_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_entered_entry_set_upstack_element (entry, receiver_element);
    gst_trace_element_entered_entry_set_downstack_element (entry, sender_element);
    gst_trace_element_entered_entry_set_enter_time (entry, start);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  dump_hierarchy_info_if_needed (current_trace, pipeline, sender_element);
  
  result = gst_pad_pull_range_orig (receiver_pad, offset, size, buffer);
  
  if (*buffer) {
    GstPad *sender_pad = GST_PAD_PEER (receiver_pad);
    {
      GstTraceDataSentEntry *entry = gst_trace_data_sent_entry_new ();
      gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
      gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
      gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
      entry->pad_mode = GST_PAD_MODE_PULL;
      entry->sender_element = sender_element;
      entry->receiver_element = receiver_element;
      entry->sender_pad = sender_pad;
      entry->receiver_pad = receiver_pad;
      entry->buffers_count = 1;
      entry->bytes_count = gst_buffer_get_size (*buffer);
      gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
    }
  }
  
  guint64 end = get_cpu_time (thread);
  
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  {
    GstTraceElementExitedEntry *entry = gst_trace_element_exited_entry_new ();
    gst_trace_entry_set_timestamp ((GstTraceEntry *)entry, current_monotonic_time ());
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_entry_set_thread_id ((GstTraceEntry *)entry, g_thread_self ());
    gst_trace_element_exited_entry_set_downstack_element (entry, sender_element);
    gst_trace_element_exited_entry_set_exit_time (entry, end);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }

  return result;
}

GstStateChangeReturn
lgi_element_set_state (GstElement *element, GstState state)
{
  GstStateChangeReturn result;
  
  optional_init ();
  
  switch (state) {
  case GST_STATE_NULL:
    if (GST_IS_PIPELINE (element)) {
      const gchar *path = g_getenv ("GST_DEBUG_DUMP_TRACE_DIR");
      const gchar *name = g_getenv ("GST_DEBUG_DUMP_TRACE_FILENAME");
      gchar *filename = g_strdup_printf ("%s/%s.gsttrace", path ? path : ".", name ? name : GST_OBJECT_NAME (element));
      // gst_element_dump_to_file (element, filename);
      gst_trace_dump_pipeline_to_file (current_trace, (GstPipeline *)element, filename);
      g_free (filename);
    }
    break;

  default:
    break;

  }
  
  result = gst_element_set_state_orig (element, state);
  
  return result;
}

#if __MACH__
# define INTERPOSE(_replacment, _replacee) \
__attribute__ ((used)) static struct { const void* replacment; const void* replacee; } _interpose_##_replacee \
__attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned long)&_replacee };
INTERPOSE (lgi_pad_push, gst_pad_push);
INTERPOSE (lgi_pad_push_list, gst_pad_push_list);
INTERPOSE (lgi_pad_push_event, gst_pad_push_event);
INTERPOSE (lgi_pad_pull_range, gst_pad_pull_range);
INTERPOSE (lgi_element_set_state, gst_element_set_state);
INTERPOSE (lgi_element_change_state, gst_element_change_state);
#endif
