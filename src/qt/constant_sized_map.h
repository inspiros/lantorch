#pragma once

#include <QMap>
#include <QMutex>
#include <QMutexLocker>

template<class Key, class T>
class ConstantSizedMap : public QMap<Key, T> {
public:
    inline ConstantSizedMap() noexcept: QMap<Key, T>() {}

    inline explicit ConstantSizedMap(int max_size) noexcept: QMap<Key, T>(), max_size_(max_size) {}

    inline ConstantSizedMap(std::initializer_list<std::pair<Key, T> > list) : QMap<Key, T>(list) {}

    inline ConstantSizedMap(int max_size, std::initializer_list<std::pair<Key, T> > list)
            : QMap<Key, T>(), max_size_(max_size) {
        for (typename std::initializer_list<std::pair<Key, T> >::const_iterator it = list.begin();
             it != list.end(); ++it)
            insert(it->first, it->second);
    }

    explicit ConstantSizedMap(const QMap<Key, T> &other) : QMap<Key, T>(other) {
        trim();
    }

    inline ~ConstantSizedMap() {
        QMutexLocker locker(&mutex_);
    }

    inline ConstantSizedMap<Key, T> &operator=(const QMap<Key, T> &other) {
        QMutexLocker locker(&mutex_);
        if (this->d != other.d) {
            QMap<Key, T> tmp(other);
            tmp.swap(*this);
        }
        trim();
        return *this;
    }

    inline ConstantSizedMap<Key, T> &operator=(QMap<Key, T> &&other) noexcept {
        QMutexLocker locker(&mutex_);
        QMap moved(std::move(other));
        swap(moved);
        trim();
        return *this;
    }

    inline bool operator==(const QMap<Key, T> &other) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::operator==(other);
    }

    inline bool operator!=(const QMap<Key, T> &other) { return !(*this == other); }

    inline int size() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::size();
    }

    inline bool isEmpty() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::isEmpty();
    }

    [[nodiscard]] inline int maxSize() const noexcept {
        return max_size_;
    }

    [[nodiscard]] inline bool isFull() {
        return this->size() >= max_size_;
    }

    inline void resize(int size) {
        max_size_ = size;
        trim();
    }

    inline void clear() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::clear();
    }

    inline int remove(const Key &akey) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::remove(akey);
    }

    inline T take(const Key &akey) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::take(akey);
    }

    inline bool contains(const Key &key) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::contains(key);
    }

    inline const Key key(const T &value, const Key &defaultKey = Key()) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::key(value, defaultKey);
    }

    inline const T value(const Key &key, const T &defaultValue = T()) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::value(key, defaultValue);
    }

    inline T &operator[](const Key &key) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::operator[](key);
    }

    inline QList<Key> keys() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::keys();
    }

    inline QList<Key> keys(const T &value) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::keys(value);
    }

    inline QList<T> values() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::values();
    }

    inline int count(const Key &key) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::count();
    }

    using iterator = typename QMap<Key, T>::iterator;
    using const_iterator = typename QMap<Key, T>::const_iterator;
    using key_iterator = typename QMap<Key, T>::key_iterator;
    using const_key_value_iterator = typename QMap<Key, T>::const_key_value_iterator;
    using key_value_iterator = typename QMap<Key, T>::key_value_iterator;

    inline iterator begin() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::begin();
    }

    inline const_iterator constBegin() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::constBegin();
    }

    inline const_iterator cbegin() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::cbegin();
    }

    inline iterator end() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::end();
    }

    inline const_iterator constEnd() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::constEnd();
    }

    inline const_iterator cend() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::cend();
    }

    inline iterator rend() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::begin() - 1;
    }

    inline key_iterator keyBegin() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::keyBegin();
    }

    inline key_iterator keyEnd() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::keyEnd();
    }

    inline key_value_iterator keyValueBegin() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::keyValueBegin();
    }

    inline key_value_iterator keyValueEnd() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::keyValueEnd();
    }

    inline const_key_value_iterator constKeyValueBegin() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::constKeyValueBegin();
    }

    inline const_key_value_iterator constKeyValueEnd() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::constKeyValueEnd();
    }

    inline iterator erase(iterator it) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::erase(it);
    }

    inline int count() {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::count();
    }

    iterator find(const Key &key) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::find(key);
    }

    const_iterator constFind(const Key &key) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::constFind(key);
    }

    iterator lowerBound(const Key &key) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::lowerBound(key);
    }

    iterator upperBound(const Key &key) {
        QMutexLocker locker(&mutex_);
        return QMap<Key, T>::upperBound(key);
    }

    inline iterator insert(const Key &key, const T &value) {
        QMutexLocker locker(&mutex_);
        auto it = QMap<Key, T>::insert(key, value);
        trim();
        return it;
    }

    inline void insert(const QMap<Key, T> &map) {
        QMutexLocker locker(&mutex_);
        QMap<Key, T>::insert(map);
        trim();
    }

protected:
    inline void trim() {
        QMutexLocker locker(&mutex_);
        while (isFull())
            this->remove(this->firstKey());
    }

    inline QMutexLocker locker() {
        return QMutexLocker(&mutex_);
    }

private:
    using QMap<Key, T>::swap;

    iterator insert(const_iterator pos, const Key &key, const T &value) {}

    using QMap<Key, T>::insertMulti;
    using QMap<Key, T>::unite;

    int max_size_ = 0;
    QRecursiveMutex mutex_;
};
