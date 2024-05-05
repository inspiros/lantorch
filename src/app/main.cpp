#include <gst/gst.h>

#include <QApplication>
#include <QScreen>
#include <QFile>
#include <QTextStream>

#include "ui/main_window.h"

#include "app_config.h"

int main(int argc, char *argv[]) {
    auto configs = AppConfig::load("../resources/configs.yaml");

    gst_init(&argc, &argv);
    QApplication app(argc, argv);
    QApplication::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    // qdarkstyle
    QString style_sheet = "";
    if (configs["app"]["ui"]["style_sheet_filepath"].IsDefined()) {
        QFile f(QString::fromStdString(
                configs["app"]["ui"]["style_sheet_filepath"].as<AppConfig::crel_path>().string()));
        if (f.exists()) {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            style_sheet = ts.readAll();
        }
    }

    MainWindow main_window;
    main_window.setStyleSheet(style_sheet);
    main_window.reset();
    main_window.show();

    if (!main_window.isFullScreen())
        main_window.move(
                QGuiApplication::screenAt(main_window.pos())->geometry().center() - main_window.rect().center());
    main_window.clearFocus();

    return QApplication::exec();
}
