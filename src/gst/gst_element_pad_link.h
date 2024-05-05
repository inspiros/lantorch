#pragma once

#include <gst/gstpad.h>

#include "gst_element_get_pad.h"
#include "gst_self_destruct_object.h"
#include "std/function_ptr.h"

namespace detail {
    typedef gboolean (*pad_filter_func)(GstPad *pad);

    /**
     * Self destruct pad linker class. This will delete itself
     * after linking or after the two pads are both freed.
     */
    struct GstPadLinker : public GstSelfDestructObject {
        GstElement *element_1, *element_2;
        const gchar *pad_1, *pad_2;
        GstPadType pad_1_type, pad_2_type;
        pad_filter_func pad_1_filter = NULL, pad_2_filter = NULL;
        GstPad *srcpad = NULL, *sinkpad = NULL;
        gulong srcpad_added_handler_id = 0, sinkpad_added_handler_id = 0;
        gboolean verbose;

        GstPadLinker(GstElement *element_1, const gchar *pad_1, GstPadType pad_1_type,
                     GstElement *element_2, const gchar *pad_2, GstPadType pad_2_type,
                     gboolean verbose = false);

        GstPadLinker(GstElement *element_1, const gchar *pad_1, GstPadType pad_1_type, pad_filter_func pad_1_filter,
                     GstElement *element_2, const gchar *pad_2, GstPadType pad_2_type, pad_filter_func pad_2_filter,
                     gboolean verbose = false);

        ~GstPadLinker();

        inline GCallback src_pad_added_callback() {
            return G_CALLBACK(std::function_ptr<void(GstElement *, GstPad *, gpointer)>(
                    [=](GstElement *src, GstPad *new_pad, gpointer u_data) -> void {
                        this->src_pad_added_handler(src, new_pad, u_data);
                    }));
        }

        inline GCallback sink_pad_added_callback() {
            return G_CALLBACK(std::function_ptr<void(GstElement *, GstPad *, gpointer)>(
                    [=](GstElement *src, GstPad *new_pad, gpointer u_data) -> void {
                        this->sink_pad_added_handler(src, new_pad, u_data);
                    }));
        }

        GstPadLinkReturn link();

        inline GstPadLinkReturn operator()() {
            return link();
        }

    private:
        void src_pad_added_handler(GstElement *src, GstPad *new_pad, gpointer);

        void sink_pad_added_handler(GstElement *src, GstPad *new_pad, gpointer);

        GstPadLinkReturn dynamic_link() const;
    };
}

/**
 * Link elements with pad names and their corresponding type.
 * This function will create "pad-added" callback for dynamic pads.
 * Be careful as this may leak data if the callback is not executed.
 */
inline GstPadLinkReturn gst_element_pad_link(
        GstElement *element_1, const gchar *pad_1, GstPadType pad_1_type,
        GstElement *element_2, const gchar *pad_2, GstPadType pad_2_type,
        gboolean verbose = false) {
    auto *pad_linker = new detail::GstPadLinker(
            element_1, pad_1, pad_1_type,
            element_2, pad_2, pad_2_type,
            verbose);
    return pad_linker->link();
}

template<typename Pad1Filter, typename Pad2Filter>
inline GstPadLinkReturn gst_element_pad_link(
        GstElement *element_1, const gchar *pad_1, GstPadType pad_1_type, Pad1Filter pad_1_filter,
        GstElement *element_2, const gchar *pad_2, GstPadType pad_2_type, Pad2Filter pad_2_filter,
        gboolean verbose = false) {
    auto *pad_linker = new detail::GstPadLinker(
            element_1, pad_1, pad_1_type, std::function_ptr<gboolean(GstPad *new_pad)>(pad_1_filter),
            element_2, pad_2, pad_2_type, std::function_ptr<gboolean(GstPad *new_pad)>(pad_2_filter),
            verbose);
    return pad_linker->link();
}

template<typename PadFilter>
inline GstPadLinkReturn gst_element_pad_link(
        GstElement *element_1, const gchar *pad_1, GstPadType pad_1_type,
        GstElement *element_2, const gchar *pad_2, GstPadType pad_2_type,
        PadFilter pad_filter,
        gboolean verbose = false) {
    return gst_element_pad_link(element_1, pad_1, pad_1_type, pad_filter,
                                element_2, pad_2, pad_2_type, pad_filter,
                                verbose);
}
