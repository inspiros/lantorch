#pragma once

#include <gst/gstbin.h>

GstElement *gst_bin_get_element_by_index(GstBin *bin, guint index);

GstElement *gst_bin_get_first_element(GstBin *bin);

GstElement *gst_bin_get_last_element(GstBin *bin);

GstElement *gst_bin_get_first_added_element(GstBin *bin);

GstElement *gst_bin_get_last_added_element(GstBin *bin);

GstElement *gst_bin_get_element_by_factory_name(GstBin *bin, const gchar *factory_name);
