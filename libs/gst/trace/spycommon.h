//
//  spycommon.h
//  instruments
//
//  Created by Cyril on 10/7/16.
//  Copyright © 2016 §	. All rights reserved.
//

#ifndef spycommon_h
#define spycommon_h

#include <gst/gst.h>
#include <gst/gstpad.h>
#include "gsttrace.h"

#if __MACH__
# include <mach/mach_init.h>
# include <mach/thread_act.h>
# include <mach/mach_port.h>
# include <mach/clock.h>
# include <mach/mach.h>
# define THREAD thread_port_t
#else
# define THREAD int
THREAD             mach_thread_self              (void);
#endif

typedef struct ListInfo
{
  guint64 size;
  guint   buffers_count;
} ListInfo;

guint64            get_cpu_time                  (THREAD       thread);

GstClockTime       current_monotonic_time        (void);

GstPad *           get_source_pad                (GstPad      *pad);

gpointer           trace_heir                    (GstElement  *element);

gpointer           get_downstack_element         (gpointer     pad);

void               dump_hierarchy_info_if_needed (GstTrace    *trace,
                                                  GstPipeline *pipeline,
                                                  GstElement  *new_element);

gboolean           for_each_buffer               (GstBuffer  **buffer,
                                                  guint        idx,
                                                  gpointer     user_data);

extern GHashTable *pipeline_by_element;

#endif /* spycommon_h */
