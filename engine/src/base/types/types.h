#pragma once

namespace Be
{

    // basic types

    using byte_t = uint8_t;
    using ssize_t = intptr_t;
    using usize_t = size_t;

    using ansi_t = char;
    using utf8_t = char8_t;
    using utf16_t = char16_t;
    using utf32_t = char32_t;

#ifdef BE_PLATFORM_WINDOWS
#if UNICODE
    using char_t = wchar_t;
#define TXT(_text_) (L"" _text_)
#else
    using char_t = ansi_t;
#define TXT(_text_) ("" _text_)
#endif
#else
    using char_t = utf8_t;
#define TXT(_text_) (u8"" _text_)
#endif

    // string types

    template <typename T, typename A = std::allocator<T>>
    using BasicString = std::basic_string<T, std::char_traits<T>, A>;

    using String = BasicString<ansi_t>;
    using WString = BasicString<wchar_t>;
    using U8String = BasicString<utf8_t>;
    using U16String = BasicString<utf16_t>;
    using U32String = BasicString<utf32_t>;

    // string view types

    template <typename T>
    using BasicStringView = std::basic_string_view<T>;
    using StringView = BasicStringView<ansi_t>;
    using WStringView = BasicStringView<wchar_t>;
    using U8StringView = BasicStringView<utf8_t>;
    using U16StringView = BasicStringView<utf16_t>;
    using U32StringView = BasicStringView<utf32_t>;

    // file path type

    using Path = std::filesystem::path;

    // smart pointer types

    template <typename T, typename D = std::default_delete<T>>
    using UniquePtr = std::unique_ptr<T, D>;

    template <typename T>
    using SharedPtr = std::shared_ptr<T>;

    template <typename T>
    using WeakPtr = std::weak_ptr<T>;

    template <typename T>
    using Ref = std::reference_wrapper<T>;

    // container types

    template <typename T, typename A = std::allocator<T>>
    using Array = std::vector<T, A>;

    template <typename T, size_t ArraySize>
    using FixedArray = std::array<T, ArraySize>;

    template <typename T, typename Hash = std::hash<T>, typename Pred = std::equal_to<T>, typename Alloc = std::allocator<T>>
    using Set = std::unordered_set<T, Hash, Pred, Alloc>;

    template <typename Key, typename T, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>, typename Alloc = std::allocator<std::pair<const Key, T>>>
    using Map = std::unordered_map<Key, T, Hash, Pred, Alloc>;

    template <typename T, typename Alloc = std::allocator<T>>
    using List = std::list<T, Alloc>;

    template <typename T, typename Alloc = std::allocator<T>>
    using Queue = std::deque<T, Alloc>;

    template <typename T>
    using Span = std::span<T>;

    using ByteArray = Array<byte_t>;
    using Data = Span<const byte_t>;

    // other types

    template <typename T>
    using Function = std::function<T>;

    template <typename T>
    using Optional = std::optional<T>;
    constexpr auto EmptyOptional = std::nullopt;

    template <class... Types>
    using Variant = std::variant<Types...>;

    template <typename FirstT, typename SecondT>
    using Pair = std::pair<FirstT, SecondT>;

    template <typename... Types>
    using Tuple = std::tuple<Types...>;

    template <typename T>
    using Atomic = std::atomic<T>;
    using AtomicFlag = std::atomic_flag;

    // time types
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    using Seconds = std::chrono::seconds;
    using Milliseconds = std::chrono::milliseconds;
    using Microseconds = std::chrono::microseconds;
    using Nanoseconds = std::chrono::nanoseconds;
    using Secondsf = std::chrono::duration<float>;
    using Secondsd = std::chrono::duration<double>;
    using Nanosecondsd = std::chrono::duration<double, std::nano>;
    using Minutes = std::chrono::minutes;
    using EngineTime = std::chrono::nanoseconds;
}

#include "base/types/clazz.h"
#include "base/types/type_traits.h"
#include "base/types/constants.h"
#include "base/types/hash.h"
#include "base/types/enums.h"
#include "base/types/string.h"
#include "base/types/fixed_string.h"
#include "base/types/cast.h"
#include "base/types/type_id.h"
