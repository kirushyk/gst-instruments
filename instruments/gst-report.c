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
gboolean show_memory = FALSE, show_types = FALSE, hierarchy = FALSE, nested_time = FALSE, dot = FALSE;

static GOptionEntry entries[] = {
  { "from",      0, 0, G_OPTION_ARG_DOUBLE, &from,        "Do not take events before timestamp into account", NULL },
  { "till",      0, 0, G_OPTION_ARG_DOUBLE, &till,        "Do not take events after timestamp into account",  NULL },
  { "memory",    0, 0, G_OPTION_ARG_NONE,   &show_memory, "Show memory usage",                                NULL },
  { "types",     0, 0, G_OPTION_ARG_NONE,   &show_types,  "Show types of elements",                           NULL },
  { "hierarchy", 0, 0, G_OPTION_ARG_NONE,   &hierarchy,   "Show hierarchy of elements",                       NULL },
  { "nested",    0, 0, G_OPTION_ARG_NONE,   &nested_time, "Include time spent by nested elements",            NULL },
  { "dot",       0, 0, G_OPTION_ARG_NONE,   &dot,         "Output in DOT format",                             NULL },
  { NULL }
};

void
render_space (gint space)
{
  int j;
  for (j = 0; j < space; j++) {
    g_print (" ");
  }
}

void
render_headstone (GstGraveyard *graveyard, GstElementHeadstone *element, gsize max_length, gsize max_type_name_length)
{
  gint j;
   
  guint64 total_time = nested_time ? gst_element_headstone_get_nested_time (element) : element->total_time;
  gsize space = element->nesting * 2;
  
  if (hierarchy) {
    g_print ("\n");
    render_space (space);
  }
  
  if (dot) {
    g_print ("%sgraph cluster_eg%p {\n", element->parent == NULL ? "di" : "sub", element->identifier);
  }
  
  if (dot && (element->parent == NULL)) {
    space = 2 * (element->nesting + 1);
    render_space (space);
    g_print ("rankdir=LR;\n");
    render_space (space);
    g_print ("labelloc=t;\n");
    render_space (space);
    g_print ("nodesep=.1;\n");
    render_space (space);
    g_print ("ranksep=.2;\n");
    render_space (space);
    g_print ("fontname=\"Avenir Next\";\n");
    render_space (space);
    g_print ("fontsize=\"14\";\n");
    render_space (space);
    g_print ("style=\"filled,rounded\";\n");
    render_space (space);
    g_print ("color=black;\n");
    render_space (space);
    g_print ("label=\"pipeline0\";\n");
    render_space (space);
    g_print ("node [style=\"filled,rounded\", shape=box, fontsize=\"14\", fontname=\"Avenir Next\", margin=\"0.2,0.2\"];\n");
    render_space (space);
    g_print ("edge [labelfontsize=\"14\", fontsize=\"14\", fontname=\"Avenir Next\"];\n");
  }
  
  if (dot) {
    space = 2 * (element->nesting + 1);
    render_space (space);
    g_print ("fillcolor=\"#ffffff\";\n");
    if (element->bytes_received) {
      render_space (space);
      g_print ("subgraph cluster_eg%p_sink {\n", element->identifier);
      space++;
      render_space (space);
      g_print ("style=\"invis\";");
      render_space (space);
      g_print ("label=\"\";");
      render_space (space);
      g_print ("en%p_sink [label=\"sink\", color=black, fillcolor=\"#ffffff\"];\n", element->identifier);
      space--;
      render_space (space);
      g_print ("}\n");
    }
    if (element->bytes_sent) {
      render_space (space);
      g_print ("subgraph cluster_eg%p_src {\n", element->identifier);
      space++;
      render_space (space);
      g_print ("style=\"invis\";");
      render_space (space);
      g_print ("label=\"\";");
      render_space (space);
      g_print ("en%p_src [label=\"src\", color=black, fillcolor=\"#ffffff\"];\n", element->identifier);
      render_space (space);
      g_print ("en%p_src [label=\"src\", color=black, fillcolor=\"#ffffff\"];\n", element->identifier);
      space--;
      render_space (space);
      g_print ("}\n");
    }
    if (element->bytes_sent && element->bytes_received) {
      g_print ("en%p_sink -> en%p_src [style=\"invis\"];\n", element->identifier, element->identifier);
    }
    render_space (space);
  }

  g_print (dot ? "label=\"%s\";" : "%s", element->name->str);
  
  space = max_length - element->name->len - ((hierarchy && !dot) ? element->nesting : 0);
  for (j = 0; j < space; j++)
    g_print (" ");
  
  if (!dot && show_types) {
    if (element->type_name) {
      g_print (" %s", element->type_name->str);
      space = max_type_name_length - element->type_name->len;
    } else {
      g_print (" ?");
      space = max_type_name_length - 1;
    }
    for (j = 0; j < space; j++) {
      g_print (" ");
    }
  }
  
  if (!dot) {
    gchar *time_string = format_time (total_time);
    g_print (" %5.1f  %8s", total_time * 100.f / graveyard->total_time, time_string);
    g_free (time_string);
  }
  
  if (!dot && show_memory) {
    gchar *memory_received_size_string = format_memory_size (element->bytes_received);
    gchar *memory_sent_size_string = format_memory_size (element->bytes_sent);
    g_print (" %9s %9s", memory_received_size_string, memory_sent_size_string);
    g_free (memory_sent_size_string);
    g_free (memory_received_size_string);
  }
  g_print ("\n");
}

