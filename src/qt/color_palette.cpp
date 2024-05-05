#include "color_palette.h"

ColorPalette::ColorPalette() : ColorPalette("tab10") {}

ColorPalette::ColorPalette(const QString &palette) {
    for (const auto &palette_group : ALL_PALETTES) {
        if (palette_group.contains(palette)) {
            name_ = palette;
            colors_ = palette_group.value(palette);
            break;
        }
    }
    if (colors_.empty())
        throw UndefinedColorPalette();
}

ColorPalette::ColorPalette(const QList<QColor> &palette) {
    colors_.reserve(palette.size());
    std::copy(palette.begin(), palette.end(), colors_.begin());
}

QString ColorPalette::name() const noexcept {
    return name_;
}

QList<QColor> ColorPalette::colors() const noexcept {
    return colors_;
}

int ColorPalette::size() const noexcept {
    return colors_.size();
}

QColor ColorPalette::at(int i) const noexcept {
    return colors_.at(i % size());
}
