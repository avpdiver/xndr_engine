#include "frameworks/threading/threading.h"

#include "../shared/unit_test_shared.h"

using namespace Be;
using namespace Be::Framework::Threading;

void UnitTest_AsyncTaskScheduler()
{
    AsyncTaskScheduler::Create();

    {
        AsyncTaskScheduler::Start();

        auto task = AsyncTaskScheduler::CreateTask([]
                                                   { LOG_INFO("Execute 1 task"); });

        LOG_INFO("Schedule ParallelFor task 1");
        AsyncTaskScheduler::Schedule(task);

        LOG_INFO("Wait ParallelFor task 1");
        AsyncTaskScheduler::Wait(task);

        AsyncTaskScheduler::Stop();
    }

    {
        AsyncTaskScheduler::Start();

        auto task = AsyncTaskScheduler::CreateTask([]
                                                   { LOG_INFO("Execute 2 task"); });

        LOG_INFO("Schedule ParallelFor task 2");
        AsyncTaskScheduler::Schedule(task);

        LOG_INFO("Wait ParallelFor task 2");
        AsyncTaskScheduler::Wait(task);

        AsyncTaskScheduler::Stop();
    }

    AsyncTaskScheduler::Destroy();

    TEST_PASSED();
}

int main()
{
    UnitTest_AsyncTaskScheduler();
    return 0;
}