//
//  spycommon.c
//  instruments
//
//  Created by Cyril on 10/7/16.
//  Copyright © 2016 §	. All rights reserved.
//

#include "spycommon.h"

THREAD
mach_thread_self()
{
  return 0;
}

guint64
get_cpu_time (THREAD thread) {
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
  if (clock_gettime (CLOCK_THREAD_CPUTIME_ID, &ts))
    return 0;
  
  return ts.tv_sec * GST_SECOND + ts.tv_nsec;
#endif
}

GstClockTime
current_monotonic_time ()
{
  static GstClockTime startup_time = GST_CLOCK_TIME_NONE;
  
#ifdef __MACH__ // Mach does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t ts;
  host_get_clock_service (mach_host_self (), SYSTEM_CLOCK, &cclock);
  clock_get_time (cclock, &ts);
  mach_port_deallocate (mach_task_self (), cclock);
#else
  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
#endif
  
  GstClockTime current_time = ts.tv_sec * GST_SECOND + ts.tv_nsec;
  
  if (GST_CLOCK_TIME_NONE == startup_time) {
    startup_time = current_time;
  }
  
  return current_time - startup_time;
}

gpointer
trace_heir (GstElement *element)
{
  GstObject *parent = NULL;
  
  if (element == NULL) {
    return NULL;
  }
  
  for (parent = GST_OBJECT (element); GST_OBJECT_PARENT (parent) != NULL; parent = GST_OBJECT_PARENT (parent));
  
  return parent;
}

gpointer
get_downstack_element (gpointer pad)
{
  gpointer element = pad;
  do {
    gpointer peer = GST_PAD_PEER (element);
    if (peer) {
      element = GST_PAD_PARENT (peer);
    } else {
      return NULL;
    }
  }
  while (!GST_IS_ELEMENT (element));
  
  return element;
}

GHashTable *pipeline_by_element = NULL;

void
dump_hierarchy_info_if_needed (GstTrace *trace, GstPipeline *pipeline, GstElement *new_element)
{
  if (pipeline_by_element == NULL) {
    pipeline_by_element = g_hash_table_new (g_direct_hash, g_direct_equal);
  } else if (g_hash_table_lookup (pipeline_by_element, new_element)) {
    return;
  }
  
  if (new_element) {
    g_hash_table_insert (pipeline_by_element, new_element, pipeline);
  }
  
  if (!g_hash_table_lookup (pipeline_by_element, pipeline)) {
    GstTraceElementDiscoveredEntry *entry = gst_trace_element_discoved_entry_new ();
    gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
    gst_trace_element_discoved_entry_init_set_element (entry, (GstElement *)pipeline);
    gst_trace_add_entry (trace, pipeline, (GstTraceEntry *)entry);
    
    g_hash_table_insert (pipeline_by_element, pipeline, pipeline);
  }
  
  if (pipeline == NULL) {
    return;
  }
  
  GstIterator *it = gst_bin_iterate_recurse (GST_BIN (pipeline));
  GValue item = G_VALUE_INIT;
  gboolean done = FALSE;
  while (!done) {
    switch (gst_iterator_next (it, &item)) {
      case GST_ITERATOR_OK:
      {
        GstElement *element = g_value_get_object (&item);
        
        GstTraceElementDiscoveredEntry *entry = gst_trace_element_discoved_entry_new ();
        gst_trace_entry_set_pipeline ((GstTraceEntry *)entry, pipeline);
        gst_trace_element_discoved_entry_init_set_element (entry, element);
        gst_trace_add_entry (trace, pipeline, (GstTraceEntry *)entry);
        
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
