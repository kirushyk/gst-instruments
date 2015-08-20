#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

void parse_output(const char *filename);

int
main (int argc, char *argv[])
{
  const char dump_filename[] = "gst-top.intercept";
  
  g_set_prgname ("gst-top-1.0");
  g_set_application_name ("GStreamer Top Tool");
  
#if defined(__APPLE__)
  g_setenv ("DYLD_FORCE_FLAT_NAMESPACE", "", FALSE);
  g_setenv ("DYLD_INSERT_LIBRARIES", "/usr/local/lib/libgstintercept.dylib", TRUE);
#elif defined(G_OS_UNIX)
  g_setenv ("LD_PRELOAD", "libgstintercept.so", TRUE);
#else
# error GStreamer API calls interception is not supported on this platform
#endif
  
  g_setenv ("GST_INTERCEPT_OUTPUT_FILE", dump_filename, TRUE);
  // g_setenv ("GST_DEBUG", "task:DEBUG", TRUE);
  
  gint status = 0;
  GError *error = NULL;
  g_spawn_sync (NULL, argv + 1, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &status, &error);
  
  if (error)
  {
    g_print ("%s\n", error->message);
  }
  else
  {
    parse_output(dump_filename);
  }
  
  return 0;
}

typedef struct GstTaskRecord
{
  gpointer identifier;
  guint64 total_downstack_time;
  
  gboolean in_upstack;
  guint64 enter_upstack_time;
  guint64 exit_upstack_time;
  
  guint64 total_upstack_time;
  GString *name;
} GstTaskRecord;
GHashTable *tasks = NULL;

typedef struct GstElementRecord
{
  gboolean is_subtop;
  gpointer identifier;
  guint64 total_time;
  GString *name;
} GstElementRecord;
GHashTable *elements = NULL;

void
for_each_task (gpointer key, gpointer value, gpointer user_data)
{
  GstTaskRecord *task = value;
  g_print ("%s: %5.3f ms\n", task->name->str, task->total_upstack_time * 0.001);
}

void
for_each_element (gpointer key, gpointer value, gpointer user_data)
{
  GstElementRecord *element = value;
  g_print ("%s: %5.3f ms\n", element->name->str, element->total_time * 0.001);
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
      // g_print ("%s: ", event_name);
      if (g_ascii_strcasecmp (event_name, "element-entered") == 0)
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
            g_hash_table_insert (tasks, task_id, task);
          }
            
          g_assert_true (task != NULL);
            
          if (task->in_upstack)
          {
            element->is_subtop = TRUE;
            task->exit_upstack_time = thread_time;
            task->total_upstack_time += task->exit_upstack_time - task->enter_upstack_time;
            task->in_upstack = FALSE;
            if (task->name == NULL)
            {
              task->name = g_string_new (from_element_name);
            }
          }
          else
          {
            element->is_subtop = FALSE;
          }
          task->total_downstack_time = 0;
        }
        else
        {
          g_printerr ("couldn't parse event: %s\n", event_name);
          exit (2);
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
          element->total_time += duration - task->total_downstack_time;
          task->total_downstack_time = duration;
          if (element->is_subtop)
          {
            task->enter_upstack_time = thread_time;
            task->in_upstack = TRUE;
          }
          else
          {
            task->in_upstack = FALSE;
          }
          
          // g_print ("%p %s %p %" G_GUINT64_FORMAT"\n", task_id, element_name, element_id, duration);
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
  
  g_hash_table_foreach (tasks, for_each_task, NULL);
  g_print ("\n");
  g_hash_table_foreach (elements, for_each_element, NULL);
  
  g_hash_table_destroy (elements);
  g_hash_table_destroy (tasks);
}
