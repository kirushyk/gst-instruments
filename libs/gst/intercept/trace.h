#ifndef __GST_INTERCEPT_TRACE_H__
#define __GST_INTERCEPT_TRACE_H__

#include <glib.h>
#include <gst/gst.h>

G_BEGIN_DECLS

void                gst_element_dump_to_file               (GstElement         *pipeline,
                                                            const gchar        *filename);

void                trace_init                             (void);

void                trace_add_entry                        (GstElement         *pipeline,
                                                            gchar              *text);

G_END_DECLS

#endif
