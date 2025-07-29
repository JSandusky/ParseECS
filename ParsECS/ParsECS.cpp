// ParsECS.cpp : Defines the entry point for the console application.
//

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include "Components/Component.h"
#include "Entities/Entity.h"
#include "Components/ComponentRegistry.h"

#include "MemoryAllocator.h"

struct TestCompState
{
    float value_;
};

struct TestComp : public Component<TestCompState>
{
    float hpLowerBound_;
    float hpUpperBound_;
};
const CompID TestComp::TypeID = 1;

struct Stuff {
    float stuff;
    float stuff2;
    float third;
};

struct SecondComp : public Component < Stuff >
{
    float stuff;
    float stuff2;
    float third;
};
const CompID SecondComp::TypeID = 4;

void TestInitialization()
{
    REGISTER_COMPONENT(TestComp, TestCompState);
    BEGIN_METADATA(TestComp);
        BEGIN_COMPONENT_PROPERTIES();
            REGISTER_PROPERTY_MEMORY(TestComp, float, offsetof(TestComp, hpLowerBound_), 0.0f, "Lower Bound", "", 0);
            REGISTER_PROPERTY_MEMORY(TestComp, float, offsetof(TestComp, hpUpperBound_), 0.0f, "Upper Bound", "", 0);
        END_PROPERTIES();
    END_METADATA();
}


void TestAllocator()
{

    MemoryMan* manager = new MemoryMan(1, 512, 64);

    void* values[32];
    memset(values, 0, sizeof values);
    for (unsigned i = 0; i < 32; ++i)
    {
        values[i] = manager->Allocate(32);
    }

    for (unsigned i = 0; i < 32; ++i)
        manager->Free(values[i]);

    delete manager;

}


int _tmain(int argc, _TCHAR* argv[])
{
    TestInitialization();

    TestAllocator();

    const size_t* scan = ExclusiveScan<TestComp, SecondComp>::offsets;
//    const size_t* secondScan = ExclusiveScan<TestComp, SecondComp>::pos;

	return 0;
}

