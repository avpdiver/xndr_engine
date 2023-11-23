#include "base/base.h"

#include <emmintrin.h>

namespace Be::ThreadUtils
{

    static Atomic<usize_t> s_thread_counter{0};
    static thread_local usize_t s_thread_index{MaxValue};

    usize_t GetCurrentThreadIndex() noexcept
    {
        if (s_thread_index < usize_t(MaxValue))
        {
            return s_thread_index;
        }

        s_thread_index = s_thread_counter.fetch_add(1, std::memory_order_seq_cst);

        return s_thread_index;
    }

    void ResetThreadIndecies() noexcept
    {
        s_thread_index = MaxValue;
        s_thread_counter.store(0, std::memory_order_seq_cst);
    }

    void Pause() noexcept
    {
        _mm_pause();
    }

}