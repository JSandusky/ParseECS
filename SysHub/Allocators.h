#pragma once

#include <algorithm>
#include <vector>

/// Simple frame allocator that can be used for loose data.
/// To use just call StartFrame() before each frame's initial usage.
/// Then allocate objects from it, discard them freely.
class FrameAllocator
{
public:
    FrameAllocator(unsigned size)
    {
        size_ = size;
        data_ = new unsigned char[size_];
        position_ = 0;
    }

    virtual ~FrameAllocator()
    {
        if (data_)
            delete[] data_;
    }

    /// Resets the position back to zero.
    void StartFrame() 
    { 
        position_ = 0; memset(data_, 0, size_); 
    }

    /// Note that this should never be called during a frame.
    void Resize(unsigned newSize)
    {
        if (newSize == size_)
            return;

        delete data_;
        size_ = newSize;
        data_ = new unsigned char[size_];
        position_ = 0;
    }

    unsigned char* Allocate(unsigned bytes)
    {
        if (bytes + position_ > size_)
            return 0x0;
        unsigned char* ret = data_;
        position_ += bytes;
    }

    template<typename T>
    T* New(unsigned count = 1)
    {
        // Can't allocate
        if (count * sizeof(T) + position_ > size_)
            return 0x0;

        T* ret = (T*)data_;
        position_ += sizeof(T) * count;
        return ret;
    }

private:
    unsigned char* data_ = 0x0;
    unsigned size_ = 0;
    unsigned position_ = 0;
};


/// Generic object pool
///     Requries: default constructor
/// The implementation details of this pool place favour on New instead of Delete,
///     however the iteration to find the position in the pointers_ table for swapping with the last element should
///     compare favorably against a STL based approach is vector/stack
/// Default constructor and destructor are both used to free any resources in the object.
template<typename T>
class PagingObjectPool
{
private:
    struct PageData
    {
        unsigned pageSize_ = 0;
        unsigned available_ = 0;
        T* data_;
        T** pointers_;

        PageData(unsigned size)
        {
            available_ = size;
            pageSize_ = size;
            data_ = new T[size];
            pointers_ = new T*[size];
            for (unsigned i = 0; i < size; ++i)
                pointers_[i] = &(data_ + i);
        }

        /// Destruct.
        ~PageData()
        {
            delete[] data_;
            delete[] pointers_;
        }

        inline bool Contains(T* object)
        {
            return data_ <= object && &(data_ + pageSize_) > object;
        }

        T* Get()
        {
            // just grab the next one available
            T* ret = pointers_[pageSize_ - available_];
            available_--;
            new (ret) T(); // default constructor in place
            return ret;
        }

        bool Free(T* object)
        {
            /// find out where this object is in the pointers table
            for (unsigned i = 0; i < pageSize_; ++i)
            {
                if (pointers_[i] == object)
                {
                    object->~T(); // make sure RAII and such all work
                    // swap the guy at the back with what we have
                    std::swap(pointers_[pageSize_ - available_ - 1], pointers_[i]);
                    --available_;
                    return true;
                }
            }
            return false;
        }
    };

public:
    /// Construct with desired size and with N initial pages.
    ObjectPool(unsigned pageSize, unsigned initialPageCount = 1) :
        pageSize_(pageSize)
    {
        for (unsigned i = 0; i < initialPageCount; ++i)
            pages_.push_back(PageData(pageSize_));
    }

    /// Destruct. std::vector will do the work.
    virtual ~ObjectPool()
    {

    }

    /// Allocate a new object, if this ever fails then there's no memory available.
    T* New()
    {
        for (unsigned i = 0; i < pages_.size(); ++i)
        {
            if (pages_[i].available_)
                return pages_[i]->Get();
        }

        pages_.push_back(PageData(pageSize_));
        return pages_.back().Get();
    }

    /// Free the given object, returns false if it doesn't belong to this ObjectPool
    bool Free(T* object)
    {
        for (unsigned i = 0; i < pages_.size(); ++i)
        {
            if (pages_[i].data_ <= object && (pages_[i].data_ + pageSize_) > object)
                return pages_[i]->Free(object);
        }
        return false;
    }

    /// Convenience overload.
    inline bool Delete(T* object) { return Free(object); }

private:
    /// List of pages in the pool.
    std::vector<PageData> pages_;
    /// Number of objects in each page.
    unsigned pageSize_ = 64;
};