#include "base/base.h"

namespace Be
{

    FileInputStream::FileInputStream(std::ifstream &ifstream) noexcept
        : m_ifstream{ifstream},
          m_mark{0}
    {
        m_ifstream.seekg(0, std::ios::end);
        m_size = m_ifstream.tellg();
        m_ifstream.seekg(0, std::ios::beg);
    }

    void FileInputStream::Read(void *dst, usize_t size)
    {
        ASSERT(m_offset + size <= m_size);

        m_ifstream.read(static_cast<char *>(dst), size);
        m_offset += size;
    }

    void FileInputStream::Skip(usize_t size)
    {
        ASSERT(m_offset + size <= m_size);
        m_offset += size;
        m_ifstream.seekg(m_offset, std::ios::beg);
    }

    void FileInputStream::SetPosition(usize_t pos)
    {
        m_offset = pos;
        m_ifstream.seekg(m_offset, std::ios::beg);
    }

    void FileInputStream::Reset()
    {
        m_offset = m_mark;
        m_ifstream.seekg(m_offset, std::ios::beg);
    }
}