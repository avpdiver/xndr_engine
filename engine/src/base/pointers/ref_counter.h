#pragma once

namespace Be
{

    class RefCounter
    {
    public:
        virtual ~RefCounter() noexcept = default;

    public:
        virtual uint64_t AddRef() noexcept
        {
            return ++m_ref_count;
        }

        virtual uint64_t Release() noexcept
        {
            uint64_t result = --m_ref_count;
            if (result == 0)
            {
                delete this;
            }
            return result;
        }

    private:
        Atomic<uint64_t> m_ref_count{1};
    };

    template <typename T>
        requires(IsBaseOf<RefCounter, T>)
    class RefCountPtr
    {
    public:
        RefCountPtr() noexcept : m_ptr(nullptr)
        {
        }

        RefCountPtr(std::nullptr_t) noexcept : m_ptr(nullptr)
        {
        }

        template <class U>
        RefCountPtr(U *other) noexcept : m_ptr(other)
        {
            InternalAddRef();
        }

        RefCountPtr(const RefCountPtr &other) noexcept : m_ptr(other.m_ptr)
        {
            InternalAddRef();
        }

        template <class U>
            requires IsConvertible<U *, T *>
        RefCountPtr(const RefCountPtr<U> &other) noexcept : m_ptr(other.m_ptr)
        {
            InternalAddRef();
        }

        RefCountPtr(RefCountPtr &&other) noexcept : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<RefCountPtr *>(&reinterpret_cast<unsigned char &>(other)))
            {
                Swap(other);
            }
        }

        template <class U>
            requires IsConvertible<U *, T *>
        RefCountPtr(RefCountPtr<U> &&other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ~RefCountPtr() noexcept
        {
            InternalRelease();
        }

    public:
        RefCountPtr &operator=(std::nullptr_t) noexcept
        {
            InternalRelease();
            return *this;
        }

        RefCountPtr &operator=(T *other) noexcept
        {
            if (m_ptr != other)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        RefCountPtr &operator=(U *other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr &operator=(const RefCountPtr &other) noexcept
        {
            if (m_ptr != other.m_ptr)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <class U>
        RefCountPtr &operator=(const RefCountPtr<U> &other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr &operator=(RefCountPtr &&other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr &&>(other)).Swap(*this);
            return *this;
        }

        template <class U>
        RefCountPtr &operator=(RefCountPtr<U> &&other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr<U> &&>(other)).Swap(*this);
            return *this;
        }

    public:
        void Swap(RefCountPtr &&r) noexcept
        {
            T *tmp = m_ptr;
            m_ptr = r.m_ptr;
            r.m_ptr = tmp;
        }

        void Swap(RefCountPtr &r) noexcept
        {
            T *tmp = m_ptr;
            m_ptr = r.m_ptr;
            r.m_ptr = tmp;
        }

    public:
        [[nodiscard]] forceinline T *Get() const noexcept
        {
            return m_ptr;
        }

        template <typename U>
            requires IsBaseOf<T, U>
        [[nodiscard]] forceinline U *As() const noexcept
        {
            return (U *)m_ptr;
        }

        operator T *() const noexcept
        {
            return m_ptr;
        }

        const T &operator*() const noexcept
        {
            return *m_ptr;
        }

        T &operator*() noexcept
        {
            return *m_ptr;
        }

        T *operator->() const noexcept
        {
            return m_ptr;
        }

        T **operator&() noexcept
        {
            return &m_ptr;
        }

    public:
        operator bool() const noexcept
        {
            return (m_ptr != nullptr);
        }

        [[nodiscard]] forceinline bool operator==(const RefCountPtr &other) noexcept
        {
            return (m_ptr == other.m_ptr);
        }

        [[nodiscard]] forceinline bool operator!=(const RefCountPtr &other) noexcept
        {
            return (m_ptr != other.m_ptr);
        }

    public:
        [[nodiscard]] forceinline T *const *GetAddressOf() const noexcept
        {
            return &m_ptr;
        }

        [[nodiscard]] forceinline T **GetAddressOf() noexcept
        {
            return &m_ptr;
        }

        [[nodiscard]] forceinline T **ReleaseAndGetAddressOf() noexcept
        {
            InternalRelease();
            return &m_ptr;
        }

    public:
        T *Release() noexcept
        {
            T *ptr{m_ptr};
            m_ptr = nullptr;
            return ptr;
        }

        // Set the pointer while keeping the object's reference count unchanged
        void Reset(T *other) noexcept
        {
            if (m_ptr != nullptr)
            {
                auto ref = m_ptr->Release();
                Unused(ref);

                // Attaching to the same object only works if duplicate references are being coalesced. Otherwise
                // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
                assert(ref != 0 || m_ptr != other);
            }

            m_ptr = other;
        }

    public:
        static RefCountPtr<T> Create(T *other) noexcept
        {
            RefCountPtr<T> Ptr;
            Ptr.Reset(other);
            return Ptr;
        }

        uint64_t Reset() noexcept
        {
            return InternalRelease();
        }

    protected:
        void InternalAddRef() const noexcept
        {
            if (m_ptr != nullptr)
            {
                m_ptr->AddRef();
            }
        }

        uint64_t InternalRelease() noexcept
        {
            uint64_t ref{0};
            T *temp{m_ptr};

            if (temp != nullptr)
            {
                m_ptr = nullptr;
                ref = temp->Release();
            }

            return ref;
        }

    protected:
        T *m_ptr{nullptr};

    protected:
        template <class U>
            requires(IsBaseOf<RefCounter, U>)
        friend class RefCountPtr;
    };

    template <typename T, typename... Args>
        requires(IsBaseOf<RefCounter, T> && IsConstructible<T, Args...>)
    [[nodiscard]] forceinline RefCountPtr<T> MakeRefCounter(Args &&...args)
    {
        return RefCountPtr<T>::Create(new T(std::forward<Args>(args)...));
    }

}

namespace std
{

    template <typename T>
    struct hash<Be::RefCountPtr<T>>
    {
        std::size_t operator()(const Be::RefCountPtr<T> &v) const
        {
            return static_cast<size_t>(v.Get());
        }
    };

}
