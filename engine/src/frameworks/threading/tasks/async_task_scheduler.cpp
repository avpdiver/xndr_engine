#include "frameworks/threading/threading.h"
#include "concurrentqueue.h"

namespace Be::Framework::Threading
{
    void SetAffinity(size_t i)
    {
#ifdef BE_PLATFORM_LINUX
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        const auto res = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        VERIFY(res == 0, "Failed to set affinity of current thread #{}: {}", i, res);
#else
        const auto res = SetThreadAffinityMask(GetCurrentThread(), DWORD_PTR(1) << i);
        VERIFY(res == 0, "Failed to set affinity of current thread #{}: {}", i, GetLastError());
#endif
    }

    static constexpr usize_t MAX_TASKS_COUNT = 4096;

    using AsyncTaskConcurrentQueue = moodycamel::ConcurrentQueue<AsyncTask *>;

    namespace SchedulerState
    {
        uint32_t thread_count{0}; // number of threads in the pool
        uint32_t main_thread_index{0};
        uint32_t background_thread_index{0};
        uint32_t performance_thread_index{0};
        uint32_t performance_thread_count{0};

        Atomic<uint32_t> thread_start_counter{0}; // Counted down when started
        Atomic<uint32_t> thread_stop_counter{0};  // Counted down when started

        std::condition_variable wait_for_job_cv{};

        AtomicFlag running;
    }

    namespace ThreadState
    {
        // thread local access
        thread_local Mutex mutex{};
        thread_local uint32_t thread_index{0}; // each thread has its own number
        thread_local usize_t task_pool_index{0};
        thread_local AsyncTask task_pool[MAX_TASKS_COUNT];
        thread_local EThreadType thread_type{EThreadType::ePerformance};

        // concurrent access
        FixedArray<AsyncTaskConcurrentQueue, 32> parents_queues;
        FixedArray<AsyncTaskConcurrentQueue, 32> children_queues;

        AsyncTask *AllocateTask() noexcept
        {
            PROFILER_SCOPE;

            if (task_pool_index >= MAX_TASKS_COUNT) [[unlikely]]
            {
                FATAL("Too much AsyncTask created");
            }
            const auto index = task_pool_index++;
            auto job_ptr = &task_pool[index & (MAX_TASKS_COUNT - 1u)]; // Ring buffer, no need to dealocate job
            return job_ptr;
        }

        AsyncTask *GetQueuedTask(uint32_t index) noexcept
        {
            PROFILER_SCOPE;

            AsyncTask *task{nullptr};

            if (ThreadState::children_queues[index].try_dequeue(task))
            {
                return task;
            }

            if (ThreadState::parents_queues[index].try_dequeue(task))
            {
                if (task->ReadyForExecute())
                {
                    return task;
                }
                else
                {
                    ThreadState::parents_queues[thread_index].enqueue(task);
                    return nullptr;
                }
            }

            return nullptr;
        }

        AsyncTask *StealTask(uint32_t index) noexcept
        {
            const auto start_index = ThreadState::thread_type == EThreadType::ePerformance ? SchedulerState::performance_thread_index : 0;

            AsyncTask *task{nullptr};
            auto steal_index = index;
            auto num_try = (ThreadState::thread_type == EThreadType::ePerformance ? SchedulerState::performance_thread_count : SchedulerState::thread_count) - 1;
            while (task == nullptr && --num_try > 0)
            {
                if (++steal_index >= SchedulerState::thread_count)
                {
                    steal_index = start_index;
                }
                task = GetQueuedTask(steal_index);
            }
            return task;
        }
    }

    void AsyncTaskScheduler::Create(uint32_t thread_count) noexcept
    {
        SchedulerState::thread_count = thread_count;
        if (SchedulerState::thread_count == 0)
        {
            SchedulerState::thread_count = ThreadUtils::MaxThreadCount();
            if (SchedulerState::thread_count == 0)
            {
                SchedulerState::thread_count = 1;
            }
        }

        LOG_INFO("AsyncTaskScheduler: Threads number {}.", SchedulerState::thread_count);

        SchedulerState::main_thread_index = 0;
        SchedulerState::background_thread_index = 1;
        SchedulerState::performance_thread_index = std::min(uint32_t(2), uint32_t(SchedulerState::thread_count - 1));
        SchedulerState::performance_thread_count = SchedulerState::thread_count - SchedulerState::performance_thread_index;

        LOG_INFO("AsyncTaskScheduler is inited.");
    }

    void AsyncTaskScheduler::Destroy() noexcept
    {
        Stop();

        LOG_INFO("AsyncTaskScheduler is destroyed.");
    }

