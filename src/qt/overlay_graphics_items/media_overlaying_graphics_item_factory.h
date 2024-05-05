#pragma once

#include <type_traits>

#include <QGraphicsObject>
#include "MediaOverlayingGraphicsItem"
#include "MediaOverlayingGraphicsItemFactory"

class MediaOverlayingGraphicsItemFactory : public QObject {
Q_OBJECT
protected:
    OverlayGraphicsView *view_ = nullptr;
    QMap<MediaOverlayingGraphicsItem *, QSharedPointer<MediaOverlayingGraphicsItem>> items_;

public:
    explicit MediaOverlayingGraphicsItemFactory(OverlayGraphicsView *view, QObject *parent = nullptr);

    ~MediaOverlayingGraphicsItemFactory() override;

    void setView(OverlayGraphicsView *view);

    [[nodiscard]] OverlayGraphicsView *view() const;

    [[nodiscard]] MediaWidget *overlayed_media_widget() const;

    template<typename T, typename... Args,
            typename std::enable_if<std::is_convertible_v<T *, MediaOverlayingGraphicsItem *>, bool>::type = true>
    inline QSharedPointer<std::remove_all_extents_t<T>> createItem(Args &&...args) {
        auto item = QSharedPointer<std::remove_all_extents_t<T>>(
                new std::remove_all_extents_t<T>(view_, std::forward<Args>(args)...));
        addItem(item);
        return item;
    }

    void addItem(const QSharedPointer<MediaOverlayingGraphicsItem> &item);

    void addItem(MediaOverlayingGraphicsItem *item);

    int removeItem(const QSharedPointer<MediaOverlayingGraphicsItem> &item);

    int removeItem(MediaOverlayingGraphicsItem *item);

    void clear();

protected:
    template<class DerivedT>
    static inline auto castItem(MediaOverlayingGraphicsItem *src) {
        return qgraphicsitem_cast<DerivedT *>(src);
    }

    template<class DerivedT>
    static inline QSharedPointer<DerivedT> castItem(
            const QSharedPointer<MediaOverlayingGraphicsItem> &src) {
        if constexpr (std::is_same_v<DerivedT, MediaOverlayingGraphicsItem>) {
            return src;
        } else {
            auto *ptr = qgraphicsitem_cast<DerivedT *>(src.data());
            return QtSharedPointer::copyAndSetPointer(ptr, src);
        }
    }
};
