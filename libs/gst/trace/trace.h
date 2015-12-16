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

#ifndef __GST_INTERCEPT_TRACE_H__
#define __GST_INTERCEPT_TRACE_H__

#include <glib.h>
#include <gst/gst.h>
#include "entry.h"

G_BEGIN_DECLS

typedef struct GstTrace GstTrace;

GstTrace *          gst_trace_new                          (void);

void                gst_trace_add_entry                    (GstPipeline        *pipeline,
                                                            GstTraceEntry      *entry);


void                gst_trace_dump_pipeline_to_file        (GstTrace           *trace,
                                                            GstElement         *pipeline,
                                                            const gchar        *filename);

void                gst_trace_free                         (GstTrace           *trace);

G_END_DECLS

#endif
