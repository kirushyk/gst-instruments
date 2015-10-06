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
#include "../configure-static.h"
#include "gst-trace-parser.h"

gdouble from = 0, till = 0;
GstClockTime from_ns = GST_CLOCK_TIME_NONE, till_ns = GST_CLOCK_TIME_NONE;
gboolean show_memory = FALSE, show_types = FALSE, hierarchy = FALSE, nested_time = FALSE, dot = FALSE, simple_pads = FALSE, dur_only = FALSE;

static GOptionEntry entries[] = {
  { "from",      0, 0, G_OPTION_ARG_DOUBLE, &from,        "Do not take events before timestamp into account", NULL },
  { "till",      0, 0, G_OPTION_ARG_DOUBLE, &till,        "Do not take events after timestamp into account",  NULL },
  { "memory",    0, 0, G_OPTION_ARG_NONE,   &show_memory, "Show memory usage",                                NULL },
  { "types",     0, 0, G_OPTION_ARG_NONE,   &show_types,  "Show types of elements",                           NULL },
  { "hierarchy", 0, 0, G_OPTION_ARG_NONE,   &hierarchy,   "Show hierarchy of elements",                       NULL },
  { "textpads",  0, 0, G_OPTION_ARG_NONE,   &simple_pads, "Show simple pad nodes (without SVG inclusion)",    NULL },
  { "nested",    0, 0, G_OPTION_ARG_NONE,   &nested_time, "Include CPU time spent by nested elements",        NULL },
  { "dot",       0, 0, G_OPTION_ARG_NONE,   &dot,         "Output in DOT format",                             NULL },
  { "duration",  0, 0, G_OPTION_ARG_NONE,   &dur_only,    "Only show duration",                               NULL },
  { NULL }
};

void
render_space (gsize space)
{
  int j;
  for (j = 0; j < space; j++) {
    g_print (" ");
  }
}

void
render_pad (gpointer key, gpointer value, gpointer user_data)
{
  GstPadHeadstone *pad = (GstPadHeadstone *)value;
  GstElementHeadstone *element = (GstElementHeadstone *)user_data;
  gint space = (element->nesting + 1);
  render_space (space);
  g_print ("subgraph cluster_eg%p_pad_%p {\n", element->identifier, pad->identifier);
  space++;
  render_space (space);
  g_print ("style=\"invis\";\n");
  render_space (space);
  g_print ("label=\"\";\n");
  render_space (space);
  if (simple_pads)
    g_print ("en%p_pad_%p [shape=\"box\" label=\"%s\", fillcolor=\"#ffffff\"];\n", element->identifier, pad->identifier, pad->mode == GST_PAD_MODE_PULL ? "[]=>" : "=>[]");
  else
    g_print ("en%p_pad_%p [shape=\"none\", label=\"\", image=\"" PREFIX "share/" PACKAGE "/%s.svg\", fillcolor=\"#ffffff\"];\n", element->identifier, pad->identifier, pad->mode == GST_PAD_MODE_PULL ? "pull" : "push");
  space--;
  render_space (space);
  g_print ("}\n");
}

void
bind_src_pad (gpointer key, gpointer value, gpointer user_data) {
  GstPadHeadstone *pad = (GstPadHeadstone *)value;
  if (pad->direction == GST_PAD_SRC) {
    GstPadHeadstone *sink_pad = (GstPadHeadstone *)user_data;
    g_print ("en%p_pad_%p -> en%p_pad_%p [style=\"invis\"];\n", sink_pad->parent_element, sink_pad->identifier, pad->parent_element, pad->identifier);
  }
}

void
bind_sink_pad (gpointer key, gpointer value, gpointer user_data) {
  GstPadHeadstone *pad = (GstPadHeadstone *)value;
  if (pad->direction == GST_PAD_SINK) {
    GstElementHeadstone *element = (GstElementHeadstone *)user_data;
    g_hash_table_foreach (element->pads, bind_src_pad, pad);
  }
}

