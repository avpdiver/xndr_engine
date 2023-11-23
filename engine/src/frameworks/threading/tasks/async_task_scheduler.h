#pragma once

namespace Be::Framework::Threading
{

    enum class EThreadType : uint8_t
    {
        eMain,
        eBackground,
        ePerformance
    };

    class AsyncTaskScheduler final : public Noninstanceable
    {
    public:
        static void Create(uint32_t thread_count = 0) noexcept;
        static void Destroy() noexcept;

    public:
        static void Start() noexcept;
        static void Stop() noexcept;

    public:
        [[nodiscard]] static AsyncTask *CreateTask(const AsyncTaskFunction &task_func = AsyncTask::EmptyFunction, AsyncTask *parent_task = nullptr) noexcept;

    public:
        static void Schedule(AsyncTask *task, EThreadType thread_type = EThreadType::ePerformance) noexcept;

    public:
        static void Wait(AsyncTask *task) noexcept;

        template <typename R, typename P>
        static bool Wait(AsyncTask *task, const std::chrono::duration<R, P> &timeout) noexcept;

    private:
        static void RunThread() noexcept;
        static bool ExecuteTask() noexcept;

    private:
        static AsyncTask *AllocateTask() noexcept;
    };

}