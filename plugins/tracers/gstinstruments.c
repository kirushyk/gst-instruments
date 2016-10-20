/* GStreamer
 * Copyright (C) 2016 Kyrylo Polezhaiev <kirushyk@gmail.com>
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

#include "config.h"
#include <gst/gst.h>
#include "../../libs/gst/trace/gsttrace.h"
#include "../../libs/gst/trace/spycommon.h"
#include "gstinstruments.h"

GST_DEBUG_CATEGORY_STATIC (gst_instruments_debug);
#define GST_CAT_DEFAULT gst_instruments_debug

#define _do_init \
GST_DEBUG_CATEGORY_INIT (gst_instruments_debug, "instruments", 0, "instruments tracer");
#define gst_instruments_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstInstrumentsTracer, gst_instruments_tracer, GST_TYPE_TRACER,  _do_init);

GstTrace *current_trace = NULL;

static GstElement *
get_real_pad_parent (GstPad *pad)
{
  GstObject *parent;
  
  if (!pad)
    return NULL;
  
  parent = GST_OBJECT_PARENT (pad);
  
  /* if parent of pad is a ghost-pad, then pad is a proxy_pad */
  if (parent && GST_IS_GHOST_PAD (parent)) {
    pad = GST_PAD_CAST (parent);
    parent = GST_OBJECT_PARENT (pad);
  }
  return GST_ELEMENT_CAST (parent);
}

void
optional_init ()
{
  if (current_trace == NULL) {
    current_trace = gst_trace_new ();
  }
}

static void
do_push_buffer_pre (GObject *self, GstClockTime ts, GstPad *sender_pad, GstBuffer *buffer)
{
  if (GST_IS_GHOST_PAD (sender_pad))
    return;
  
  optional_init ();
  
  GstPad *receiver_pad = GST_PAD_PEER (sender_pad);
  if (GST_IS_GHOST_PAD (receiver_pad))
    return;
  receiver_pad = get_source_pad (receiver_pad);
  
  sender_pad = get_source_pad (sender_pad);
  
  GstElement *sender_element = get_real_pad_parent (sender_pad);
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstPipeline *pipeline = trace_heir (sender_element);
  dump_hierarchy_info_if_needed (current_trace, pipeline, sender_element);
  
  /** @note: Do I need mach_port_deallocate call after logging event ? */
  THREAD thread = mach_thread_self ();
  guint64 start = get_cpu_time (thread);
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
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
}

static void
do_push_buffer_list_pre (GObject *self, GstClockTime ts, GstPad *sender_pad, GstBufferList *list)
{
  if (GST_IS_GHOST_PAD (sender_pad))
    return;
  
  optional_init ();
  
  GstPad *receiver_pad = GST_PAD_PEER (sender_pad);
  if (GST_IS_GHOST_PAD (receiver_pad))
    return;
  receiver_pad = get_source_pad (receiver_pad);
  
  sender_pad = get_source_pad (sender_pad);
  
  GstElement *sender_element = get_real_pad_parent (sender_pad);
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstPipeline *pipeline = trace_heir (sender_element);
  dump_hierarchy_info_if_needed (current_trace, pipeline, sender_element);
  
  THREAD thread = mach_thread_self ();
  guint64 start = get_cpu_time (thread);
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
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
  entry->receiver_pad = receiver_pad;
  entry->buffers_count = list_info.buffers_count;
  entry->bytes_count = list_info.size;
  gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
}

static void
do_pull_range_pre (GObject *self, GstClockTime ts, GstPad *receiver_pad, guint64 offset, guint size)
{
  optional_init ();
  
  GstPad *sender_pad = GST_PAD_PEER (receiver_pad);
  sender_pad = get_source_pad (sender_pad);
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstPipeline *pipeline = trace_heir (receiver_element);
  GstElement *sender_element = GST_PAD_PARENT (sender_pad);
  
  THREAD thread = mach_thread_self ();
  guint64 start = get_cpu_time (thread);
#if __MACH__
  mach_port_deallocate (mach_task_self (), thread);
#endif
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
  
  dump_hierarchy_info_if_needed (current_trace, pipeline, receiver_element);
}

