#include <dlfcn.h>
#include <glib.h>
#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>

void parse_output(const char *filename);

int
main (int argc, char *argv[])
{
  g_set_prgname ("gst-report-1.0");
  g_set_application_name ("GStreamer Report Tool");
  
  if (argc != 2)
    return 1;
  
  parse_output(argv[1]);
  
  return 0;
}

typedef struct GstTaskRecord
{
  gpointer identifier;
  gpointer upstack_element_identifier;
  GString *name;
  
  guint64 current_downstack_time;
  guint64 total_downstack_time;
  
  gboolean in_upstack;
  guint64 enter_upstack_time;
  guint64 exit_upstack_time;
  guint64 total_upstack_time;
} GstTaskRecord;
GHashTable *tasks = NULL;

typedef struct GstElementRecord
{
  guint64 data_sent;
  guint64 data_received;
  gboolean is_subtop;
  gpointer identifier;
  guint64 total_time;
  GString *name;
} GstElementRecord;
GHashTable *elements = NULL;
GArray *elements_sorted = NULL;

gint
elements_compare_func (gconstpointer a, gconstpointer b)
{
  gint diff = (gint64)(*(GstElementRecord **)b)->total_time - (gint64)(*(GstElementRecord **)a)->total_time;
  if (diff > 0)
    return 1;
  else if (diff < 0)
    return -1;
  return 0;
}

void
for_each_task (gpointer key, gpointer value, gpointer user_data)
{
  GstTaskRecord *task = value;
  if (task->upstack_element_identifier)
  {
    GstElementRecord *upstack_element = g_hash_table_lookup (elements, task->upstack_element_identifier);
    
    if (!upstack_element)
    {
      upstack_element = g_new0 (GstElementRecord, 1);
      upstack_element->is_subtop = FALSE;
      upstack_element->data_sent = 0;
      upstack_element->data_received = 0;
      upstack_element->identifier = task->upstack_element_identifier;
      upstack_element->total_time = 0;
      upstack_element->name = g_string_new (task->name->str);
      g_hash_table_insert (elements, task->upstack_element_identifier, upstack_element);
    }
    
    upstack_element->total_time += task->total_upstack_time;
  }
}

void
for_each_element (gpointer key, gpointer value, gpointer user_data)
{
  g_array_append_val (elements_sorted, value);
}

