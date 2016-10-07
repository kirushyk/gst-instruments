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

#ifndef __GST_TRACE_PARSER_H__
#define __GST_TRACE_PARSER_H__

#include <glib.h>
#include <gst/gst.h>

typedef struct GstGraveyard
{
  int dsec;
  GHashTable                           *tasks;
  GHashTable                           *elements;
  GArray                               *elements_sorted;
  GstClockTime                          total_cpu_time;
  
  GstClockTime                          from;
  GstClockTime                          till;

  GstClockTime                          duration;
} GstGraveyard;

GstGraveyard *      gst_graveyard_new_from_trace           (const char              *filename,
                                                            GstClockTime             from,
                                                            GstClockTime             till,
                                                            gboolean                 query_duration_only);

void                gst_graveyard_free                     (GstGraveyard            *graveyard);

#endif
