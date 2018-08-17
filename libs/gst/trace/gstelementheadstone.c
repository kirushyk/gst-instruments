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

#include "gstelementheadstone.h"

void
gst_element_headstone_add_child (GstElementHeadstone *parent, GstElementHeadstone *child)
{
  GList *iterator;
  child->parent = parent;
  for (iterator = parent->children; iterator != NULL; iterator = iterator->next) {
    if (iterator->data == child) {
      return;
    }
  }
  parent->children = g_list_prepend (parent->children, child);
}

guint64
gst_element_headstone_get_nested_time (GstElementHeadstone *element)
{
  GList *child;
  guint64 result = element->total_cpu_time;
  for (child = element->children; child != NULL; child = child->next) {
    result += gst_element_headstone_get_nested_time (child->data);
  }
  return result;
}

gfloat
gst_element_headstone_get_nested_load (GstElementHeadstone *element)
{
  GList *child;
  gfloat result = element->cpu_load;
  for (child = element->children; child != NULL; child = child->next) {
    result += gst_element_headstone_get_nested_load (child->data);
  }
  return result;
}