void
parse_output (const char *filename)
{
  tasks = g_hash_table_new (g_direct_hash, g_direct_equal);
  elements = g_hash_table_new (g_direct_hash, g_direct_equal);
  
  FILE *input = fopen (filename, "rt");
  if (input == NULL)
    return;
  
  while (!feof (input))
  {
    gchar event_name[1000];
    if (fscanf (input, "%s", event_name) == 1)
    {
      if (g_ascii_strcasecmp (event_name, "element-discovered") == 0)
      {
        gpointer element_id;
        gchar element_name[1000];
        gchar element_type_name[1000];
        if (fscanf (input, "%p %s %s\n", &element_id, element_name, element_type_name) == 3)
        {
          
        }
      }
      else if (g_ascii_strcasecmp (event_name, "element-entered") == 0)
      {
        gpointer task_id;
        gchar from_element_name[1000];
        gpointer from_element_id;
        gchar element_name[1000];
        gpointer element_id;
        guint64 thread_time;
        if (fscanf (input, "%p %s %p %s %p %" G_GUINT64_FORMAT "\n", &task_id, from_element_name, &from_element_id, element_name, &element_id, &thread_time) == 6)
        {
          GstElementRecord *element = g_hash_table_lookup (elements, element_id);
          if (!element)
          {
            element = g_new0 (GstElementRecord, 1);
            element->data_sent = 0;
            element->data_received = 0;
            element->is_subtop = FALSE;
            element->identifier = element_id;
            element->total_time = 0;
            element->name = g_string_new (element_name);
            g_hash_table_insert (elements, element_id, element);
          }
          
          g_assert_true (element != NULL);
            
          GstTaskRecord *task = g_hash_table_lookup (tasks, task_id);
          if (!task)
          {
            task = g_new0 (GstTaskRecord, 1);
            task->identifier = task_id;
            task->total_upstack_time = 0;
            task->enter_upstack_time = 0;
            task->in_upstack = TRUE;
            task->name = NULL;
            task->upstack_element_identifier = NULL;
            g_hash_table_insert (tasks, task_id, task);
          }
            
          g_assert_true (task != NULL);
          
          if (task->in_upstack)
          {
            element->is_subtop = TRUE;
            task->exit_upstack_time = thread_time;
            if (task->enter_upstack_time > 0)
              task->total_upstack_time += task->exit_upstack_time - task->enter_upstack_time;
            task->in_upstack = FALSE;
            if (task->name == NULL)
            {
              task->upstack_element_identifier = from_element_id;
              task->name = g_string_new (from_element_name);
            }
          }
          else
          {
            element->is_subtop = FALSE;
          }
          task->total_downstack_time += task->current_downstack_time;
          task->current_downstack_time = 0;
        }
        else
        {
          g_printerr ("couldn't parse event: %s\n", event_name);
          exit (2);
        }
      }
      else if (g_ascii_strcasecmp (event_name, "data-sent") == 0)
      {
        gpointer element_from;
        gpointer element_to;
        gint buffers_count;
        guint64 size;
        if (fscanf (input, "%p %p %d %" G_GUINT64_FORMAT "\n", &element_from, &element_to, &buffers_count, &size) == 4)
        {
          GstElementRecord *element = g_hash_table_lookup (elements, element_from);
          if (!element)
          {
            g_printerr ("couldn't find element %p\n", element_from);
            exit (2);
          }
          element->data_sent += size;
          
          element = g_hash_table_lookup (elements, element_to);
          if (!element)
          {
            g_printerr ("couldn't find element %p\n", element_to);
            exit (2);
          }
          element->data_received += size;
        }
      }
      else if (g_ascii_strcasecmp (event_name, "element-exited") == 0)
      {
        gpointer task_id;
        gchar element_name[1000];
        gpointer element_id;
        guint64 thread_time;
        guint64 duration;
        if (fscanf (input, "%p %s %p %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT "\n", &task_id, element_name, &element_id, &thread_time, &duration) == 5)
        {
          GstTaskRecord *task = g_hash_table_lookup (tasks, task_id);
          if (!task)
          {
            g_printerr ("couldn't find task %p\n", task_id);
            exit (2);
          }
          GstElementRecord *element = g_hash_table_lookup (elements, element_id);
          if (!element)
          {
            g_printerr ("couldn't find element %p: %s\n", element_id, element_name);
            exit (2);
          }
          element->total_time += duration - task->current_downstack_time;
          task->current_downstack_time = duration;
          if (element->is_subtop)
          {
            task->enter_upstack_time = thread_time;
            task->in_upstack = TRUE;
          }
          else
          {
            task->in_upstack = FALSE;
          }
        }
        else
        {
          g_printerr ("couldn't parse event: %s\n", event_name);
          exit (2);
        }
      }
      else if (g_ascii_strcasecmp (event_name, "task") == 0)
      {
        gpointer task_id;
        if (fscanf (input, "%p\n", &task_id) != 1)
        {
          g_printerr ("couldn't parse task\n");
          exit (2);
        }
      }
      else
      {
        g_printerr ("couldn't recognize event: %s\n", event_name);
        exit (1);
      }
    }
  }
  fclose (input);
  
  gint index = 0;
  
  g_hash_table_foreach (tasks, for_each_task, NULL);
  
  gint elements_count = g_hash_table_size (elements);
  elements_sorted = g_array_sized_new (FALSE, FALSE, sizeof(gpointer), elements_count);
  g_hash_table_foreach (elements, for_each_element, NULL);
  g_array_sort (elements_sorted, elements_compare_func);
  for (index = 0; index < elements_count; index++)
  {
    GstElementRecord *element = g_array_index (elements_sorted, GstElementRecord *, index);
    g_print ("%s: %5.3f ms\n", element->name->str, element->total_time * 0.000001);
  }
  
  g_array_free (elements_sorted, TRUE);
  g_hash_table_destroy (elements);
  g_hash_table_destroy (tasks);
}
