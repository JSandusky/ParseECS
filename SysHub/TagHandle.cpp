#include "TagHandle.h"
#include "Serializer.h"

void Test()
{
    TagDataArray<int, 32, 0xCDCD> datums;
    int val = *datums.Get(0);

    PolymorphicDatumArray<32, int, float> datums2;
}

namespace Organism
{

    void TagFile::WriteHeader(Serializer* src)
    {
        RIFF::WriteHeader(src);
        src->Serialize(flags_);
        src->Serialize(priority_);
        src->Serialize(language_);
        src->Serialize(unused1_);
        src->Serialize(unused2_);
    }

    bool TagFile::ReadHeader(Serializer* src)
    {
        bool ret = RIFF::ReadHeader(src);
        ret &= src->Serialize(flags_);
        ret &= src->Serialize(priority_);
        ret &= src->Serialize(language_);
        ret &= src->Serialize(unused1_);
        ret &= src->Serialize(unused2_);
        return ret;
    }

}