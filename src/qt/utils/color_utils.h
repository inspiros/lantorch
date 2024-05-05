#pragma once

#include <QColor>

QColor invertColor(QColor color, bool black_white = false) {
    if (black_white) {  // https://stackoverflow.com/a/3943023/112731
        return (color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) > 186 ? Qt::black : Qt::white;
    }
    return QColor(255 - color.red(), 255 - color.green(), 255 - color.blue());
}
