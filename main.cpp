#include<cstddef>
#include <iostream>
#include <map>
#include<memory>
#include<tuple>

using namespace std;

struct BadOptionalAccess {
};

template <typename T>
class Optional {
private:
    alignas(T) unsigned char data[sizeof(T)];
    bool defined = false;

public:
    Optional() = default;
    Optional(const T& elem) {
        new (data) T(elem);
        defined = true;
    }
    Optional(T && elem) {
        new (data) T(std::move(elem));
        defined = true;
    }
    Optional(const Optional& other) {
        if (other.has_value()) {
            new (data) T(other.value());
            defined = true;
        }
    }
    Optional& operator=(const Optional& other) {
        if (data == other.data) {
            return *this;
        }
        if (defined) {
            if (other.has_value()) {
                value() = other.value();
            } else {
                reset();
            }
        } else {
            if (other.has_value()) {
                new (data) T(other.value());
                defined = true;
            }
        }
        return *this;
    }
    Optional& operator=(const T& elem) {
        if (defined) {
            value() = elem;
        } else {
            new (data) T(elem);
            defined = true;
        }
        return *this;
    }
    Optional& operator=(T&& elem) {
        if (defined) {
            value() = std::move(elem);
        } else {
            new (data) T(std::move(elem));
            defined = true;
        }
        return *this;
    }

    bool has_value() const {
        return defined;
    }

    T& operator*() {
        if (!defined) {
            throw BadOptionalAccess();
        } else {
            return *reinterpret_cast<T*>(data);
        }
    }
    const T& operator*() const {
        if (!defined) {
            throw BadOptionalAccess();
        } else {
            return *reinterpret_cast<const T*>(data);
        }
    }

    T* operator->() {
        if (!defined) {
            throw BadOptionalAccess();
        }
        return reinterpret_cast<T*>(data);
    }
    const T* operator->() const {
        if (!defined) {
            throw BadOptionalAccess();
        }
        return reinterpret_cast<const T*>(data);
    }

    T& value() {
        if (!defined) {
            throw BadOptionalAccess();
        }
        return *reinterpret_cast<T*>(data);
    }
    const T& value() const {
        if (!defined) {
            throw BadOptionalAccess();
        }
        return *reinterpret_cast<const T*>(data);
    }

    void reset() {
        if (defined) {
            value().~T();
            defined = false;
        }
    }

    ~Optional() {
        reset();
    }
};

template <typename T>
class List {
private:
    size_t sz = 0;

    struct Elem {
        Optional<T> val;
        Elem* next = nullptr;
        Elem* prev = nullptr;
        Elem() = default;
        Elem(const T& el) : val(el) {}
    };
    class Iterator {
    private:
        const Elem* elem = nullptr;

    public:
        Iterator() = default;
        Iterator& operator++() {
            elem = elem->next;
            return *this;
        }
        Iterator operator++(int) {
            const Elem* tmp = elem;
            elem = elem->next;
            return tmp;
        }
        Iterator& operator--() {
            elem = elem->prev;
            return *this;
        }
        Iterator operator--(int) {
            const Elem* tmp = elem;
            elem = elem->prev;
            return tmp;
        }
        const T&operator*() const {
            return elem->val.value();
        }
        bool operator == (Iterator it) const {
            return elem == it.elem;
        }
        bool operator != (Iterator it) const {
            return elem != it.elem;
        }

        Iterator(const Elem* el) : elem(el) {}
    };

    Elem base;

    void push(Elem* elem, const T& tp) {
        Elem* el = new Elem(tp);
        elem->next->prev = el;
        el->next = elem->next;
        el->prev = elem;
        elem->next = el;
    }
    void pop(Elem* elem) {
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;
        delete elem;
    }

public:
    List() {
        base.next = base.prev = &base;
    }
    size_t size() const {
        return sz;
    }
    size_t size() {
        return sz;
    }
    void push_back(const T& a) {
        push(base.prev, a);
        sz++;
    }
    void push_front(const T& a) {
        push(&base, a);
        sz++;
    }
    void pop_front() {
        pop(base.next);
        sz--;
    }
    void pop_back() {
        pop(base.prev);
        sz--;
    }
    Iterator begin() {
        return Iterator(base.next);
    }
    Iterator end() {
        return Iterator(&base);
    }
    ~List() {
        while (sz > 0) {
            pop_back();
        }
    }
};
