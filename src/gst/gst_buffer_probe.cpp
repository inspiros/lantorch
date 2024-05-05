#include "gst_buffer_probe.h"

GstBufferProbe::~GstBufferProbe() {
    if (_pad && GST_OBJECT_REFCOUNT(GST_OBJECT_CAST(_pad))) {
        gst_pad_remove_probe(_pad, callback_id);
    }
}

gulong GstBufferProbe::add_on(GstPad *pad) {
    if (is_added())
        g_error("callback already added with id=%lu", callback_id);
    callback_id = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                                    this->to_callback(), NULL, NULL);
    if (callback_id) {
        _pad = pad;
        bind_lifecycle(pad);
    }
    return callback_id;
}

gulong GstBufferProbe::add_on(GstElement *element, const gchar *pad_name) {
    if (is_added())
        g_error("callback already added with id=%lu", callback_id);
    GstPad *pad;
    pad = gst_element_get_static_pad(element, pad_name);
    callback_id = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                                    this->to_callback(), NULL, NULL);
    if (callback_id) {
        _pad = pad;
        bind_lifecycle(pad);
    }
    gst_object_unref(pad);
    return callback_id;
}
