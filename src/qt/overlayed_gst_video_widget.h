#pragma once

#include "GstVideoWidget"
#include "OverlayWidget"

class OverlayedGstVideoWidget : public GstVideoWidget {
Q_OBJECT
    OverlayWidget *overlay_;

public:
    explicit OverlayedGstVideoWidget(QWidget *parent = nullptr);

    [[nodiscard]] OverlayWidget *overlay() const;
};
