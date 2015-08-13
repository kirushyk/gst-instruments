#include "intercept.h"
#include <dlfcn.h>
#include <stdio.h>
#include <gst/gst.h>
#include <gst/gstpad.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h> 
#include <stdlib.h>

#include <signal.h>
#include <time.h>

#include <mach/mach_init.h>
#include <mach/thread_act.h>
#include <mach/mach_port.h>

typedef uint64_t u64;

static u64 get_cpu_time(thread_port_t thread) {
  mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
  thread_basic_info_data_t info;
  
  int kr = thread_info(thread, THREAD_BASIC_INFO, (thread_info_t) &info, &count);
  if (kr != KERN_SUCCESS) {
    return 0;
  }
  
  return (u64) info.user_time.seconds * (u64) 1e6 +
  (u64) info.user_time.microseconds +
  (u64) info.system_time.seconds * (u64) 1e6 +
  (u64) info.system_time.microseconds;
}

static u64 get_thread_id(thread_port_t thread) {
  mach_msg_type_number_t count = THREAD_IDENTIFIER_INFO_COUNT;
  thread_identifier_info_data_t info;
  
  int kr = thread_info(thread, THREAD_IDENTIFIER_INFO, (thread_info_t) &info, &count);
  if (kr != KERN_SUCCESS) {
    return 0;
  }
  
  return (u64) info.thread_id;
}

void *libgstreamer = NULL;

GstFlowReturn (*gst_pad_push_orig) (GstPad *pad, GstBuffer *buffer) = NULL;
GstFlowReturn (*gst_pad_push_list_orig) (GstPad *pad, GstBufferList *list) = NULL;
GstFlowReturn (*gst_pad_pull_range_orig) (GstPad *pad, guint64 offset, guint size, GstBuffer **buffer) = NULL;

void
deinit ()
{
  dlclose(libgstreamer);
}

void *
get_libgstreamer ()
{
  atexit(deinit);
  
  if (libgstreamer == NULL)
    libgstreamer = dlopen("libgstreamer-1.0.dylib", RTLD_NOW);
  
  return libgstreamer;
}

gpointer get_downstack_element(gpointer pad)
{
  gpointer element = pad;
  do
  {
    element = GST_PAD_PARENT (GST_PAD_PEER (element));
  }
  while (!GST_IS_ELEMENT(element));
  
  return element;
}

GstFlowReturn
gst_pad_push (GstPad *pad, GstBuffer *buffer)
{
  GstFlowReturn result;
  
  if (gst_pad_push_orig == NULL)
  {
    gst_pad_push_orig = dlsym(get_libgstreamer(), "gst_pad_push");
    
    if (gst_pad_push_orig == NULL)
    {
      fprintf (stderr, "can not link to gst_pad_push\n");
      return GST_FLOW_CUSTOM_ERROR;
    }
    else
    {
      fprintf (stderr, "gst_pad_push linked: %p\n", gst_pad_push_orig);
    }
  }
  
  thread_port_t thread = mach_thread_self();
  
  gpointer *element = get_downstack_element(pad);
  
  fprintf (stderr, "{%llu} %s: %p {\n", get_thread_id(thread), GST_ELEMENT_NAME (element), element);

  u64 start = get_cpu_time(thread);
  
  result = gst_pad_push_orig (pad, buffer);
  
  u64 duration = get_cpu_time(thread) - start;
  mach_port_deallocate(mach_task_self(), thread);
  
  fprintf (stderr, "} %s: %p (%llu µs)\n", GST_ELEMENT_NAME (element), element, duration);
  
  return result;
}

GstFlowReturn
gst_pad_push_list (GstPad *pad, GstBufferList *list)
{
  GstFlowReturn result;
  
  if (gst_pad_push_list_orig == NULL)
  {
    gst_pad_push_list_orig = dlsym(get_libgstreamer(), "gst_pad_push_list");
    
    if (gst_pad_push_list_orig == NULL)
    {
      fprintf (stderr, "can not link to gst_pad_push_list\n");
      return GST_FLOW_CUSTOM_ERROR;
    }
    else
    {
      fprintf (stderr, "gst_pad_push_list linked: %p\n", gst_pad_push_orig);
    }
  }
  
  thread_port_t thread = mach_thread_self();
  
  gpointer *element = get_downstack_element(pad);
  
  fprintf (stderr, "{%llu} %s: %p {\n", get_thread_id(thread), GST_ELEMENT_NAME (element), element);
  
  u64 start = get_cpu_time(thread);

  result = gst_pad_push_list_orig (pad, list);

  u64 duration = get_cpu_time(thread) - start;
  mach_port_deallocate(mach_task_self(), thread);

  fprintf (stderr, "} %s: %p (%llu µs)\n", GST_ELEMENT_NAME (element), element, duration);
  
  return result;
}

GstFlowReturn
gst_pad_pull_range (GstPad *pad, guint64 offset, guint size, GstBuffer **buffer)
{
  GstFlowReturn result;
  
  if (gst_pad_pull_range_orig == NULL)
  {
    gst_pad_pull_range_orig = dlsym(get_libgstreamer(), "gst_pad_pull_range");

    if (gst_pad_pull_range_orig == NULL)
    {
      fprintf (stderr, "can not link to gst_pad_pull_range\n");
      return GST_FLOW_CUSTOM_ERROR;
    }
    else
    {
      fprintf (stderr, "gst_pad_pull_range linked: %p\n", gst_pad_pull_range_orig);
    }
  }
  
  thread_port_t thread = mach_thread_self();
  
  gpointer *element = get_downstack_element(pad);
  
  fprintf (stderr, "{%llu} %s: %p {\n", get_thread_id(thread), GST_ELEMENT_NAME (element), element);

  u64 start = get_cpu_time(thread);

  result = gst_pad_pull_range_orig (pad, offset, size, buffer);

  u64 duration = get_cpu_time(thread) - start;
  mach_port_deallocate(mach_task_self(), thread);
  
  fprintf (stderr, "} %s: %p (%llu µs)\n", GST_ELEMENT_NAME (element), element, duration);

  return result;
}
