/* GStreamer Instruments
 * Copyright (C) 2015 Kyrylo Polezhaiev <kirushyk@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
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

gchar *
format_time (GstClockTime nanoseconds, gboolean use_mu_symbol)
{
  if (nanoseconds < 1000) {
    return g_strdup_printf ("%" G_GUINT64_FORMAT " ns", nanoseconds);
  } else if (nanoseconds < 10000) {
    return g_strdup_printf ("%.2f %ss", 0.001 * nanoseconds, use_mu_symbol ? "μ" : "u");
  } else if (nanoseconds < 100000) {
    return g_strdup_printf ("%.1f %ss", 0.001 * nanoseconds, use_mu_symbol ? "μ" : "u");
  } else if (nanoseconds < 1000000) {
    return g_strdup_printf ("%.0f %ss", 0.001 * nanoseconds, use_mu_symbol ? "μ" : "u");
  } else if (nanoseconds < 10000000) {
    return g_strdup_printf ("%.2f ms", 0.000001 * nanoseconds);
  } else if (nanoseconds < 100000000) {
    return g_strdup_printf ("%.1f ms", 0.000001 * nanoseconds);
  } else if (nanoseconds < 1000000000) {
    return g_strdup_printf ("%.0f ms", 0.000001 * nanoseconds);
  } else if (nanoseconds < 10000000000) {
    return g_strdup_printf ("%.2f s", 0.000000001 * nanoseconds);
  } else if (nanoseconds < 100000000000) {
    return g_strdup_printf ("%.1f s", 0.000000001 * nanoseconds);
  } else {
    return g_strdup_printf ("%.0f s", 0.000000001 * nanoseconds);
  }
}

gchar *
format_memory_size (gsize bytes)
{
  if (bytes < 1024) {
    return g_strdup_printf ("%" G_GUINT64_FORMAT " B", bytes);
  } else if (bytes < 10240) {
    return g_strdup_printf ("%.2f KiB", bytes / 1024.f);
  } else if (bytes < 102400) {
    return g_strdup_printf ("%.1f KiB", bytes / 1024.f);
  } else if (bytes < 1024000) {
    return g_strdup_printf ("%.0f KiB", bytes / 1024.f);
  } else if (bytes < 10485760) {
    return g_strdup_printf ("%.2f MiB", bytes / 1048576.f);
  } else if (bytes < 104857600) {
    return g_strdup_printf ("%.1f MiB", bytes / 1048576.f);
  } else if (bytes < 1048576000) {
    return g_strdup_printf ("%.0f MiB", bytes / 1048576.f);
  } else if (bytes < 10737418240) {
    return g_strdup_printf ("%.2f GiB", bytes / 1073741824.f);
  } else if (bytes < 107374182400) {
    return g_strdup_printf ("%.1f GiB", bytes / 1073741824.f);
  } else {
    return g_strdup_printf ("%.0f GiB", bytes / 1073741824.f);
  }
}
