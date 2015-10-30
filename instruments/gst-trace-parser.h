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

typedef struct GstPadHeadstone
{
  gpointer                              identifier;
  /// @note: We do not care about pad's names
  gpointer                              parent_element;
  
  gpointer                              peer;
  gpointer                              peer_element;
  
  guint64                               bytes;
  GstPadMode                            mode;
  GstPadDirection                       direction;
  
} GstPadHeadstone;

typedef struct GstElementHeadstone
{
  gpointer                              identifier;
  GString                              *name;
  GString                              *type_name;
  
  struct GstElementHeadstone           *parent;
  GList                                *children;
  gint                                  nesting;
  
  guint64                               bytes_sent;
  guint64                               bytes_received;
  GList                                *from;
  GList                                *to;
  GHashTable                           *pads;
  
  gboolean                              is_subtopstack;
  guint64                               total_time;
  float                                 cpu_load;

} GstElementHeadstone;

typedef struct GstGraveyard
{
  GHashTable                           *tasks;
  GHashTable                           *elements;
  GArray                               *elements_sorted;
  GstClockTime                          total_time;
  
  GstClockTime                          from;
  GstClockTime                          till;

  GstClockTime                          duration;
} GstGraveyard;

GstGraveyard *      gst_graveyard_new_from_trace           (const char              *filename,
                                                            GstClockTime             from,
                                                            GstClockTime             till,
                                                            gboolean                 query_duration_only);

void                gst_element_headstone_add_child        (GstElementHeadstone     *parent,
                                                            GstElementHeadstone     *child);

guint64             gst_element_headstone_get_nested_time  (GstElementHeadstone     *element);

gfloat              gst_element_headstone_get_nested_load  (GstElementHeadstone     *element);

void                gst_graveyard_free                     (GstGraveyard            *graveyard);

#endif
