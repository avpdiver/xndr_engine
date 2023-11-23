#pragma once

namespace Be
{

    class OutputStream : public Stream
    {
    public:
        virtual ~OutputStream() noexcept = default;

    public:
        virtual void Write(const void *src, usize_t size) = 0;
    };

    template <typename T>
    OutputStream &operator<<(OutputStream &os, const T &src)
    {
        os.Write(&src, sizeof(T));
        return os;
    }

    class FileOutputStream final : public OutputStream
    {
    public:
        FileOutputStream(std::ofstream &ifstream) noexcept;

    public:
        void Write(const void *const src, usize_t size) override;

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
        std::ofstream &m_ofstream;
        usize_t m_size{0};
        usize_t m_offset{0};
        usize_t m_mark{0};
    };

}