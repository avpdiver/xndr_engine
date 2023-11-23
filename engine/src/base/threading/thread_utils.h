#pragma once

#define EXCLUSIVE_LOCK(SYNC_OBJ) \
    std::scoped_lock BE_CONCAT_RAW(ex_lock_, __COUNTER__) { SYNC_OBJ }
#define SHARED_LOCK(SYNC_OBJ) \
    std::shared_lock BE_CONCAT_RAW(shared_lock_, __COUNTER__) { SYNC_OBJ }
#define EXCLUSIVE_LOCK_DEFERRED(SYNC_OBJ) \
    std::unique_lock BE_CONCAT_RAW(ex_lock_, __COUNTER__) { SYNC_OBJ, std::defer_lock }
#define SHARED_LOCK_DEFERRED(SYNC_OBJ) \
    std::shared_lock BE_CONCAT_RAW(shared_lock_, __COUNTER__) { SYNC_OBJ, std::defer_lock }

namespace Be
{
    using ThreadID = std::thread::id;
    using ThreadHandle = std::thread::native_handle_type;
    using Mutex = std::mutex;
    using SharedMutex = std::shared_mutex;
    using RecursiveMutex = std::recursive_mutex;
    using ConditionVariable = std::condition_variable;

    namespace ThreadUtils
    {
        inline constexpr uint32_t SpinBeforeLock = 1'000;
        inline constexpr uint32_t LargeSpinBeforeLock = 10'000;

        forceinline void InvalidateCpuCache() noexcept
        {
            return std::atomic_thread_fence(std::memory_order::acquire);
        }

        forceinline void FlushCpuCache() noexcept
        {
            return std::atomic_thread_fence(std::memory_order::release);
        }

        forceinline void NoReorderPrevCode() noexcept
        {
            return std::atomic_signal_fence(std::memory_order::acquire);
        }

        forceinline void NoReorderNextCode() noexcept
        {
            return std::atomic_signal_fence(std::memory_order::release);
        }

        [[nodiscard]] forceinline auto GetID() noexcept
        {
            return std::this_thread::get_id();
        }

        [[nodiscard]] forceinline uint64_t GetIntID() noexcept
        {
            return uint64_t(HashOf(std::this_thread::get_id()));
        }

        [[nodiscard]] usize_t GetCurrentThreadIndex() noexcept;
        void ResetThreadIndecies() noexcept;

        [[nodiscard]] forceinline uint32_t MaxThreadCount() noexcept
        {
            return std::thread::hardware_concurrency();
        }

        template <typename R, typename P>
        forceinline void Sleep(const std::chrono::duration<R, P> &relative_time) noexcept
        {
            std::this_thread::sleep_for(relative_time);
        }

        forceinline bool Yield() noexcept
        {
#ifdef BE_PLATFORM_WINDOWS
            return SwitchToThread();
#elif BE_PLATFORM_LINUX
            std::this_thread::yield();
            return true;
#endif
        }

        template <typename R, typename P>
        forceinline void YieldOrSleep(const std::chrono::duration<R, P> &relative_time) noexcept
        {
            if (!Yield())
            {
                Sleep(relative_time);
            }
        }

        forceinline void YieldOrSleep() noexcept
        {
            if (!Yield())
            {
                Sleep(Milliseconds{1});
            }
        }

        void Pause() noexcept;

    }
}

#include "base/threading/spin_lock.h"