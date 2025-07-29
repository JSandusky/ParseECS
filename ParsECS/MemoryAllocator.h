#pragma once

#include "list.h"

#include <cstdint>
#include <memory>

struct MemoryChunk
{
    static const unsigned char PATTERN_ALIGN;
    static const unsigned char PATTERN_ALLOC;
    static const unsigned char PATTERN_FREE;

    unsigned short position_;
    unsigned short length_;
    unsigned flags_ : 24;
    unsigned used_ : 8;
    list_node<MemoryChunk> instrusiveList_;

    MemoryChunk(unsigned short pos, unsigned short length);

    inline void* startAddress(void* relativeTo) { return (char*)relativeTo + position_; }
    inline void* endAddress(void* relativeTo, bool withGuard = true) { return (char*)relativeTo + position_ + length_ - (withGuard ? 0 : sizeof(size_t)); }
    inline bool checkGuardByte(void* relativeTo) { return memcmp((char*)relativeTo + position_ + length_, &PATTERN_ALIGN, sizeof(unsigned char)) == 0; }
    inline void writeGuardByte(void* relativeTo) { memset((char*)relativeTo + position_ + length_, PATTERN_ALIGN, sizeof(size_t)); }
    inline void freeData(void* relativeTo) { memset((char*)relativeTo + position_, PATTERN_FREE, length_); }
};

struct MemoryPage
{
    list_node<MemoryPage> instrusiveList_;
    list<MemoryChunk, &MemoryChunk::instrusiveList_> chunks_;
    unsigned short pageSize_;
    unsigned short freeBytes_;
    unsigned short usedBytes_ = 0;
    void* address_;

    /// Construct for a given page size
    MemoryPage(unsigned short pageSize);
    /// Destruct and release everything.
    ~MemoryPage();

    /// Allocate a block of memory that is the larger of size / minimumSize.
    /// Returns 0x0 if there was no room to allocate.
    void* Allocate(size_t size, size_t minimumSize);
    /// Free a block of memory that was previously allocated.
    void Free(void* memory);
    /// Wipes all allocation history to a clean slate.
    void Clear();

    /// Returns true if the given address is contained within this page's block.
    inline bool Contains(void* memory) { return memory >= (char*)address_ && memory < (char*)address_ + pageSize_; }

    /// Will collapse any free chunks before or after this one into a contiguous free block.
    /// If the given node collapsed onto the previous node the return value will be that previous node, otherwise it will be given node
    MemoryChunk* Coalesce(MemoryChunk* node);

    /// Iterates thorugh the Page invoking Coalesce on all unallocated nodes to check for the opportunity to collapse.
    void Clean();
};

struct MemoryMan
{  
    /// List of all of the pages in the memory manager.
    list<MemoryPage, &MemoryPage::instrusiveList_> pages_;
    /// Specifies the size of pages in the manager.
    unsigned short pageSize_;
    /// Specifies the minimum size of a memory block that may be allocated within in a page.
    unsigned short minimumBlockSize_;

    MemoryMan(unsigned short pageSize, unsigned short minimumBlockSize = 63);
    MemoryMan(unsigned short pageCount, unsigned short pageSize, unsigned short minimumBlockSize = 63);
    ~MemoryMan();

    /// Allocates a block of memory of at least 'size' bytes.
    void* Allocate(size_t size);
    /// Frees the given memory, returns 0x0 if freed
    /// Otherwise it returns the given value as it is not contained in here.
    void* Free(void* memory);
    /// Erases all content.
    void Clear();
    /// Clears only the data in the pages, leaves the count of pages pending.
    void ClearPages();
    /// Will add or remove pages until at the desired count.
    void SetPageCount(unsigned short pageCount);
};