    void AsyncTaskScheduler::Start() noexcept
    {
        ThreadUtils::ResetThreadIndecies();

        SchedulerState::main_thread_index = ThreadUtils::GetCurrentThreadIndex();
        SchedulerState::thread_start_counter = SchedulerState::thread_count - 1;
        SchedulerState::thread_stop_counter = SchedulerState::thread_count - 1;
        SchedulerState::running.test_and_set();

        for (uint32_t i = 1; i < SchedulerState::thread_count; i++)
        {
            std::thread t{RunThread};
            t.detach();
        }

        while (SchedulerState::thread_start_counter.load() > 0)
        {
            ThreadUtils::YieldOrSleep();
        }
    }

    void AsyncTaskScheduler::Stop() noexcept
    {
        SchedulerState::running.clear();
        while (SchedulerState::thread_stop_counter != 0)
        {
            ThreadUtils::Yield();
        }
    }

    bool AsyncTaskScheduler::ExecuteTask() noexcept
    {
        PROFILER_SCOPE;

        auto task = ThreadState::GetQueuedTask(ThreadState::thread_index);
        if (task == nullptr)
        {
            return false;
        }

        task->Execute();

        return true;
    }

    void AsyncTaskScheduler::RunThread() noexcept
    {
        ThreadState::thread_index = ThreadUtils::GetCurrentThreadIndex();
        SetAffinity(ThreadState::thread_index);

        std::unique_lock mutex_lock{ThreadState::mutex};
        SchedulerState::thread_start_counter--;

        LOG_INFO("Thread #{} created.", ThreadState::thread_index);

        while (SchedulerState::running.test())
        {
            if (!ExecuteTask())
            {
                SchedulerState::wait_for_job_cv.wait_for(mutex_lock, Microseconds(100));
            }
        }

        AsyncTask *task;
        while (ThreadState::children_queues[ThreadState::thread_index].try_dequeue(task))
        {
        }
        while (ThreadState::parents_queues[ThreadState::thread_index].try_dequeue(task))
        {
        }

        LOG_INFO("Thread #{} destroyed.", ThreadState::thread_index);

        SchedulerState::thread_stop_counter--;
    }

    AsyncTask *AsyncTaskScheduler::AllocateTask() noexcept
    {
        return ThreadState::AllocateTask();
    }

    void AsyncTaskScheduler::Schedule(AsyncTask *task, EThreadType thread_type) noexcept
    {
        PROFILER_SCOPE;

        const auto is_parent = (task->m_children.load(std::memory_order_relaxed) > 1);

        auto thread_index = SchedulerState::main_thread_index;

        if (thread_type == EThreadType::eBackground)
        {
            thread_index = SchedulerState::background_thread_index;
        }
        else if (thread_type == EThreadType::ePerformance)
        {
            thread_index = (rand() % SchedulerState::performance_thread_count) + SchedulerState::performance_thread_index;
        }

        if (is_parent)
        {
            ThreadState::parents_queues[thread_index].enqueue(task);
        }
        else
        {
            ThreadState::children_queues[thread_index].enqueue(task);
        }

        // wake up the thread
        SchedulerState::wait_for_job_cv.notify_all();
    }

    AsyncTask *AsyncTaskScheduler::CreateTask(const AsyncTaskFunction &task_func, AsyncTask *parent_task) noexcept
    {
        PROFILER_SCOPE;

        return PlacementNew<AsyncTask>(ThreadState::AllocateTask(), task_func, parent_task);
    }

    void AsyncTaskScheduler::Wait(AsyncTask *task) noexcept
    {
        PROFILER_SCOPE;

        if (!task || task->IsFinished())
        {
            return;
        }

        // Wake any threads that might be sleeping
        SchedulerState::wait_for_job_cv.notify_all();

        while (task->IsFinished())
        {
            if (!ExecuteTask()) // no task executed
            {
                ThreadUtils::Pause();
            }
        }
    }

    template <typename R, typename P>
    bool AsyncTaskScheduler::Wait(AsyncTask *task, const std::chrono::duration<R, P> &timeout) noexcept
    {
        PROFILER_SCOPE;

        if (!task || task->IsFinished())
        {
            return true;
        }

        // Wake any threads that might be sleeping
        SchedulerState::wait_for_job_cv.notify_all();

        const auto start_time = std::chrono::high_resolution_clock::now();

        while (task->IsFinished())
        {
            if (!ExecuteTask()) // no task executed
            {
                ThreadUtils::Pause();
            }

            if (std::chrono::high_resolution_clock::now() - start_time >= timeout)
            {
                return false;
            }
        }

        return true;
    }
    
}