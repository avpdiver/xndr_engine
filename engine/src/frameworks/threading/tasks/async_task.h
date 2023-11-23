#pragma once

namespace Be::Framework::Threading
{

    class AsyncTaskScheduler;
    class AsyncTask;

    using AsyncTaskFunction = std::function<void()>;

    class alignas(BE_CACHE_LINE) AsyncTask final
    {
        friend class AsyncTaskScheduler;

    public:
        static constexpr auto EmptyFunction = []() {};

    public:
        AsyncTask() noexcept
            : m_function{EmptyFunction},
              m_parent{nullptr},
              m_children{1} // self

        {
        }

        AsyncTask(const AsyncTaskFunction &func, AsyncTask *parent = nullptr) noexcept
            : m_function{func},
              m_parent{parent},
              m_children{1} // self
        {
            if (m_parent != nullptr)
            {
                m_parent->AddChild();
            }
        }

    public:
        [[nodiscard]] forceinline bool IsFinished() const noexcept
        {
            return (m_children.load(std::memory_order::relaxed) == 0);
        }

        [[nodiscard]] forceinline bool ReadyForExecute() const noexcept
        {
            return (m_children.load(std::memory_order::relaxed) == 1);
        }

    public:
        [[nodiscard]] forceinline void *GetData()
        {
            return m_data;
        }

    private:
        void Execute() noexcept;

    private:
        forceinline void AddChild() noexcept
        {
            PROFILER_SCOPE;

            m_children.fetch_add(1, std::memory_order::relaxed);
        }

        forceinline void ChildCompleted() noexcept
        {
            PROFILER_SCOPE;
            
            m_children.fetch_sub(1, std::memory_order::relaxed);
        }

    private:
        byte_t m_data[64]; // 64 bytes

    private:
        AsyncTaskFunction m_function;   // 32 bytes
        AsyncTask *m_parent{nullptr};   // 8 bytes
        Atomic<uint32_t> m_children{0}; // 4 bytes

        // 108 bytes
    };

}