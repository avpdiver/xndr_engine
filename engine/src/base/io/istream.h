#pragma once

namespace Be
{
    class InputStream : public Stream
    {
    public:
        virtual ~InputStream() noexcept = default;

    public:
        [[nodiscard]] virtual usize_t GetSize() const = 0;
        [[nodiscard]] virtual usize_t GetAvailable() const = 0;
        virtual void Read(void *dst, usize_t size) = 0;
        virtual void Skip(usize_t size) = 0;
    };

    template <typename T>
    InputStream &operator>>(InputStream &is, T &dst)
    {
        is.Read(&dst, sizeof(T));
        return is;
    }

    class ByteArrayInputStream final : public InputStream
    {
    public:
        explicit ByteArrayInputStream(const Data &data) noexcept
            : m_data{data.data()},
              m_size{data.size_bytes()},
              m_offset{0},
              m_mark{0}
        {
        }

    public:
        [[nodiscard]] forceinline usize_t GetSize() const override
        {
            return m_size;
        }

        [[nodiscard]] forceinline usize_t GetAvailable() const override
        {
            return (m_size - m_offset);
        }

        forceinline void Read(void *dst, usize_t size) override
        {
            ASSERT(m_offset + size <= m_size);

            MemCopy(dst, m_data + m_offset, size);
            m_offset += size;
        }

        forceinline void Skip(usize_t size) override
        {
            ASSERT(m_offset + size <= m_size);
            m_offset += size;
        }

    public:
        [[nodiscard]] forceinline usize_t GetPosition() const override
        {
            return m_offset;
        }

        forceinline void SetPosition(usize_t pos) override
        {
            ASSERT(pos < m_size)
            m_offset = pos;
        }

    public:
        [[nodiscard]] forceinline bool IsMarkSupported() const override
        {
            return true;
        }

        forceinline void Mark() override
        {
            m_mark = m_offset;
        }

        forceinline void Reset() override
        {
            m_offset = m_mark;
        }

    private:
        const byte_t *const m_data{nullptr};
        const usize_t m_size;
        usize_t m_offset{0};
        usize_t m_mark{0};
    };

    class FileInputStream final : public InputStream
    {
    public:
        FileInputStream(std::ifstream &ifstream) noexcept;

    public:
        [[nodiscard]] forceinline usize_t GetSize() const override
        {
            return m_size;
        }

        [[nodiscard]] forceinline usize_t GetAvailable() const override
        {
            return (m_size - m_offset);
        }

        void Read(void *dst, usize_t size) override;
        void Skip(usize_t size) override;

    public:
        [[nodiscard]] forceinline usize_t GetPosition() const override
        {
            return m_offset;
        }

        void SetPosition(usize_t pos) override;

    public:
        [[nodiscard]] forceinline bool IsMarkSupported() const override
        {
            return true;
        }

        forceinline void Mark() override
        {
            m_mark = m_offset;
        }

        void Reset() override;

    private:
        std::ifstream &m_ifstream;
        usize_t m_size{0};
        usize_t m_offset{0};
        usize_t m_mark{0};
    };

}