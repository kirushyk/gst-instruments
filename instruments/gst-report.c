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

#include "gst-trace-parser.h"

int
main (int argc, char *argv[])
{
  int i;
  
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
    g_print ("%s: %5.3f ms", element->name->str, element->total_time * 0.000001);
    if (FALSE)
      g_print (" %" G_GUINT64_FORMAT " B %" G_GUINT64_FORMAT " B", element->data_received, element->data_sent);
    g_print ("\n");
  }
  gst_graveyard_free (graveyard);
  
  return 0;
}
