#include "../MemoryAllocator.h"

//void TestAllocator()
//{
//
//    MemoryMan* manager = new MemoryMan(1, 512, 64);
//
//    void* values[32];
//    memset(values, 0, sizeof values);
//    for (unsigned i = 0; i < 32; ++i)
//    {
//        values[i] = manager->Allocate(32);
//    }
//
//    for (unsigned i = 0; i < 32; ++i)
//        manager->Free(values[i]);
//
//    delete manager;
//
//}