#pragma once

#include <QList>
#include <QMutex>
#include <QMutexLocker>

template<typename T>
class ConstantSizedList : public QList<T> {
public:
    inline ConstantSizedList() noexcept: QList<T>() {}

    inline explicit ConstantSizedList(int max_size) noexcept: QList<T>(), max_size_(std::max(0, max_size)) {}

    explicit ConstantSizedList(const QList<T> &l) : QList<T>(l) {}

    explicit ConstantSizedList(int max_size, const QList<T> &l) : QList<T>(), max_size_(std::max(0, max_size)) {
        append(l);
    }

    inline int size() {
        QMutexLocker locker(&mutex_);
        return QList<T>::size();
    }

    [[nodiscard]] inline int maxSize() const noexcept {
        return max_size_;
    }

    [[nodiscard]] inline bool isFull() {
        return max_size_ && this->size() == max_size_;
    }

    void reserve(int size);

    void resizeTrimFirst(int size);

    void resizeTrimLast(int size);

    void append(const T &t);

    void append(const QList<T> &t);

    void prepend(const T &t);

    void insertTrimFirst(int i, const T &t);

    void insertTrimLast(int i, const T &t);

    using iterator = typename QList<T>::iterator;
    using const_iterator = typename QList<T>::const_iterator;

    iterator insertRemoveFirst(iterator before, const T &t);

    iterator insertRemoveLast(iterator before, const T &t);

    void removeFirst();

    void removeLast();

protected:
    void trimFirst();

    void trimLast();

private:
    using QList<T>::insert;
    int max_size_ = 0;
    QRecursiveMutex mutex_;
};

template<typename T>
using ConstSizedList = ConstantSizedList<T>;

template<typename T>
void ConstantSizedList<T>::reserve(int size) {
    QList<T>::reserve(max_size_ ? std::min(max_size_, size) : size);
}

template<typename T>
void ConstantSizedList<T>::resizeTrimFirst(int size) {
    int diff = size - max_size_;
    if (diff < 0) {
        QMutexLocker locker(&mutex_);
        this->erase(this->begin(), this->begin() - diff);
    }
    max_size_ = size;
}

template<typename T>
void ConstantSizedList<T>::resizeTrimLast(int size) {
    int diff = size - max_size_;
    if (diff < 0) {
        QMutexLocker locker(&mutex_);
        this->erase(this->end() + diff, this->end());
    }
    max_size_ = size;
}

template<typename T>
void ConstantSizedList<T>::append(const T &t) {
    QMutexLocker locker(&mutex_);
    QList<T>::append(t);
    trimFirst();
}

template<typename T>
void ConstantSizedList<T>::append(const QList<T> &t) {
    QMutexLocker locker(&mutex_);
    QList<T>::append(t);
    trimFirst();
}

template<typename T>
void ConstantSizedList<T>::prepend(const T &t) {
    QMutexLocker locker(&mutex_);
    QList<T>::prepend(t);
    trimLast();
}

template<typename T>
void ConstantSizedList<T>::insertTrimFirst(int i, const T &t) {
    QMutexLocker locker(&mutex_);
    QList<T>::insert(i, t);
    trimFirst();
}

template<typename T>
void ConstantSizedList<T>::insertTrimLast(int i, const T &t) {
    QMutexLocker locker(&mutex_);
    QList<T>::insert(i, t);
    trimLast();
}

template<typename T>
typename ConstantSizedList<T>::iterator ConstantSizedList<T>::insertRemoveFirst(
        ConstantSizedList<T>::iterator before, const T &t) {
    QMutexLocker locker(&mutex_);
    QList<T>::insert(before, t);
    trimFirst();
}

template<typename T>
typename ConstantSizedList<T>::iterator ConstantSizedList<T>::insertRemoveLast(
        ConstantSizedList<T>::iterator before, const T &t) {
    QMutexLocker locker(&mutex_);
    QList<T>::insert(before, t);
    trimLast();
}

template<typename T>
void ConstantSizedList<T>::removeFirst() {
    QMutexLocker locker(&mutex_);
    QList<T>::removeFirst();
}

template<typename T>
void ConstantSizedList<T>::removeLast() {
    QMutexLocker locker(&mutex_);
    QList<T>::removeLast();
}

template<typename T>
void ConstantSizedList<T>::trimFirst() {
    while (isFull())
        this->removeFirst();
}

template<typename T>
void ConstantSizedList<T>::trimLast() {
    while (isFull())
        this->removeLast();
}
