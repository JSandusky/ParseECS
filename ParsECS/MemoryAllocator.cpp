#include "MemoryAllocator.h"

#include <algorithm>
#include <assert.h>

const unsigned char MemoryChunk::PATTERN_ALIGN = 0xFC;
const unsigned char MemoryChunk::PATTERN_ALLOC = 0xFD;
const unsigned char MemoryChunk::PATTERN_FREE = 0xFE;

MemoryChunk::MemoryChunk(unsigned short pos, unsigned short length)
{
    position_ = pos;
    length_ = length;
    flags_ = 0;
    used_ = 0;
}

MemoryPage::MemoryPage(unsigned short pageSize)
{
    pageSize_ = freeBytes_ = pageSize;
    address_ = malloc(pageSize);
    usedBytes_ = 0;
    
    // Fill with allocation pattern
    memset(address_, MemoryChunk::PATTERN_ALLOC, pageSize_);

    chunks_.insert_tail(new MemoryChunk(0, pageSize));
}

MemoryPage::~MemoryPage()
{
    Clear();
    if (chunks_.head())
        delete chunks_.remove_head();
}

void* MemoryPage::Allocate(size_t size, size_t minimumSize)
{
    auto current = chunks_.head();
    while (current)
    {
        size = std::max(size, minimumSize);
        if (current->used_ == 0 && current->length_ > size)
        {
            const size_t finalSize = size + sizeof(size_t);
            MemoryChunk* newAllocChunk = new MemoryChunk(current->position_, finalSize);
            chunks_.insert_before(newAllocChunk, current);

            current->position_ += newAllocChunk->length_; //guard byte
            current->length_ -= newAllocChunk->length_; // guard byte

            newAllocChunk->writeGuardByte(address_);
            newAllocChunk->used_ = 1;
            freeBytes_ -= finalSize;
            usedBytes_ += finalSize;
            return (char*)address_ + newAllocChunk->position_;
        }
        current = chunks_.next(current);
    }
    return 0x0;
}

void MemoryPage::Free(void* memory)
{
    auto current = chunks_.head();
    while (current)
    {
        if (current->used_ == 1 && ((char*)address_ + current->position_) == memory)
        {
            assert(current->checkGuardByte(address_));

            freeBytes_ += current->length_;
            usedBytes_ -= current->length_;
            current->freeData(address_); // mark data as freed
            current->used_ = 0; // mark node as free
            Coalesce(current);
            return;
        }
        current = chunks_.next(current);
    }
}

void MemoryPage::Clear()
{
    if (address_)
        free(address_);
    freeBytes_ = pageSize_;
    usedBytes_ = 0;
    address_ = 0x0;
    while (chunks_.tail() && chunks_.tail() != chunks_.head())
        delete chunks_.remove_tail();
}

MemoryChunk* MemoryPage::Coalesce(MemoryChunk* node)
{
    // Collapse the next node back onto us?
    if (auto nextChunk = chunks_.next(node))
    {
        if (nextChunk->used_ == 0)
        {
            node->length_ += nextChunk->length_;
            chunks_.remove(nextChunk);
            delete nextChunk;
        }
    }

    // Collapse ourself into the previous chunk?
    if (auto prevChunk = chunks_.prev(node))
    {
        if (prevChunk->used_ == 0)
        {
            prevChunk->length_ += node->length_;
            chunks_.remove(node);
            delete node;
            return prevChunk;
        }
    }
    return node;
}

void MemoryPage::Clean()
{
    auto current = chunks_.head();
    while (current)
    {
        if (current->used_ == 0)
            current = Coalesce(current);
        if (current)
            current = chunks_.next(current);
    }
}

MemoryMan::MemoryMan(unsigned short pageSize, unsigned short minimumBlockSize) :
    pageSize_(pageSize),
    minimumBlockSize_(minimumBlockSize)
{
    pages_.insert_head(new MemoryPage(pageSize_));
}

MemoryMan::MemoryMan(unsigned short pageCount, unsigned short pageSize, unsigned short minimumBlockSize) :
    pageSize_(pageSize),
    minimumBlockSize_(minimumBlockSize)
{
    for (unsigned i = 0; i < pageCount; ++i)
        pages_.insert_tail(new MemoryPage(pageSize_));
}

MemoryMan::~MemoryMan()
{

}

void* MemoryMan::Allocate(size_t size)
{
    auto page = pages_.head();
    while (page)
    {
        // Is the page valid and if so could we even possible fit?
        if (page->address_ && page->freeBytes_ > std::max(size, (size_t)minimumBlockSize_))
        {
            if (void* alloc = page->Allocate(size, minimumBlockSize_))
                return alloc;
        }
        page = pages_.next(page);
    }

    pages_.insert_tail(new MemoryPage(pageSize_));
    return pages_.tail()->Allocate(size, minimumBlockSize_);
}

void* MemoryMan::Free(void* memory)
{
    auto page = pages_.head();
    while (page)
    {
        if (page->Contains(memory))
        {
            page->Free(memory);
            return 0x0;
        }
        page = pages_.next(page);
    }
    return memory;
}

void MemoryMan::Clear()
{
    while (pages_.tail() && pages_.tail() != pages_.head())
        delete pages_.remove_tail();
    pages_.head()->Clear();
}

void MemoryMan::ClearPages()
{
    auto head = pages_.head();
    while (head)
    {
        head->Clear();
        head = pages_.next(head);
    }
}

void MemoryMan::SetPageCount(unsigned short pageCount)
{
    auto head = pages_.head();
    int ct = 0;
    while (head) 
    {
        ++ct;
        head = pages_.next(head);
    }

    while (ct > pageCount)
    {
        delete pages_.remove_tail();
        --ct;
    }

    while (ct < pageCount)
    {
        pages_.insert_tail(new MemoryPage(pageSize_));
        ++ct;
    }
}