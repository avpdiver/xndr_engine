#pragma once

namespace Be
{

    template <typename T, typename A1, typename A2>
    forceinline BasicString<T, A1> &&operator<<(BasicString<T, A1> &&lhs, const BasicString<T, A2> &rhs)
    {
        return std::move(std::move(lhs).append(rhs.data(), rhs.size()));
    }

    template <typename T, typename A1, typename A2>
    forceinline BasicString<T, A1> &operator<<(BasicString<T, A1> &lhs, const BasicString<T, A2> &rhs)
    {
        return lhs.append(rhs.data(), rhs.size());
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &&operator<<(BasicString<T, A> &&lhs, const BasicStringView<T> &rhs)
    {
        return std::move(std::move(lhs).append(rhs.data(), rhs.size()));
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &operator<<(BasicString<T, A> &lhs, const BasicStringView<T> &rhs)
    {
        return lhs.append(rhs.data(), rhs.size());
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &&operator<<(BasicString<T, A> &&lhs, T const *const rhs)
    {
        return std::move(std::move(lhs).append(rhs));
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &operator<<(BasicString<T, A> &lhs, T const *const rhs)
    {
        return lhs.append(rhs);
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &&operator<<(BasicString<T, A> &&lhs, const T rhs)
    {
        return std::move(std::move(lhs) += rhs);
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &operator<<(BasicString<T, A> &lhs, const T rhs)
    {
        return (lhs += rhs);
    }

    template <typename T, typename A1, typename A2>
    forceinline BasicString<T, A1> &&operator>>(const BasicString<T, A2> &lhs, BasicString<T, A1> &&rhs)
    {
        rhs.insert(rhs.begin(), lhs.begin(), lhs.end());
        return std::move(rhs);
    }

    template <typename T, typename A1, typename A2>
    forceinline BasicString<T, A1> &operator>>(const BasicString<T, A2> &lhs, BasicString<T, A1> &rhs)
    {
        rhs.insert(rhs.begin(), lhs.begin(), lhs.end());
        return rhs;
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &&operator>>(const BasicStringView<T> &lhs, BasicString<T, A> &&rhs)
    {
        rhs.insert(rhs.begin(), lhs.begin(), lhs.end());
        return std::move(rhs);
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &operator>>(const BasicStringView<T> &lhs, BasicString<T, A> &rhs)
    {
        rhs.insert(rhs.begin(), lhs.begin(), lhs.end());
        return rhs;
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &&operator>>(T const *const lhs, BasicString<T, A> &&rhs)
    {
        rhs.insert(rhs.begin(), lhs);
        return std::move(rhs);
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &operator>>(T const *const lhs, BasicString<T, A> &rhs)
    {
        rhs.insert(0u, lhs);
        return rhs;
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &&operator>>(const T lhs, BasicString<T, A> &&rhs)
    {
        rhs.insert(0u, lhs);
        return std::move(rhs);
    }

    template <typename T, typename A>
    forceinline BasicString<T, A> &operator>>(const T lhs, BasicString<T, A> &rhs)
    {
        rhs.insert(0u, lhs);
        return rhs;
    }

    [[nodiscard]] forceinline String ToString(String value)
    {
        return value;
    }

    [[nodiscard]] forceinline String ToString(const char value[])
    {
        return String{value};
    }

    template <typename T>
    [[nodiscard]] EnableIf<!IsEnum<T>, String> ToString(const T &value)
    {
        return std::to_string(value);
    }

    namespace StringUtils
    {
        template <typename StringType>
        [[nodiscard]] forceinline bool IsNullTerminated(const StringType &str) noexcept
        {
            return str.empty() || str.data()[str.size()] == 0;
        }

        template <typename T>
        [[nodiscard]] static T Parse(const String &str) noexcept
        {
            if constexpr (IsSameType<String, T>)
            {
                return str;
            }
            else if constexpr (IsEnum<T>)
            {
                using safe_type = std::underlying_type_t<T>;
                return static_cast<T>(Parse<safe_type>(str));
            }
            else if constexpr (IsSameType<bool, T>)
            {
                return str == "true" || Parse<Optional<int32_t>>(str) == 1;
            }
            else if constexpr (IsOptional<T>)
            {
                using base_type = T::value_type;
                base_type temp;
                std::istringstream iss(str);

                if ((iss >> temp).fail())
                {
                    return EmptyOptional;
                }
                return temp;
            }
            else if constexpr (IsInteger<T>)
            {
                return static_cast<T>(std::stoul(str));
            }
            else if constexpr (IsFloat<T>)
            {
                return static_cast<T>(std::stod(str));
            }
            else
            {
                long double temp;
                std::istringstream iss(str);
                iss >> temp;
                return static_cast<T>(temp);
            }
        }

        [[nodiscard]] forceinline String Trim(const String &str) noexcept
        {
            String s{str};
            s.erase(0, s.find_first_not_of(" \n\r\t"));
            s.erase(s.find_last_not_of(" \n\r\t") + 1);
            return s;
        }

        [[nodiscard]] Array<String> Split(const String &str, char sep) noexcept;

        forceinline bool ReplaceFirst(String &str, const String &token, const String &to) noexcept
        {
            const auto startPos = str.find(token);
            if (startPos == String::npos)
            {
                return false;
            }

            str.replace(startPos, token.length(), to);
            return true;
        }

        forceinline bool Replace(String &str, const String &token, const String &to) noexcept
        {
            auto res{false};
            while (ReplaceFirst(str, token, to))
            {
                res = true;
            }
            return res;
        }

        [[nodiscard]] forceinline String ToUpper(const String &str) noexcept
        {
            String res(str);
            for (auto &c : res)
            {
                c = std::toupper(c, std::locale());
            }
            return res;
        }

        [[nodiscard]] forceinline String ToLower(const String &str) noexcept
        {
            String res(str);
            for (auto &c : res)
            {
                c = std::tolower(c, std::locale());
            }
            return res;
        }

        [[nodiscard]] forceinline std::wstring ToWstring(const String &str) noexcept
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            return conv.from_bytes(str);
        }

        [[nodiscard]] forceinline String ToUtf8(const std::wstring &str) noexcept
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            return conv.to_bytes(str);
        }
    }
}