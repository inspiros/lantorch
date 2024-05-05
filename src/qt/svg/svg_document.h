#pragma once

#include <QDomDocument>
#include "SvgRenderer"

class SvgDocument {
    QDomDocument svg_doc_;

public:
    SvgDocument();

    explicit SvgDocument(const QDomDocument &document);

    explicit SvgDocument(const QString &filename);

    explicit SvgDocument(QXmlStreamReader *contents, bool namespaceProcessing = true);

    SvgDocument &apply(const std::function<void(QDomElement &)> &f, bool recursive = true);

    [[nodiscard]] SvgDocument clone() const;

    bool setContent(
            const QByteArray &text, bool namespaceProcessing, QString *errorMsg = nullptr, int *errorLine = nullptr,
            int *errorColumn = nullptr);

    bool setContent(
            const QString &text, bool namespaceProcessing, QString *errorMsg = nullptr, int *errorLine = nullptr,
            int *errorColumn = nullptr);

    bool setContent(
            QIODevice *dev, bool namespaceProcessing, QString *errorMsg = nullptr, int *errorLine = nullptr,
            int *errorColumn = nullptr);

    bool setContent(
            const QByteArray &text, QString *errorMsg = nullptr, int *errorLine = nullptr, int *errorColumn = nullptr);

    bool setContent(
            const QString &text, QString *errorMsg = nullptr, int *errorLine = nullptr, int *errorColumn = nullptr);

    bool setContent(
            QIODevice *dev, QString *errorMsg = nullptr, int *errorLine = nullptr, int *errorColumn = nullptr);

    [[nodiscard]] QByteArray contents() const noexcept;

    [[nodiscard]] QDomDocument document() const noexcept;

    SvgRenderer toRenderer(QObject *parent = nullptr) const;
};
