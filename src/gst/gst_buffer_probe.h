#pragma once

#include "std/function_ptr.h"

#include "gst_self_destruct_object.h"

/**
 * A self destruct buffer probe class.
 */
class GstBufferProbe : public GstSelfDestructObject {
    GstPad *_pad = NULL;
    gulong callback_id = 0;
    GstPadProbeCallback callback_func = NULL;

public:
    GstBufferProbe() = default;

    ~GstBufferProbe() override;

    template<typename Function>
    inline void set_callback_func(Function &&cb) {
        callback_func = std::function_ptr<GstPadProbeReturn(GstPad *, GstPadProbeInfo *, gpointer)>(cb);
    }

    virtual inline GstPadProbeReturn call(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
        if (callback_func)
            return callback_func(pad, info, user_data);
        return GST_PAD_PROBE_OK;
    }

    inline GstPadProbeReturn operator()(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
        return call(pad, info, user_data);
    }

    inline GstPadProbeCallback to_callback() {
        return std::function_ptr<GstPadProbeReturn(GstPad *, GstPadProbeInfo *, gpointer)>(
                [=](GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
                    return this->call(pad, info, user_data);
                });
    }

    gulong add_on(GstPad *pad);

    gulong add_on(GstElement *element, const gchar *pad_name);

    [[nodiscard]] inline gboolean is_added() const {
        return callback_id != 0;
    }
};
