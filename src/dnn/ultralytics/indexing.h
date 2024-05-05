#pragma once

#undef slots
#include <ATen/TensorIndexing.h>
#define slots Q_SLOTS

namespace ultralytics {
    namespace indexing {
        const auto BboxXSlice = at::indexing::Slice(0, 4, 2);
        const auto BboxYSlice = at::indexing::Slice(1, 4, 2);
        const auto BboxXYSlice = at::indexing::Slice(at::indexing::None, 4);
    }
}
