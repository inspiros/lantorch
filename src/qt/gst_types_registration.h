#pragma once

#include <gst/gst.h>

#include <QMetaType>

// TODO: register as needed
const int GST_CLOCK_TIME_META_ID = qRegisterMetaType<GstClockTime>("GstClockTime");
