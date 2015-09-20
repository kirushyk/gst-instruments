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

gdouble from = 0, till = 0;
GstClockTime from_ns = GST_CLOCK_TIME_NONE, till_ns = GST_CLOCK_TIME_NONE;
gboolean show_memory;

static GOptionEntry entries[] =
{
  { "from",   0, 0, G_OPTION_ARG_DOUBLE, &from,        "Do not take events before timestamp into account", NULL },
  { "till",   0, 0, G_OPTION_ARG_DOUBLE, &till,        "Do not take events after timestamp into account", NULL },
  { "memory", 0, 0, G_OPTION_ARG_NONE,   &show_memory, "Show memory usage", NULL },
  { NULL }
};

gint
main (gint argc, gchar *argv[])
{
  gint i, j;
  gsize max_length = 0;
  GError *error = NULL;
  GOptionContext *option_context;
  
  g_set_prgname ("gst-report-1.0");
  g_set_application_name ("GStreamer Report Tool");
  
  option_context = g_option_context_new (NULL);
  g_option_context_add_main_entries (option_context, entries, NULL);
  g_option_context_set_summary (option_context, g_get_application_name ());
  if (!g_option_context_parse (option_context, &argc, &argv, &error))
  {
    g_print ("could not parse arguments: %s\n", error->message);
    g_print ("%s", g_option_context_get_help (option_context, TRUE, NULL));
    return 1;
  }
  g_option_context_free (option_context);
  
  if (from > 0)
    from_ns = from * GST_SECOND;
  if (till > 0)
    till_ns = till * GST_SECOND;
  
  GstGraveyard *graveyard = gst_graveyard_new_from_trace (argv[argc - 1], from_ns, till_ns);
  if (graveyard == NULL)
    return 3;
  
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
  g_print ("  %%CPU    TIME");
  
  if (show_memory)
    g_print ("    INPUT     OUTPUT");
  
  g_print ("\n");
  
  for (i = 0; i < graveyard->elements_sorted->len; i++)
  {
    GstElementHeadstone *element = g_array_index (graveyard->elements_sorted, GstElementHeadstone *, i);
    
    gchar *time_string = format_time (element->total_time);
    g_print ("%s", element->name->str);
    gsize space = max_length - element->name->len;
    for (j = 0; j < space; j++)
      g_print (" ");
    g_print (" %5.1f  %8s", element->total_time * 100.f / graveyard->total_time, time_string);
    g_free (time_string);
    
    if (show_memory)
    {
      gchar *memory_received_size_string = format_memory_size (element->bytes_received);
      gchar *memory_sent_size_string = format_memory_size (element->bytes_sent);
      g_print (" %9s %9s", memory_received_size_string, memory_sent_size_string);
      g_free (memory_sent_size_string);
      g_free (memory_received_size_string);
    }
    g_print ("\n");
  }
  gst_graveyard_free (graveyard);
  
  return 0;
}
