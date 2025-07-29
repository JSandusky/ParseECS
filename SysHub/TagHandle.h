#pragma once

#include "RIFF.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <unordered_map>

typedef uint32_t StringID;

struct TagFile
{

};

struct TagDatabase
{
    /// Sorted by priorities
    std::vector<TagFile*> tagFiles_;
};

template<typename T>
struct TagReference
{
    T** pointer_;
    T* Get(TagDatabase* database = 0x0) 
    {
        if (pointer_)
        {
            if (T* deref = *pointer_)
                return deref;
        }
        else if (id_ && database)
        {
            if (pointer_ = Resolve(database))
                return *pointer_;
        }
        return 0x0;
    }
    
    T** Resolve(TagDatabase* database)
    {
        return 0x0;
    }
    StringID id_ = 0;
};

struct TagHandle
{
#define TAG_NULL_INDEX 0x3FFF
    union {
        struct {
            unsigned type_ : 6;     // Up to 64 top level types
            unsigned counter_ : 12; // Up to 4095 max
            unsigned index_ : 14;   // up to 16k max
        };
        unsigned uintValue_;
    };

    template<typename T>
    T* GetObject() {
        if (index_ != TAG_NULL_INDEX)
            return 0x0;
        return (_GetDataList() + index_);
    }

    /// Gets the object list for this tag
    template<typename T>
    T* _GetDataList()
    {
        return 0x0;
    }

    void stuff()
    {
    }
};

template<typename T>
struct TagDataArrayHandle
{
    int16_t salt_ = 0;
    uint16_t index_ = 0;
};

/// A DatumArray is a fixed length container whose elements may exist or not.
/// Code generation also has path specifically for handling DatumArrays
template<typename T, uint16_t SIZE, uint16_t SALT>
struct TagDataArray
{
    static const uint32_t FreeAddrMark = 0xFEEEFEEE;
    static const uint32_t ElementSize = sizeof(T) + sizeof(uint16_t);
    static const uint32_t BUFFER_SIZE = SIZE * sizeof(T) + sizeof(uint16_t); // Useful for doing bulk memcpy

    struct DatumRecord
    {
        /// Salt is a confirmation that something really does exist here
        int16_t salt_;
        T element_;

        bool IsValid() { return salt_ < 0; }
        void Clear() { memset(this, 0, sizeof(DatumRecord)); }
    };
    
    /// Keep a running total, use "swap and pop" when we free
    int16_t salt_ = SALT;
    uint16_t elementCount_ = 0;
    DatumRecord buffer_[SIZE];

    TagDataArray()
    {
        elementCount_ = 0;
        memset(buffer_, 0, BUFFER_SIZE);
    }

    ~TagDataArray()
    {
        elementCount_ = 0;
        memset(buffer_, 0, BUFFER_SIZE);
    }

    T* Get(unsigned index)
    {
        assert(Has(index));
        if (!Has(index))
            return (T*)&buffer_[index];
        return 0x0;
    }

    T& GetRef(unsigned index)
    {
        assert(Has(index));
        return buffer_[index];
    }

    T* Allocate()
    {
        assert(elementCount_ < SIZE);
        if (elementCount_ < SIZE)
        {
            buffer_[elementCount_].salt_ = NextSalt();
            T* ret = &buffer_[elementCount_].element_;
            ++elementCount_;
            return ret;
        }
        return 0x0;
    }

    void Free(unsigned index)
    {
        assert(Has(index));
        if (Has(index))
        {
            buffer_[index].salt_ = 0x0;
            memset(&buffer_[index], FreeAddrMark, sizeof(DatumRecord));

            // Could do a sort and pop
            if (index < elementCount_ - 1)
                memcpy(&buffer_[index], &buffer_[index + 1], sizeof(T) * (elementCount_ - index - 1));

            --elementCount_;
        }
    }

    void Set(unsigned index, const T& value)
    {
        if (!Has(index))
        {
            if (T* allocated = Allocate(index))
                *allocated = value;
        }
        else
            *Get(index) = value;
    }

    inline void SwapElements(unsigned lhs, unsigned rhs)
    {
        assert(Has(lhs) && Has(rhs));
        std::swap(buffer_[lhs], buffer_[rhs]);
    }

    inline bool Has(unsigned index)
    {
        return index < elementCount_;
    }

    inline int16_t NextSalt()
    {
        return (++salt_) | 0x8000; // keep the negative bit on at all times
    }
};

//template<typename...T>
//const uint32_t LargestTypeSize<T...>::Largest = std::max(sizeof(T)...);

/// Unlike a DatumArray that uses fixed size salted identifiers, the PolymorphicDatumArray
/// Can use any DECLARE_TAG() type
template<uint16_t SIZE, typename...T>
struct PolymorphicDatumArray
{
    static uint32_t LargestSize() { return std::max(sizeof(T)...); }
    // Salt indicates our type
    unsigned char buffer_[SIZE * sizeof(uint16_t)];

};

namespace Organism
{
    struct TagIndex {
        uint32_t tagType_;
        uint32_t tagID_;
        uint32_t offset_;
    };

    struct TagChunk : public RIFFChunk
    {
    protected:
        
    };

    struct TagIndexChunk : public RIFFChunk
    {
        TagIndex* GetIndices() { return (TagIndex*)data_; }
        uint32_t GetIndexCount() const { return size_ / sizeof(TagIndex); }

        void BuildIndex(RIFF* tagFile)
        {
            std::vector<TagIndex> indices;

            tagFile->CalculateOffsets();
            for (unsigned i = 0; i < tagFile->GetChunkCount(true); ++i)
            {
                if (auto tagGroup = tagFile->GetChunk(i))
                {
                    auto groupID = tagGroup->type_;
                    // Group of tags of a given type
                    if (tagFile->GetChunk(i)->IsList())
                    {
                        for (unsigned t = 0; t < ((RIFF*)tagGroup)->GetChunkCount(false); ++t)
                        {
                            auto tagChunk = ((RIFF*)tagGroup)->GetChunk(t);
                            TagIndex index;
                            memcpy(&index.tagType_, groupID, sizeof(uint32_t));
                            memcpy(&index.tagID_, tagChunk->type_, sizeof(uint32_t));
                            index.offset_ = tagChunk->offset_;
                            indices.push_back(index);
                        }
                    }
                }
            }
        }
    };

    struct TagFile : public RIFF
    {
        uint32_t flags_;
        uint32_t priority_;
        uint32_t language_;
        uint32_t unused1_;
        uint32_t unused2_;
    protected:
        virtual RIFFChunk* CreateChunk(const char* typeID)
        {
            if (strcmp(typeID, "INDX") == 0)
                return new TagIndexChunk();
            return RIFF::CreateChunk(typeID);
        }
        /// Header methods are abstracted so that potentially extra data could be included in derived types.
        virtual unsigned GetHeaderSize() const { return 8 + sizeof(uint32_t) * 5; }
        /// Header methods are abstracted so that potentially extra data could be included in derived types.
        virtual void WriteHeader(Serializer* src);
        /// Header methods are abstracted so that potentially extra data could be included in derived types.
        virtual bool ReadHeader(Serializer* src);
    };
}