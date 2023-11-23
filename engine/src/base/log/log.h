#pragma once

#define LOG_ERROR(...) ::Be::Logger::Log(::Be::Logger::ELevel::eError, __VA_ARGS__)
#define LOG_WARN(...) ::Be::Logger::Log(::Be::Logger::ELevel::eWarn, __VA_ARGS__)
#define LOG_INFO(...) ::Be::Logger::Log(::Be::Logger::ELevel::eInfo, __VA_ARGS__)
#ifdef BE_DEBUG
#define LOG_DEBUG(...) ::Be::Logger::Log(::Be::Logger::ELevel::eDebug, __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

namespace Be
{
    class Logger final
    {
    public:
        enum class ELevel : uint8_t
        {
            eError,
            eWarn,
            eInfo,
            eDebug
        };

    private:
        Logger() = default;

    public:
        template <class... Args>
        static void Log(ELevel level, std::format_string<Args...> fmt, Args &&...args)
        {
            const auto msg = std::format(fmt, std::forward<Args>(args)...);
            const char *lvl = "INFO";
            if (level == ELevel::eDebug)
            {
                lvl = "DEBUG";
            }
            else if (level == ELevel::eWarn)
            {
                lvl = "WARN";
            }
            else if (level == ELevel::eError)
            {
                lvl = "ERROR";
            }
            std::cout << std::format("{}: {}\n", lvl, msg);
        }
    };

}