void
render_container (GstGraveyard *graveyard, GstElementHeadstone *element, gsize max_length, gsize max_type_name_length)
{
  GList *child;
  render_headstone (graveyard, element, max_length, max_type_name_length);
  for (child = element->children; child != NULL; child = child->next) {
    render_container (graveyard, child->data, max_length, max_type_name_length);
  }
  if (dot) {
    render_space (element->nesting * 2);
    g_print ("}\n");
  }
}

gint
main (gint argc, gchar *argv[])
{
  gint i, j;
  gsize max_length = 0, max_type_name_length = 0;
  GError *error = NULL;
  GOptionContext *option_context;
  
  g_set_prgname ("gst-report-1.0");
  g_set_application_name ("GStreamer Report Tool");
  
  option_context = g_option_context_new (NULL);
  g_option_context_add_main_entries (option_context, entries, NULL);
  g_option_context_set_summary (option_context, g_get_application_name ());
  if (!g_option_context_parse (option_context, &argc, &argv, &error)) {
    g_print ("could not parse arguments: %s\n", error->message);
    g_print ("%s", g_option_context_get_help (option_context, TRUE, NULL));
    return 1;
  }
  g_option_context_free (option_context);
  
  if (from > 0)
    from_ns = from * GST_SECOND;
  if (till > 0)
    till_ns = till * GST_SECOND;
  
  if (dot) {
    hierarchy = TRUE;
    show_types = TRUE;
    show_memory = TRUE;
  }
  
  GstGraveyard *graveyard = gst_graveyard_new_from_trace (argv[argc - 1], from_ns, till_ns);
  if (graveyard == NULL)
    return 3;
  
  for (i = 0; i < graveyard->elements_sorted->len; i++) {
    GstElementHeadstone *element = g_array_index (graveyard->elements_sorted, GstElementHeadstone *, i);
    if (element->name->len + element->nesting > max_length) {
      max_length = element->name->len + element->nesting;
    }
    if (element->type_name && element->type_name->len > max_type_name_length) {
      max_type_name_length = element->type_name->len;
    }
  }
  
  if (!dot) {
    g_print ("ELEMENT");
    gsize space = max_length - 7; // sizeof "ELEMENT"
    for (j = 0; j < space; j++)
      g_print (" ");
    
    if (show_types) {
      g_print (" TYPE");
      space = max_type_name_length - 4; // sizeof "ELEMENT"
      for (j = 0; j < space; j++)
        g_print (" ");
    }
    g_print ("  %%CPU   TIME");
    
    if (show_memory)
      g_print ("     INPUT     OUTPUT");
    
    g_print ("\n");
  }
  
  for (i = 0; i < graveyard->elements_sorted->len; i++) {
    GstElementHeadstone *element = g_array_index (graveyard->elements_sorted, GstElementHeadstone *, i);
    if (hierarchy) {
      if (element->parent == NULL) {
        render_container (graveyard, element, max_length, max_type_name_length);
      }
    } else {
      render_headstone (graveyard, element, max_length, max_type_name_length);
    }
  }
  
  gst_graveyard_free (graveyard);
  
  return 0;
}
