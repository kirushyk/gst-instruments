/* GStreamer Instruments
 * Copyright (C) 2015 Kyrylo Polezhaiev <kirushyk@gmail.com>
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

#ifndef __GSTELEMENTHEADSTONE_H__
#define __GSTELEMENTHEADSTONE_H__

#include <glib.h>
#include <gst/gst.h>

typedef struct GstElementHeadstone
{
  gpointer                              identifier;
  GString                              *name;
  GString                              *type_name;
  
  struct GstElementHeadstone           *parent;
  GList                                *children;
  gint                                  nesting_level;
  
  guint64                               bytes_sent;
  guint64                               bytes_received;
  GHashTable                           *pads;
  
  gboolean                              is_subtopstack;
  guint64                               total_cpu_time;
  float                                 cpu_load;
  
} GstElementHeadstone;

void                gst_element_headstone_add_child        (GstElementHeadstone     *parent,
                                                            GstElementHeadstone     *child);

guint64             gst_element_headstone_get_nested_time  (GstElementHeadstone     *element);

gfloat              gst_element_headstone_get_nested_load  (GstElementHeadstone     *element);

#endif
