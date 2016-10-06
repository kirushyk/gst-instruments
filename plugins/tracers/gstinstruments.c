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
#include "gstinstruments.h"

GST_DEBUG_CATEGORY_STATIC (gst_instruments_debug);
#define GST_CAT_DEFAULT gst_instruments_debug

#define _do_init \
GST_DEBUG_CATEGORY_INIT (gst_instruments_debug, "instruments", 0, "instruments tracer");
#define gst_instruments_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstInstrumentsTracer, gst_instruments_tracer, GST_TYPE_TRACER,
                         _do_init);




static void
gst_instruments_tracer_class_init (GstInstrumentsTracerClass * klass)
{
  
}

static void
gst_instruments_tracer_init (GstInstrumentsTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);
//  gst_tracing_register_hook (tracer, "pad-push-pre",
//                             G_CALLBACK (do_push_buffer_pre));
//  gst_tracing_register_hook (tracer, "pad-push-list-pre",
//                             G_CALLBACK (do_push_buffer_pre));
//  gst_tracing_register_hook (tracer, "pad-push-post",
//                             G_CALLBACK (do_push_buffer_post));
//  gst_tracing_register_hook (tracer, "pad-push-list-post",
//                             G_CALLBACK (do_push_buffer_post));
//  gst_tracing_register_hook (tracer, "pad-pull-range-pre",
//                             G_CALLBACK (do_pull_range_pre));
//  gst_tracing_register_hook (tracer, "pad-pull-range-post",
//                             G_CALLBACK (do_pull_range_post));
//  gst_tracing_register_hook (tracer, "pad-push-event-pre",
//                             G_CALLBACK (do_push_event_pre));
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

