#pragma once

namespace Be::Framework::Threading
{

    struct ParallelForData final
    {
        void *data;
        usize_t count;
    };

    template <typename T>
    AsyncTask *ParallelFor(T *data, usize_t count, usize_t tasks, const AsyncTaskFunction &func, EThreadType thread_type = EThreadType::ePerformance) noexcept
    {
        PROFILER_SCOPE;

        const ssize_t per_task = std::max(count / tasks, usize_t(1));
        ssize_t rest = count;

        auto root_task = AsyncTaskScheduler::CreateTask();
        while (rest > 0)
        {
            auto task = AsyncTaskScheduler::CreateTask(func, root_task);
            auto tdata = static_cast<ParallelForData *>(task->GetData());
            tdata->data = data;
            tdata->count = std::min(per_task, rest);

            AsyncTaskScheduler::Schedule(task, thread_type);

            rest -= per_task;
            data += per_task;
        }

        AsyncTaskScheduler::Schedule(root_task, thread_type);
        return root_task;
    }

}