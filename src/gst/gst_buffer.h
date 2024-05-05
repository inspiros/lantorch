#pragma once

#include <gst/gstbuffer.h>
#include <gst/video/video-format.h>

GstBuffer *gst_buffer_new_memdup_from_contiguous(gconstpointer data, gint width, gint height, GstVideoFormat format);
