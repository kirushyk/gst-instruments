#ifndef __GST_INSTRUMENTS_TRACER_H__
#define __GST_INSTRUMENTS_TRACER_H__

#include <gst/gst.h>
#include <gst/gsttracer.h>

G_BEGIN_DECLS

#define GST_TYPE_INSTRUMENTS_TRACER \
  (gst_latency_tracer_get_type())
#define GST_INSTRUMENTS_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_INSTRUMENTS_TRACER,GstInstrumentsTracer))
#define GST_INSTRUMENTS_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_INSTRUMENTS_TRACER,GstInstrumentsTracerClass))
#define GST_IS_INSTRUMENTS_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_INSTRUMENTS_TRACER))
#define GST_IS_INSTRUMENTS_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_INSTRUMENTS_TRACER))
#define GST_INSTRUMENTS_TRACER_CAST(obj) ((GstInstrumentsTracer *)(obj))

typedef struct _GstInstrumentsTracer GstInstrumentsTracer;
typedef struct _GstInstrumentsTracerClass GstInstrumentsTracerClass;

/**
 * GstInstrumentsTracer:
 *
 * Opaque #GstInstrumentsTracer data structure
 */
struct _GstInstrumentsTracer {
  GstTracer 	 parent;

  /*< private >*/
};

struct _GstInstrumentsTracerClass {
  GstTracerClass parent_class;

  /* signals */
};

G_GNUC_INTERNAL GType gst_instruments_tracer_get_type (void);

G_END_DECLS

#endif

