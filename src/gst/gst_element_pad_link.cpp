#include "gst_element_pad_link.h"

namespace detail {
    GstPadLinker::GstPadLinker(
            GstElement *element_1, const gchar *pad_1, GstPadType pad_1_type,
            GstElement *element_2, const gchar *pad_2, GstPadType pad_2_type,
            gboolean verbose)
            : element_1(element_1), pad_1(pad_1), pad_1_type(pad_1_type), pad_1_filter(NULL),
              element_2(element_2), pad_2(pad_2), pad_2_type(pad_2_type), pad_2_filter(NULL),
              verbose(verbose) {
        srcpad = gst_element_try_get_pad(element_1, pad_1, pad_1_type);
        sinkpad = gst_element_try_get_pad(element_2, pad_2, pad_2_type);
        if (srcpad)
            bind_lifecycle(srcpad);
        if (sinkpad)
            bind_lifecycle(sinkpad);
    }

    GstPadLinker::GstPadLinker(
            GstElement *element_1, const gchar *pad_1, GstPadType pad_1_type, pad_filter_func pad_1_filter,
            GstElement *element_2, const gchar *pad_2, GstPadType pad_2_type, pad_filter_func pad_2_filter,
            gboolean verbose)
            : element_1(element_1), pad_1(pad_1), pad_1_type(pad_1_type), pad_1_filter(pad_1_filter),
              element_2(element_2), pad_2(pad_2), pad_2_type(pad_2_type), pad_2_filter(pad_2_filter),
              verbose(verbose) {
        srcpad = gst_element_try_get_pad(element_1, pad_1, pad_1_type);
        sinkpad = gst_element_try_get_pad(element_2, pad_2, pad_2_type);
        if (srcpad)
            bind_lifecycle(srcpad);
        if (sinkpad)
            bind_lifecycle(sinkpad);
    }

    GstPadLinker::~GstPadLinker() {
        if (pad_1_type != GST_PAD_TYPE_DYNAMIC && srcpad)
            gst_object_unref(srcpad);
        else if (srcpad_added_handler_id) {
            g_signal_handler_disconnect(element_1, srcpad_added_handler_id);
        }
        if (pad_2_type != GST_PAD_TYPE_DYNAMIC && sinkpad)
            gst_object_unref(sinkpad);
        else if (sinkpad_added_handler_id) {
            g_signal_handler_disconnect(element_2, sinkpad_added_handler_id);
        }
    }

    GstPadLinkReturn GstPadLinker::link() {
        if (pad_1_type != GST_PAD_TYPE_DYNAMIC && pad_2_type != GST_PAD_TYPE_DYNAMIC) {
            GstPadLinkReturn result;
            if (!srcpad || !sinkpad)
                result = GST_PAD_LINK_REFUSED;
            else
                result = gst_pad_link(srcpad, sinkpad);
            delete this;
            return result;
        } else {
            if (!srcpad) {
                srcpad_added_handler_id = g_signal_connect(
                        element_1, "pad-added",
                        this->src_pad_added_callback(),
                        NULL);
            }
            if (!sinkpad) {
                sinkpad_added_handler_id = g_signal_connect(
                        element_2, "pad-added",
                        this->sink_pad_added_callback(),
                        NULL);
            }
            return GST_PAD_LINK_OK;
        }
    }

    void GstPadLinker::src_pad_added_handler(GstElement *src, GstPad *new_pad, gpointer) {
        if (srcpad || (pad_1_filter && !pad_1_filter(new_pad)))
            return;
        srcpad = new_pad;
        bind_lifecycle(srcpad);
        if (verbose)
            g_print("dynamic src pad acquired\n");
        if (pad_2_type == GST_PAD_TYPE_DYNAMIC && !sinkpad)
            return;
        dynamic_link();
        delete this;
    }

    void GstPadLinker::sink_pad_added_handler(GstElement *src, GstPad *new_pad, gpointer) {
        if (sinkpad || pad_2_filter && !pad_2_filter(new_pad))
            return;
        sinkpad = new_pad;
        bind_lifecycle(sinkpad);
        if (verbose)
            g_print("dynamic sink_ pad acquired\n");
        if (pad_1_type == GST_PAD_TYPE_DYNAMIC && !srcpad)
            return;
        dynamic_link();
        delete this;
    }

    GstPadLinkReturn GstPadLinker::dynamic_link() const {
        if (!srcpad || !sinkpad)  // filtered
            return GST_PAD_LINK_REFUSED;

        // check if either pad is already linked
        if (gst_pad_is_linked(srcpad)) {
            if (verbose)
                g_printerr("src pad already linked, ignoring\n");
            return GST_PAD_LINK_REFUSED;
        } else if (gst_pad_is_linked(sinkpad)) {
            if (verbose)
                g_printerr("sink pad already linked, ignoring\n");
            return GST_PAD_LINK_REFUSED;
        }

        // try linking
        auto result = gst_pad_link(srcpad, sinkpad);
        if (verbose) {
            if (GST_PAD_LINK_FAILED(result)) {
                auto *src_pad_caps = gst_pad_get_current_caps(srcpad);
                auto *sink_pad_caps = gst_pad_get_current_caps(sinkpad);
                g_printerr("dynamic linking failed:\n"
                           "srcpad: %s\n"
                           "sinkpad: %s\n",
                           gst_caps_to_string(src_pad_caps),
                           gst_caps_to_string(sink_pad_caps));
                gst_caps_unref(src_pad_caps);
                gst_caps_unref(sink_pad_caps);
            } else {
                g_print("dynamic linking successful\n");
            }
        }
        return result;
    }
}
