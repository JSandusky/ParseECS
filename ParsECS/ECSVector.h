#pragma once

#include <vector>

template<typename T, std::enable_if<(std::is_pointer<T>::value)> >
struct SimpleECSVectorAlloc
{
    T Create() {
        return new T();
    }

    T InPlaceCreate() {
        return Create();
    }

    void Free(T ptr)
    {
        delete ptr;
    }

    T InPlaceFree(T ptr)
    {
        delete ptr;
        return 0x0;
    }
};

template<typename T>
class ECSVector : protected std::vector<T>
{
public:
    
    ECSVector() { }
    ECSVector(SimpleECSVectorAlloc<T>* allocator) { setAllocator(allocator); }
    virtual ~ECSVector() { clear(); }

    void setAllocator(SimpleECSVectorAlloc<T>* allocator) { allocator_ = allocator; }

    // Move scope of std:vector parts to expose
    using std::vector<T>::operator[];
    using std::vector<T>::front;
    using std::vector<T>::capacity;

    size_t size() const { return listEnd_; }

    inline T& end() { (*this)[listEnd_ - 1]; }

    T& GetNew() {
        if (listEnd_ < size())
        {
            (*this)[listEnd_] = allocator_->InPlaceCreate();
            T& ret = (*this)[listEnd_];
            ++listEnd_;
            return ret;
        }
        Grow();
        return GetNew();
    }

    void erase(size_t index) {
        (*this)[index] = allocator_->InPlaceFree((*this)[index]);
        std::swap((*this)[index], (*this)[listEnd_ - 1]);
        --listEnd_;
    }

    void grow() {
        const size_t currentPos = size();
        resize(::log(size()));
        for (unsigned i = currentPos; i < size(); ++i)
            (*this)[i] = allocator_->Create();
    }

    void clear() {
        for (unsigned i = 0; i < std::vector<T>::size(); ++i)
            allocator_->Free((*this)[i]);
        std::vector<T>::clear();
        listEnd_ = 0x0;
    }

private:
    size_t listEnd_ = 0;
    SimpleECSVectorAlloc<T>* allocator_ = 0x0;
};