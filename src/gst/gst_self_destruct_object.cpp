#include "gst_self_destruct_object.h"

GstSelfDestructObject::GstSelfDestructObject() {
    g_rec_mutex_init(&lock);
}

GstSelfDestructObject::~GstSelfDestructObject() {
    g_list_foreach(lifecycle_binders, std::function_ptr<void(gpointer, gpointer)>([this](gpointer obj, gpointer) {
        g_object_remove_toggle_ref(G_OBJECT(obj), self_destruct_callback(), NULL);
    }), NULL);
    g_list_free(lifecycle_binders);
}

void GstSelfDestructObject::bind_lifecycle(gpointer obj) {
    if (obj) {
        g_rec_mutex_lock(&lock);
        g_object_add_toggle_ref(G_OBJECT(obj), self_destruct_callback(), NULL);
        lifecycle_binders = g_list_append(lifecycle_binders, obj);
        binds_count += 1;
        g_rec_mutex_unlock(&lock);
    }
}

void GstSelfDestructObject::bind_lifecycle_many(gpointer obj, ...) {
    va_list args;
    va_start(args, obj);
    while (obj) {
        g_rec_mutex_lock(&lock);
        g_object_add_toggle_ref(G_OBJECT(obj), self_destruct_callback(), NULL);
        lifecycle_binders = g_list_append(lifecycle_binders, obj);
        binds_count += 1;
        g_rec_mutex_unlock(&lock);
        obj = va_arg(args, gpointer);
    }
    va_end(args);
}
