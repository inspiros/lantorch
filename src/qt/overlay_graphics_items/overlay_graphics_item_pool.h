#pragma once

#include <QException>
#include <QPointer>
#include <utility>

#include "OverlayGraphicsItem"

class InsufficientPoolSizeException : public QException {
public:
    explicit InsufficientPoolSizeException(QString message = "") noexcept
            : message(std::move(message)) {}

    InsufficientPoolSizeException(const InsufficientPoolSizeException &re) {
        this->message = re.message;
    }

    ~InsufficientPoolSizeException() override = default;

    void raise() const override {
        throw *this;
    }

    [[nodiscard]] InsufficientPoolSizeException *clone() const override {
        return new InsufficientPoolSizeException(*this);
    }

    [[nodiscard]] const char *what() const noexcept override {
        return this->message.toStdString().c_str();
    }

private:
    QString message;
};

template<typename ItemType,
        typename std::enable_if<std::is_convertible_v<ItemType *, OverlayGraphicsTrait *>, bool>::type = true>
class OverlayGraphicsItemPool : public QObject {
public:
    OverlayGraphicsView *view_ = nullptr;
    QList<QSharedPointer<ItemType>> items_;
    int n_active_items_ = 0;

public:
    explicit OverlayGraphicsItemPool(OverlayGraphicsView *view, QObject *parent = nullptr)
            : QObject(parent), view_(view) {}

    inline OverlayGraphicsView *view() {
        return view_;
    }

    inline void set_view(OverlayGraphicsView *view) {
        view_ = view;
    }

    inline QList<QSharedPointer<ItemType>> items() {
        return items_.mid(0);
    }

    [[nodiscard]] inline int size() const {
        return items_.size();
    }

    template<typename... Args>
    void resize(int n, Args &&...args) {
        items_.reserve(n);
        if (n > items_.size()) {
            for (auto i = items_.size(); i < n; i++) {
                auto item = QSharedPointer<ItemType>(new ItemType(view_, std::forward<Args>(args)...));
                item->setVisible(false);
                items_.push_back(item);
            }
        } else {
            items_.resize(n);
        }
    }

    template<typename Factory, typename... Args>
    void factory_resize(int n, Factory factory, Args &&...args) {
        items_.reserve(n);
        if (n > items_.size()) {
            for (auto i = items_.size(); i < n; i++) {
                auto item = factory->template createItem<ItemType>(std::forward<Args>(args)...);
                item->setVisible(false);
                items_.push_back(item);
            }
        } else {
            for (auto i = 0; i < items_.size() - n; i++) {
                items_.removeLast();
            }
        }
    }

    template<typename... Args>
    inline void expand(int n, Args &&...args) {
        resize(items_.size() + n, std::forward<Args>(args)...);
    }

    template<typename Factory, typename... Args>
    inline void factory_expand(int n, Factory factory, Args &&...args) {
        factory_resize(items_.size() + n, std::forward<Factory>(factory), std::forward<Args>(args)...);
    }

    QList<QSharedPointer<ItemType>> request(int n) {
        checkAvailable(n);
        for (auto i = 0; i < n; i++) {
            items_[i]->setVisible(true);
        }
        if (n < n_active_items_) {
            for (auto iter = items_.begin() + n;
                 iter != items_.begin() + n_active_items_;
                 iter = std::next(iter)) {
                (*iter)->setVisible(false);
            }
        }
        n_active_items_ = n;
        return items_.mid(0, n);
    }

    template<typename UpdateFunc>
    QList<QSharedPointer<ItemType>> submit(int n, UpdateFunc f) {
        checkAvailable(n);
        for (auto i = 0; i < n; i++) {
            auto item = items_[i];
            f(i, item);
            item->setVisible(true);
        }
        if (n < n_active_items_) {
            for (auto iter = items_.begin() + n;
                 iter != items_.begin() + n_active_items_;
                 iter = std::next(iter)) {
                (*iter)->setVisible(false);
            }
        }
        n_active_items_ = n;
        return items_.mid(0, n);
    }

    inline void addToScene(QGraphicsScene *scene) {
        for (auto it = items_.rbegin(); it != items_.rend(); it++) {
            scene->addItem((*it).data());
        }
    }

    inline void removeFromScene(QGraphicsScene *scene) {
        for (const auto &it: items_)
            scene->removeItem(it.data());
    }

protected:
    inline void checkAvailable(int n) {
        if (n > items_.size())
            throw InsufficientPoolSizeException(
                    QString("%d items requested, while pool size is currently %d").arg(n).arg(items_.size()));
    }
};
