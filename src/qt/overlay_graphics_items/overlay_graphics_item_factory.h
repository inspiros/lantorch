#pragma once

#include <type_traits>

#include <QGraphicsObject>
#include "OverlayGraphicsItem"

class OverlayGraphicsItemFactory : public QObject {
Q_OBJECT
protected:
    OverlayGraphicsView *view_ = nullptr;
    QMap<OverlayGraphicsItem *, QSharedPointer<OverlayGraphicsItem>> items_;

public:
    explicit OverlayGraphicsItemFactory(OverlayGraphicsView *view, QObject *parent = nullptr);

    ~OverlayGraphicsItemFactory() override;

    void setView(OverlayGraphicsView *view);

    [[nodiscard]] OverlayGraphicsView *view() const;

    [[nodiscard]] QObject *overlayed_object() const;

    template<typename T, typename... Args,
            typename std::enable_if<std::is_convertible_v<T *, OverlayGraphicsItem *>, bool>::type = true>
    inline QSharedPointer<std::remove_all_extents_t<T>> createItem(Args &&...args) {
        auto item = QSharedPointer<std::remove_all_extents_t<T>>(
                new std::remove_all_extents_t<T>(view_, std::forward<Args>(args)...));
        addItem(item);
        return item;
    }

    void addItem(const QSharedPointer<OverlayGraphicsItem> &item);

    void addItem(OverlayGraphicsItem *item);

    int removeItem(const QSharedPointer<OverlayGraphicsItem> &item);

    int removeItem(OverlayGraphicsItem *item);

    void clear();

protected:
    template<class DerivedT>
    static inline auto castItem(OverlayGraphicsItem *src) {
        return qgraphicsitem_cast<DerivedT *>(src);
    }

    template<class DerivedT>
    static inline QSharedPointer<DerivedT> castItem(
            const QSharedPointer<OverlayGraphicsItem> &src) {
        if constexpr (std::is_same_v<DerivedT, OverlayGraphicsItem>) {
            return src;
        } else {
            auto *ptr = qgraphicsitem_cast<DerivedT *>(src.data());
            return QtSharedPointer::copyAndSetPointer(ptr, src);
        }
    }
};
