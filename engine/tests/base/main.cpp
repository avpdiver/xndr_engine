#include "unit_tests_common.h"

extern void UnitTest_Mallocs();
extern void UnitTest_Allocators();

int main()
{
    UnitTest_Mallocs();
    UnitTest_Allocators();
    
    return 0;
}