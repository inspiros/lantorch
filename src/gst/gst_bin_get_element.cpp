#include "gst_bin_get_element.h"

#include <limits>

GstElement *gst_bin_get_element_by_index(GstBin *bin, guint index) {
    GstIterator *it = gst_bin_iterate_sorted(bin);
    GValue data = G_VALUE_INIT;
    GstElement *e = NULL;
    guint id = 0;
    gboolean done = FALSE;
    while (!done) {
        switch (gst_iterator_next(it, &data)) {
            case GST_ITERATOR_OK:
                e = GST_ELEMENT_CAST(g_value_get_object(&data));
                g_value_reset(&data);
                if (id == index)
                    done = TRUE;
                id += 1;
                break;
            case GST_ITERATOR_RESYNC:
                gst_iterator_resync(it);
                break;
            case GST_ITERATOR_DONE:
                if (index != std::numeric_limits<decltype(index)>::max())
                    e = NULL;
            default:
                done = TRUE;
                break;
        }
    }
    gst_iterator_free(it);
    g_value_unset(&data);
    return e;
}

GstElement *gst_bin_get_first_element(GstBin *bin) {
    return gst_bin_get_element_by_index(bin, 0);
}

GstElement *gst_bin_get_last_element(GstBin *bin) {
    return gst_bin_get_element_by_index(bin, std::numeric_limits<guint>::max());
}

GstElement *gst_bin_get_first_added_element(GstBin *bin) {
    return gst_bin_get_last_element(bin);
}

GstElement *gst_bin_get_last_added_element(GstBin *bin) {
    return gst_bin_get_first_element(bin);
}

GstElement *gst_bin_get_element_by_factory_name(GstBin *bin, const gchar *factory_name) {
    GstIterator *it = gst_bin_iterate_sorted(bin);
    GValue data = G_VALUE_INIT;
    GstElement *e = NULL;
    gboolean done = FALSE;
    while (!done) {
        switch (gst_iterator_next(it, &data)) {
            case GST_ITERATOR_OK:
                e = GST_ELEMENT_CAST(g_value_get_object(&data));
                g_value_reset(&data);
                if (g_str_equal(
                        gst_object_get_name(GST_OBJECT_CAST(gst_element_get_factory(e))),
                        factory_name))
                    done = TRUE;
                break;
            case GST_ITERATOR_RESYNC:
                gst_iterator_resync(it);
                break;
            case GST_ITERATOR_DONE:
                e = NULL;
            default:
                done = TRUE;
                break;
        }
    }
    gst_iterator_free(it);
    g_value_unset(&data);
    return e;
}
