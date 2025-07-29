#include "Serializer.h"

#include <memory>
#include <algorithm>

BufferSerializer::BufferSerializer(void* buffer, size_t bufferSize, bool writeMode) :
    buffer_(buffer),
    bufferSize_(bufferSize),
    writeMode_(writeMode)
{

}

size_t BufferSerializer::Serialize(void* data, size_t size)
{
    if (!buffer_ || !bufferSize_ || bufferSize_ <= bufferPos_)
        return 0;
    
    size_t bytes = std::max(size, bufferSize_ - bufferPos_);
    if (writeMode_)
        memcpy(buffer_, data, bytes);
    else
        memcpy(data, buffer_, bytes);
    bufferPos_ += bytes;
    return bytes;
}

size_t BufferSerializer::Seek(size_t newPosition)
{
    return (bufferPos_ = std::min(newPosition, bufferSize_));
}

VectorSerializer::VectorSerializer() : 
    writeMode_(true)
{

}

VectorSerializer::VectorSerializer(const std::vector<unsigned char>& buffer, bool writeMode)
{
    buffer_ = buffer;
    writeMode_ = writeMode;
}

VectorSerializer::VectorSerializer(void* buffer, size_t bufferSize, bool writeMode)
{
    buffer_.resize(bufferSize);
    memcpy(buffer_.data(), buffer, bufferSize);
    writeMode_ = writeMode;
}

size_t VectorSerializer::Serialize(void* dest, size_t size)
{
    if (writeMode_)
    {
        if (size + bufferPos_ > buffer_.size())
            buffer_.resize(buffer_.size() + size + bufferPos_);
    }
    else
    {
        if (size + bufferPos_ > buffer_.size())
            size = buffer_.size() - bufferPos_;
        if (!size)
            return 0;
    }

    unsigned char* srcPtr = &buffer_[bufferPos_];
    unsigned char* destPtr = (unsigned char*)dest;

    if (writeMode_)
        std::swap(srcPtr, destPtr);

    bufferPos_ += size;

    unsigned copySize = size;
    memcpy(destPtr, srcPtr, size);
    //while (copySize >= sizeof(unsigned))
    //{
    //    *((unsigned*)destPtr) = *((unsigned*)srcPtr);
    //    srcPtr += sizeof(unsigned);
    //    destPtr += sizeof(unsigned);
    //    copySize -= sizeof(unsigned);
    //}
    //if (copySize & sizeof(unsigned short))
    //{
    //    *((unsigned short*)destPtr) = *((unsigned short*)srcPtr);
    //    srcPtr += sizeof(unsigned short);
    //    destPtr += sizeof(unsigned short);
    //}
    //if (copySize & 1)
    //    *destPtr = *srcPtr;

    return size;
}

size_t VectorSerializer::Seek(size_t newPosition)
{
    return bufferPos_ = newPosition;
}