#include "trace.h"
#include <stdio.h>

GMutex trace_mutex;

typedef struct TraceEntry
{
  GstElement *pipeline;
  gchar *text;
} TraceEntry;

GList *trace_entries = NULL;

void
gst_element_dump_to_file (GstElement *pipeline, const gchar *filename)
{
  GList *iterator;
  g_mutex_lock (&trace_mutex);
  
  FILE *output = fopen (filename, "wt");
  
  for (iterator = g_list_last (trace_entries); iterator != NULL; iterator = iterator->prev)
  {
    TraceEntry *entry = (TraceEntry *)iterator->data;
    if (entry)
    {
      if ((pipeline == NULL) || (entry->pipeline == pipeline))
      {
        fprintf(output, "%s\n", entry->text);
        
        /*
        iterator->data = NULL;
        g_free(entry->text);
        g_free(entry);
        */
      }
    }
  }
  
  // g_list_free(trace_entries);
  // trace_entries = NULL;
  
  /*
  if (iterator->prev)
  {
    GList *prev = iterator->prev->prev;
    if (prev)
      prev->next = iterator;
    iterator->prev = prev;
  }
  else
  {
    iterator->prev = NULL;
  }
  */
  fclose (output);
  
  g_mutex_unlock (&trace_mutex);
}

void trace_init (void)
{
  g_mutex_init (&trace_mutex);
}

void
trace_add_entry (GstElement *pipeline, gchar *text)
{
  TraceEntry *entry = g_new0 (TraceEntry, 1);
  entry->pipeline = pipeline;
  entry->text = text;
  
  g_mutex_lock (&trace_mutex);
  trace_entries = g_list_prepend (trace_entries, entry);
  g_mutex_unlock (&trace_mutex);
}
