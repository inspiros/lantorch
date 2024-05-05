#pragma once

#include <gst/gst.h>

#include "std/function_ptr.h"

class GstSelfDestructObject {
    GRecMutex lock{};
    GList *lifecycle_binders = NULL;
    uint binds_count = 0;
    const GToggleNotify self_destruct_func = std::function_ptr<void(gpointer, GObject *, gboolean)>(
            [this](gpointer, GObject *, gboolean) {
                g_rec_mutex_lock(&lock);
                binds_count -= 1;
                gboolean to_be_destroyed = binds_count == 0;
                g_rec_mutex_unlock(&lock);
                if (to_be_destroyed)
                    delete this;
            });

protected:
    inline GToggleNotify self_destruct_callback() {
        return self_destruct_func;
    }

public:
    GstSelfDestructObject();

    virtual ~GstSelfDestructObject();

    void bind_lifecycle(gpointer obj);

    void bind_lifecycle_many(gpointer obj, ...);
};
