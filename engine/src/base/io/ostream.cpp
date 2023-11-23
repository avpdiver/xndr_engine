#include "base/base.h"

namespace Be
{

    FileOutputStream::FileOutputStream(std::ofstream &ofstream) noexcept
        : m_ofstream{ofstream},
          m_mark{0}
    {
    }

    void FileOutputStream::Write(const void *src, usize_t size)
    {
        m_ofstream.write(static_cast<const char *>(src), size);
        m_offset += size;
    }

    void FileOutputStream::SetPosition(usize_t pos)
    {
        m_offset = pos;
        m_ofstream.seekp(m_offset, std::ios::beg);
    }

    void FileOutputStream::Reset()
    {
        m_offset = m_mark;
        m_ofstream.seekp(m_offset, std::ios::beg);
    }

}