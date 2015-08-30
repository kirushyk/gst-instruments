#include "intercept.h"
#include <dlfcn.h>
#include <stdio.h>
#include <glib.h>
#include <gst/gst.h>
#include <gst/gstpad.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h> 
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "trace.h"

#if __APPLE__
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#include <mach/mach_port.h>
#define THREAD thread_port_t
#else
#include <signal.h>
#include <time.h>
#define THREAD int
THREAD mach_thread_self() {return 0;}
#endif

#define LGI_ELEMENT_NAME(e) ((e != NULL) ? GST_ELEMENT_NAME((e)) : "0")

static guint64 get_cpu_time (THREAD thread) {
#if __APPLE__
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

void *libgstreamer = NULL;

GstStateChangeReturn (*gst_element_change_state_orig) (GstElement * element, GstStateChange transition) = NULL;
GstFlowReturn (*gst_pad_push_orig) (GstPad *pad, GstBuffer *buffer) = NULL;
GstFlowReturn (*gst_pad_push_list_orig) (GstPad *pad, GstBufferList *list) = NULL;
GstFlowReturn (*gst_pad_pull_range_orig) (GstPad *pad, guint64 offset, guint size, GstBuffer **buffer) = NULL;
gboolean (*gst_pad_push_event_orig) (GstPad *pad, GstEvent *event) = NULL;

void
deinit ()
{
  const gchar * output_filename = g_getenv ("GST_INTERCEPT_OUTPUT_FILE");
  if (output_filename)
  {
    gst_pipeline_dump_to_file(NULL, output_filename);
  }
  
  dlclose (libgstreamer);
}

void *
get_libgstreamer ()
{
  atexit (deinit);
  
  if (libgstreamer == NULL)
  {
#if __APPLE__
    libgstreamer = dlopen ("libgstreamer-1.0.dylib", RTLD_NOW);
#else
    libgstreamer = dlopen ("/usr/local/lib/libgstreamer-1.0.so", RTLD_NOW);
#endif
  }
  
  trace_init();
  
  return libgstreamer;
}

gpointer trace_heir (GstElement * element)
{
  GstObject *parent = NULL;
  for (parent = GST_OBJECT(element); GST_OBJECT_PARENT(parent) != NULL; parent = GST_OBJECT_PARENT(parent));
  if (GST_IS_PIPELINE(parent))
  {
    return parent;
    // trace_add_entry(parent, "heir %s %p %s %p", LGI_ELEMENT_NAME(element), element, LGI_ELEMENT_NAME(parent), parent);
  }
  return NULL;
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

GstStateChangeReturn
gst_element_change_state (GstElement * element, GstStateChange transition)
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
#if __APPLE__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME (element), element, end, duration));
  
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
  
  gpointer element_from = gst_pad_get_parent_element (pad);
  gpointer element = get_downstack_element (pad);
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  trace_add_entry (pipeline, g_strdup_printf ("element-entered %p %s %p %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element_from), element_from, LGI_ELEMENT_NAME(element), element, start));
  
  result = gst_pad_push_orig (pad, buffer);
  
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __APPLE__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME (element), element, end, duration));

  return result;
}

GstFlowReturn
gst_pad_push_list (GstPad *pad, GstBufferList *list)
{
  GstFlowReturn result;
  GstPipeline *pipeline = NULL;
  
  if (gst_pad_push_list_orig == NULL)
  {
    gst_pad_push_list_orig = dlsym (get_libgstreamer (), "gst_pad_push_list");
    
    if (gst_pad_push_list_orig == NULL)
    {
      GST_ERROR ("can not link to gst_pad_push_list\n");
      return GST_FLOW_CUSTOM_ERROR;
    }
    else
    {
      GST_INFO ("gst_pad_push_list linked: %p\n", gst_pad_push_orig);
    }
  }
  
  THREAD thread = mach_thread_self ();
  
  gpointer element_from = gst_pad_get_parent_element (pad);
  gpointer element = get_downstack_element (pad);
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  trace_add_entry (pipeline, g_strdup_printf ("element-entered %p %s %p %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element_from), element_from, LGI_ELEMENT_NAME(element), element, start));

  result = gst_pad_push_list_orig (pad, list);
  
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __APPLE__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME (element), element, end, duration));
  
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
    
    if (gst_pad_push_event_orig == NULL)
    {
      GST_ERROR ("can not link to gst_pad_push_event\n");
      return FALSE;
    }
    else
    {
      GST_INFO ("gst_pad_push_event linked: %p\n", gst_pad_push_event_orig);
    }
  }
  
  THREAD thread = mach_thread_self ();
  
  gpointer element_from = gst_pad_get_parent_element (pad);
  gpointer element = get_downstack_element (pad);
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  if (element_from && element)
  {
      trace_add_entry (pipeline, g_strdup_printf ("element-entered %p %s %p %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element_from), element_from, LGI_ELEMENT_NAME(element), element, start));
  }
  
  result = gst_pad_push_event_orig (pad, event);
  
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __APPLE__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  if (element_from && element)
  {
      trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME (element), element, end, duration));
  }
  
  return result;
}

GstFlowReturn
gst_pad_pull_range (GstPad *pad, guint64 offset, guint size, GstBuffer **buffer)
{
  GstFlowReturn result;
  GstPipeline *pipeline = NULL;
  
  if (gst_pad_pull_range_orig == NULL)
  {
    gst_pad_pull_range_orig = dlsym (get_libgstreamer (), "gst_pad_pull_range");

    if (gst_pad_pull_range_orig == NULL)
    {
      GST_ERROR ("can not link to gst_pad_pull_range\n");
      return GST_FLOW_CUSTOM_ERROR;
    }
    else
    {
      GST_INFO ("gst_pad_pull_range linked: %p\n", gst_pad_pull_range_orig);
    }
  }
  
  THREAD thread = mach_thread_self();
  
  gpointer element_from = gst_pad_get_parent_element (pad);
  gpointer element = get_downstack_element (pad);
  
  pipeline = trace_heir (element);
  
  guint64 start = get_cpu_time (thread);
  
  trace_add_entry (pipeline, g_strdup_printf ("element-entered %p %s %p %s %p %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME(element_from), element_from, LGI_ELEMENT_NAME(element), element, start));

  result = gst_pad_pull_range_orig (pad, offset, size, buffer);
  
  guint64 end = get_cpu_time (thread);
  guint64 duration = end - start;
#if __APPLE__
  mach_port_deallocate (mach_task_self (), thread);
#endif
  
  trace_add_entry (pipeline, g_strdup_printf ("element-exited %p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT, g_thread_self (), LGI_ELEMENT_NAME (element), element, end, duration));

  return result;
}
