#pragma once

#include <gst/gstpad.h>

typedef enum {
    GST_PAD_TYPE_STATIC,
    GST_PAD_TYPE_REQUEST,
    GST_PAD_TYPE_DYNAMIC,
} GstPadType;

inline GstPad *gst_element_get_pad(
        GstElement *element, const gchar *pad, GstPadType type = GST_PAD_TYPE_STATIC) {
    switch (type) {
        case GST_PAD_TYPE_STATIC:
            return gst_element_get_static_pad(element, pad);
        case GST_PAD_TYPE_REQUEST:
            return gst_element_request_pad_simple(element, pad);
        default:
            g_error("cannot get dynamically added pad");
    }
}

inline GstPad *gst_element_try_get_pad(
        GstElement *element, const gchar *pad, GstPadType type = GST_PAD_TYPE_STATIC) {
    switch (type) {
        case GST_PAD_TYPE_STATIC:
            return gst_element_get_static_pad(element, pad);
        case GST_PAD_TYPE_REQUEST:
            return gst_element_request_pad_simple(element, pad);
        default:
            return NULL;
    }
}
