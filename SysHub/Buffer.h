#pragma once

struct Buffer
{
    void* data_;
    size_t size_;
};

/// Safe iteration and ranged-based-for on arrays
template<typename T>
struct Span
{
    T* start_; // actually the current element
    T* end_; // actualy the last element

    Span(T* start, size_t count) { start_ = start; end_ = start_ + count; }
    Span(T* start, T* end) { start_ = start; end_ = end; }

    bool IsFinished() const { return start_ > end_; }
    T* begin() { return start_; }
    T* begin() const { return start_; }

    T* end() { return end_ + 1; }
    T* end() const { return end_ + 1; }

    T* operator++() {
        return start_ += 1;
    }

    T& operator*() { return *start_; }
    const T& operator*() const { return *start_; }
};