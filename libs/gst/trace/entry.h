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

#ifndef __GST_TRACE_ENTRY_H__
#define __GST_TRACE_ENTRY_H__

#include <glib.h>

typedef enum GstTraceEntryType
{
  GST_TRACE_ENTRY_TYPE_ELEMENT_DISCOVERED,
  GST_TRACE_ENTRY_TYPE_ELEMENT_ENTERED,
  GST_TRACE_ENTRY_TYPE_ELEMENT_EXITED,
  GST_TRACE_ENTRY_TYPE_DATA_SENT
} GstTraceEntryType;

typedef struct GstTraceEntry
{
  GstTraceEntryType type;
  GstClockTime timestamp;
  GstPipeline *pipeline;
} GstTraceEntry;

#endif