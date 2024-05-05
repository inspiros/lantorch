#pragma once

#include <functional>

#include <gst/gstpad.h>
#include "gst_element_get_pad.h"

#include "std/function_ptr.h"

//namespace detail {
//    /**
//     * Self destruct pad probe adder class, always create with new.
//     */
//    struct GstPadProbeAdder {
//        GstElement *element;
//        const gchar *pad_name;
//        GstPadType pad_type;
//        pad_filter_func pad_filter = NULL;
//        GstPad *pad = NULL;
//        gboolean verbose;
//
//        GstPadProbeAdder(GstElement *element, const gchar *pad, GstPadType pad_type,
//                         gboolean verbose = false);
//
//        GstPadProbeAdder(GstElement *element, const gchar *pad, GstPadType pad_type, pad_filter_func pad_filter,
//                         gboolean verbose = false);
//
//        ~GstPadProbeAdder();
//
//        inline GCallback pad_added_callback() {
//            return G_CALLBACK(std::function_ptr<void(GstElement *, GstPad *, gpointer)>(
//                    [=](GstElement *src, GstPad *new_pad, gpointer u_data) -> void {
//                        this->pad_added_handler(src, new_pad, u_data);
//                    }));
//        }
//
//        gulong add();
//
//        inline gulong operator()() {
//            return add();
//        }
//
//    private:
//        void pad_added_handler(GstElement *src, GstPad *new_pad, gpointer);
//
//        gulong dynamic_add() const;
//    };
//}

template<typename Callback_Type>
gulong gst_element_pad_add_probe(
        GstElement *element,
        const char *padname,
        GstPadProbeType mask,
        Callback_Type &&cb,
        gpointer user_data = NULL,
        GDestroyNotify destroy_data = NULL) {
    GstPad *pad;
    pad = gst_element_get_static_pad(element, padname);
    auto id = gst_pad_add_probe(
            pad, mask,
            std::function_ptr<GstPadProbeReturn(GstPad *, GstPadProbeInfo *, gpointer)>(cb),
            user_data, destroy_data);
    gst_object_unref(pad);
    return id;
}

void gst_element_pad_remove_probe(
        GstElement *element,
        const char *padname,
        gulong id) {
    GstPad *pad;
    pad = gst_element_get_static_pad(element, padname);
    gst_pad_remove_probe(pad, id);
    gst_object_unref(pad);
}
