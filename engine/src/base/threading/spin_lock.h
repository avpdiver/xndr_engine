#pragma once

namespace Be
{

    class SpinMutex final : public Noncopyable
    {
    public:
        forceinline void Lock()
        {
            while (m_lock.test_and_set(std::memory_order_acquire)) // acquire lock
            {
#if defined(__cpp_lib_atomic_flag_test)
                while (m_lock.test(std::memory_order_relaxed)) // test lock
#endif
                    ;
            }
        }
        forceinline void Unlock()
        {
            m_lock.clear(std::memory_order_release);
        }
    private:
        std::atomic_flag m_lock{ATOMIC_FLAG_INIT};
    };

    class SpinLock final : public Noncopyable
    {
    public:
        explicit forceinline SpinLock(SpinMutex &mutex) : m_mutex{mutex}
        {
            m_mutex.Lock();
        }
        forceinline ~SpinLock()
        {
            m_mutex.Unlock();
        }
    private:
        SpinMutex &m_mutex;
    };

}