void
render_headstone (GstGraveyard *graveyard, GstElementHeadstone *element, gsize max_length, gsize max_type_name_length)
{
  gint j;
  
  guint64 total_time = nested_time ? gst_element_headstone_get_nested_time (element) : element->total_time;
  gsize space = element->nesting;
  
  if (hierarchy) {
    render_space (space);
  }
  
  if (dot) {
    g_print ("%sgraph cluster_eg%p {\n", element->parent == NULL ? "di" : "sub", element->identifier);
  }
  
  if (dot && (element->parent == NULL)) {
    space = (element->nesting + 1);
    render_space (space);
    g_print ("rankdir=LR;\n");
    render_space (space);
    g_print ("labelloc=t;\n");
    render_space (space);
    g_print ("nodesep=.1;\n");
    render_space (space);
    g_print ("ranksep=.1;\n");
/*
    render_space (space);
    g_print ("fontname=\"Avenir Next\";\n");
*/
    render_space (space);
    g_print ("fontsize=\"14\";\n");
    render_space (space);
    g_print ("style=\"filled,rounded\";\n");
    render_space (space);
    g_print ("color=black;\n");
    render_space (space);
    g_print ("label=\"pipeline0\";\n");
    render_space (space);
    g_print ("node [style=\"filled\", shape=box, fontsize=\"14\", margin=\"0.1,0.1\"];\n");
    render_space (space);
    g_print ("edge [labelfontsize=\"14\", fontsize=\"14\"];\n"); // , fontname=\"Avenir Next\"
  }
  
  if (dot) {
    space = (element->nesting + 1);
    render_space (space);
    g_print ("fillcolor=\"#ffffff\";\n");
    g_hash_table_foreach (element->pads, render_pad, element);
    g_hash_table_foreach (element->pads, bind_sink_pad, element);
    render_space (space);
    guint64 total_time = nested_time ? gst_element_headstone_get_nested_time (element) : element->total_time;
    gchar *time_string = format_time (total_time);
    
    g_print ("label=<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"0\">"
             "<TR>"
             "<TD COLSPAN=\"2\" ALIGN=\"RIGHT\">%s</TD>"
             "</TR>"
             "<TR>"
             "<TD COLSPAN=\"2\" ALIGN=\"RIGHT\">%s</TD>"
             "</TR>"
             "<TR>"
             "<TD ALIGN=\"RIGHT\">Time:</TD>"
             "<TD ALIGN=\"RIGHT\">%s</TD>"
             "</TR>"
             "<TR>"
             "<TD ALIGN=\"RIGHT\"></TD>"
             "<TD ALIGN=\"RIGHT\">(%.1f%%)</TD>"
             "</TR>"
             "</TABLE>>;\n", element->type_name->str, element->name->str, time_string, total_time * 100.f / graveyard->total_time);
  
    g_free (time_string);
  }
  else
  {
    g_print ("%s", element->name->str);
  }

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
    g_print (" %5.1f  %8s", (nested_time ? gst_element_headstone_get_nested_load (element) : element->cpu_load) * 100.f, time_string);
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
render_connection (gpointer key, gpointer value, gpointer user_data)
{
  GstPadHeadstone *pad = (GstPadHeadstone *)value;
  if (pad->direction == GST_PAD_SRC) {
    GstElementHeadstone *element = (GstElementHeadstone *)user_data;
    gint space = (element->nesting + 1);
    render_space (space);
    gchar *memory_sent_size_string = format_memory_size (pad->bytes);
    g_print ("en%p_pad_%p -> en%p_pad_%p [label=\"%s\"];\n", element->identifier, pad->identifier, pad->peer_element, pad->peer, memory_sent_size_string);
    g_free (memory_sent_size_string);
  }
}

void
render_container (GstGraveyard *graveyard, GstElementHeadstone *element, gsize max_length, gsize max_type_name_length)
{
  GList *child;
  render_headstone (graveyard, element, max_length, max_type_name_length);
  for (child = element->children; child != NULL; child = child->next) {
    GstElementHeadstone *child_element = child->data;
    render_container (graveyard, child_element, max_length, max_type_name_length);
    if (dot) {
      g_hash_table_foreach (child_element->pads, render_connection, child_element);
    }
  }
  if (dot) {
    render_space (element->nesting);
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
  
  GstGraveyard *graveyard = gst_graveyard_new_from_trace (argv[argc - 1], from_ns, till_ns, dur_only);
  if (graveyard == NULL)
    return 3;
  
  if (dur_only) {
    g_print ("%" G_GUINT64_FORMAT, graveyard->duration);
    return 0;
  }
  
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
      for (j = 0; j < space; j++) {
        g_print (" ");
      }
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
