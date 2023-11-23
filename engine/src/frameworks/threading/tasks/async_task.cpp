#include "frameworks/threading/threading.h"

namespace Be::Framework::Threading
{
    void AsyncTask::Execute() noexcept
    {
        PROFILER_SCOPE;
        
        try
        {
            m_function();
        }
        catch (...)
        {            
        }
        if (m_parent)
        {
            m_parent->ChildCompleted();
        }
    }

}