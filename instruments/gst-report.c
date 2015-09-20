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

#include "formatters.h"
#include "gst-trace-parser.h"

gint
main (gint argc, gchar *argv[])
{
  gint i, j;
  gsize max_length = 0;
  
  g_set_prgname ("gst-report-1.0");
  g_set_application_name ("GStreamer Report Tool");
  
  if (argc != 2)
    return 1;
  
  GstGraveyard *graveyard = gst_graveyard_new_from_trace (argv[1]);
  if (graveyard == NULL)
    return 2;
  
  for (i = 0; i < graveyard->elements_sorted->len; i++)
  {
    GstElementHeadstone *element = g_array_index (graveyard->elements_sorted, GstElementHeadstone *, i);
    if (element->name->len > max_length)
      max_length = element->name->len;
  }
  
  g_print ("ELEMENT");
  gsize space = max_length - 7; // sizeof "ELEMENT"
  for (j = 0; j < space; j++)
    g_print (" ");
  g_print ("CPU   TIME\n");

  for (i = 0; i < graveyard->elements_sorted->len; i++)
  {
    GstElementHeadstone *element = g_array_index (graveyard->elements_sorted, GstElementHeadstone *, i);
    
    gchar *time_string = format_time (element->total_time);
    g_print ("%s", element->name->str);
    gsize space = max_length - element->name->len;
    for (j = 0; j < space; j++)
      g_print (" ");
    g_print (" %5.1f%% %s", element->total_time * 100.f / graveyard->total_time, time_string);
    g_free (time_string);
    
    if (FALSE)
    {
      gchar *memory_received_size_string = format_memory_size (element->data_received);
      gchar *memory_sent_size_string = format_memory_size (element->data_sent);
      g_print (" got %s sent %s", memory_received_size_string, memory_sent_size_string);
      g_free (memory_sent_size_string);
      g_free (memory_received_size_string);
    }
    g_print ("\n");
  }
  gst_graveyard_free (graveyard);
  
  return 0;
}
