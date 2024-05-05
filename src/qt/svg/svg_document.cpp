#include "svg_document.h"

#include <QFile>

SvgDocument::SvgDocument() : svg_doc_{} {}

SvgDocument::SvgDocument(const QDomDocument &document) : svg_doc_(document) {}

SvgDocument::SvgDocument(const QString &filename) : SvgDocument() {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    svg_doc_.setContent(file.readAll());
}

SvgDocument::SvgDocument(QXmlStreamReader *contents, bool namespaceProcessing) {
    svg_doc_.setContent(contents, namespaceProcessing);
}

namespace {
    inline void apply_impl(const std::function<void(QDomElement &)> &f,
                           QDomElement &element,
                           bool recursive) {
        f(element);
        if (recursive) {
            for (int i = 0; i < element.childNodes().count(); i++) {
                if (!element.childNodes().at(i).isElement())
                    continue;
                auto child = element.childNodes().at(i).toElement();
                apply_impl(f, child, recursive);
            }
        }
    }
}

SvgDocument &SvgDocument::apply(const std::function<void(QDomElement &)> &f, bool recursive) {
    if (f) {
        auto element = svg_doc_.documentElement();
        apply_impl(f, element, recursive);
    }
    return *this;
}

SvgDocument SvgDocument::clone() const {
    return SvgDocument(svg_doc_.cloneNode(true).toDocument());
}

bool SvgDocument::setContent(
        const QByteArray &text, bool namespaceProcessing, QString *errorMsg, int *errorLine, int *errorColumn) {
    return svg_doc_.setContent(text, namespaceProcessing, errorMsg, errorLine, errorColumn);
}

bool SvgDocument::setContent(
        const QString &text, bool namespaceProcessing, QString *errorMsg, int *errorLine, int *errorColumn) {
    return svg_doc_.setContent(text, namespaceProcessing, errorMsg, errorLine, errorColumn);
}

bool SvgDocument::setContent(
        QIODevice *dev, bool namespaceProcessing, QString *errorMsg, int *errorLine, int *errorColumn) {
    return svg_doc_.setContent(dev, namespaceProcessing, errorMsg, errorLine, errorColumn);
}


bool SvgDocument::setContent(const QByteArray &text, QString *errorMsg, int *errorLine, int *errorColumn) {
    return svg_doc_.setContent(text, errorMsg, errorLine, errorColumn);
}

bool SvgDocument::setContent(const QString &text, QString *errorMsg, int *errorLine, int *errorColumn) {
    return svg_doc_.setContent(text, errorMsg, errorLine, errorColumn);
}

bool SvgDocument::setContent(QIODevice *dev, QString *errorMsg, int *errorLine, int *errorColumn) {
    return svg_doc_.setContent(dev, errorMsg, errorLine, errorColumn);
}

QByteArray SvgDocument::contents() const noexcept {
    return svg_doc_.toByteArray();
}

QDomDocument SvgDocument::document() const noexcept {
    return svg_doc_;
}

SvgRenderer SvgDocument::toRenderer(QObject *parent) const {
    return SvgRenderer(svg_doc_.toByteArray(), parent);
}
