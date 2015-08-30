#ifndef __GST_INTERCEPT_TRACE_H__
#define __GST_INTERCEPT_TRACE_H__

#include <glib.h>
#include <gst/gst.h>

G_BEGIN_DECLS

void                gst_pipeline_dump_to_file              (GstPipeline        *pipeline,
                                                            const gchar        *filename);

G_END_DECLS

#endif
