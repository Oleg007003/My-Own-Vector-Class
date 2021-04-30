#include <algorithm>
#include <cstdint>
#include <iostream>
#include <math.h>
#include <memory>

template <typename T>
struct RawMemory {
    T* buf = nullptr;
    size_t rsv = 0;
    static T* Allocate(size_t n) {
        return static_cast<T*>(operator new(n * sizeof(T)));
    }

    static void Deallocate(T* buffer) {
        operator delete(buffer);
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buf, other.buf);
        std::swap(rsv, other.rsv);
    }
    RawMemory() = default;

    RawMemory(size_t n) {
        buf = Allocate(n);
        rsv = n;
    }

    RawMemory(const RawMemory&) = delete;

    RawMemory(RawMemory&& other) {
        Swap(other);
    }

    RawMemory& operator = (const RawMemory&) = delete;

    RawMemory& operator = (RawMemory&& other) noexcept {
        Swap(other);
        return *this;
    }

    ~RawMemory() {
        Deallocate(buf);
    }

    T* operator + (size_t i) {
        return buf + i;
    }
    const T* operator + (size_t i) const {
        return buf + i;
    }
    T&operator[] (size_t i) {
        return buf[i];
    }
    const T&operator[] (size_t i) const {
        return buf[i];
    }
};

template <typename T>
class Vector {
private:
    RawMemory<T> data;
    size_t sz = 0;

    static void Construct(void * buf) {
        new (buf) T();
    }
    static void Construct(void * buf, const T& elem) {
        new (buf) T(elem);
    }
    static void Construct(void * buf, const T&& elem) {
        new (buf) T(std::move(elem) );
    }
    static void Destroy(T* buf) {
        buf->~T();
    }


public:
    Vector() = default;
    explicit Vector(size_t n) : data(n) {
        std::uninitialized_value_construct_n(data.buf, n);
        sz = n;
    }
    Vector(const Vector& v) : data(v.sz) {
        std::uninitialized_copy_n(v.data.buf, v.sz, data.buf);
        sz = v.sz;
    }
    Vector(Vector&& v) noexcept {
        swap(v);
    }
    void swap(Vector& v) noexcept {
        data.Swap(v.data);
        std::swap(sz, v.sz);
    }
    size_t capacity() const {
        return data.rsv;
    }
    size_t size() const {
        return sz;
    }
    Vector& operator = (const Vector& other) {
        if (other.sz > data.rsv) {
            Vector tmp(other);
            swap(tmp);
        } else {
            for (size_t i = 0; i < sz && i < other.sz; i++) {
                data[i] = other[i];
            }
            if (sz < other.sz) {
                std::uninitialized_copy_n(other.data + sz, other.sz - sz, data.buf + sz);
            }
            if (sz > other.sz) {
                std::destroy_n(data.buf + other.sz, sz - other.sz);
            }
            sz = other.sz;
        }
        return *this;
    }
    Vector& operator = (Vector&& other) noexcept {
        swap(other);
        return *this;
    }
    T& operator[] (size_t i) {
        return data.buf[i];
    }
    const T& operator[] (size_t i) const {
        return data.buf[i];
    }
    void push_back(const T& a) {
        if (sz == data.rsv) {
            reserve(sz == 0 ? 1 : sz * 2);
        }
        new (data + sz) T(a);
        sz++;
    }
    void push_back(T&& a) {
        if (sz == data.rsv) {
            reserve(sz == 0 ? 1 : sz * 2);
        }
        new (data + sz) T(std::move(a));
        sz++;
    }
    void pop_back() {
        std::destroy_at(data + sz - 1);
        --sz;
    }
    void clear() {
        resize(0);
    }
    void reserve(size_t n) {
        if (n > data.rsv) {
            RawMemory<T> data2(n);
            std::uninitialized_move_n(data.buf, sz, data2.buf);
            std::destroy_n(data.buf, sz);
            data.Swap(data2);
        }
    }
    void resize(size_t n) {
        reserve(n);
        if (sz < n) {
            std::uninitialized_value_construct_n(data + sz, n - sz);
        } else if (sz > n) {
            std::destroy_n(data + n, sz - n);
        }
        sz = n;
    }
    T* begin() {
        return data.buf;
    }
    const T* begin() const {
        return data.buf;
    }
    T* end() {
        return data + sz;
    }
    const T* end() const {
        return data + sz;
    }
    ~Vector() {
        std::destroy_n(data.buf, sz);
    }
};