void
do_pull_range_post (GObject *self, GstClockTime ts, GstPad *receiver_pad, GstBuffer *buffer, GstFlowReturn res)
{
  if (GST_IS_GHOST_PAD (receiver_pad))
    return;
  
  optional_init ();
  
  GstPad *sender_pad = GST_PAD_PEER (receiver_pad);
  sender_pad = get_source_pad (sender_pad);
  
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstElement *sender_element = GST_PAD_PARENT (sender_pad);
  GstPipeline *pipeline = trace_heir (sender_element);
  
  if (buffer)
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
    entry->bytes_count = gst_buffer_get_size (buffer);
    gst_trace_add_entry (current_trace, pipeline, (GstTraceEntry *)entry);
  }
  
  THREAD thread = mach_thread_self ();
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
}

static void
do_push_buffer_post (GstTracer *self, guint64 ts, GstPad *sender_pad)
{
  optional_init ();
  
  GstPad *receiver_pad = GST_PAD_PEER (sender_pad);
  if (GST_IS_GHOST_PAD (receiver_pad))
    return;
  receiver_pad = get_source_pad (receiver_pad);
  
  sender_pad = get_source_pad (sender_pad);
  
  GstElement *sender_element = get_real_pad_parent (sender_pad);
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstPipeline *pipeline = trace_heir (sender_element);
  
  THREAD thread = mach_thread_self ();
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
}

static void
do_push_buffer_list_post (GstTracer *self, guint64 ts, GstPad *sender_pad)
{
  optional_init ();
  
  GstPad *receiver_pad = GST_PAD_PEER (sender_pad);
  if (GST_IS_GHOST_PAD (receiver_pad))
    return;
  receiver_pad = get_source_pad (receiver_pad);
  
  sender_pad = get_source_pad (sender_pad);
  
  GstElement *sender_element = get_real_pad_parent (sender_pad);
  GstElement *receiver_element = GST_PAD_PARENT (receiver_pad);
  GstPipeline *pipeline = trace_heir (sender_element);
  
  THREAD thread = mach_thread_self ();
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
}

static void
do_push_event_pre (GstTracer *self, guint64 ts, GstPad *pad, GstEvent *ev)
{
  optional_init ();
  
  GstElement *element = get_real_pad_parent (pad);
  if (element) {
    GstPipeline *pipeline = trace_heir (element);
    dump_hierarchy_info_if_needed (current_trace, pipeline, element);
  }
}

static void
gst_instruments_tracer_class_init (GstInstrumentsTracerClass * klass)
{
  
}

void
do_element_change_state_pre (GObject *self, GstClockTime ts, GstElement *element, GstStateChange transition)
{
  optional_init ();
}

void
do_element_change_state_post (GObject *self, GstClockTime ts, GstElement *element, GstStateChange transition, GstStateChangeReturn result)
{
  optional_init ();
  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      if (GST_IS_PIPELINE (element)) {
        const gchar *path = g_getenv ("GST_DEBUG_DUMP_TRACE_DIR");
        const gchar *name = g_getenv ("GST_DEBUG_DUMP_TRACE_FILENAME");
        gchar *filename = g_strdup_printf ("%s/%s.gsttrace", path ? path : ".", name ? name : GST_OBJECT_NAME (element));
        gst_trace_dump_pipeline_to_file (current_trace, (GstPipeline *)element, filename);
        g_free (filename);
      }
      break;
      
    default:
      break;
      
  }

}

static void
gst_instruments_tracer_init (GstInstrumentsTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);
  gst_tracing_register_hook (tracer, "element-change-state-pre",
    G_CALLBACK (do_element_change_state_pre));
  gst_tracing_register_hook (tracer, "element-change-state-post",
    G_CALLBACK (do_element_change_state_post));
  gst_tracing_register_hook (tracer, "pad-push-pre",
    G_CALLBACK (do_push_buffer_pre));
  gst_tracing_register_hook (tracer, "pad-push-list-pre",
    G_CALLBACK (do_push_buffer_list_pre));
  gst_tracing_register_hook (tracer, "pad-push-post",
    G_CALLBACK (do_push_buffer_post));
  gst_tracing_register_hook (tracer, "pad-push-list-post",
    G_CALLBACK (do_push_buffer_list_post));
  gst_tracing_register_hook (tracer, "pad-pull-range-pre",
    G_CALLBACK (do_pull_range_pre));
  gst_tracing_register_hook (tracer, "pad-pull-range-post",
    G_CALLBACK (do_pull_range_post));
  gst_tracing_register_hook (tracer, "pad-push-event-pre",
    G_CALLBACK (do_push_event_pre));
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_tracer_register (plugin, "instruments", gst_instruments_tracer_get_type ()))
    return FALSE;
  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, instruments,
    "gst-instruments tracer", plugin_init, VERSION, "LGPL",
    "instruments", "GST_PACKAGE_ORIGIN");
