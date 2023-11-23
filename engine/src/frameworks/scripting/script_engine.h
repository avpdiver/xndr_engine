#pragma once

namespace Be::Framework::Scripting
{

    namespace details
    {
        static constexpr uint32_t NamedIDs_Start = 3 << 24;
    }

    class ScriptEngine final : public Noncopyable
    {
    public:
        ScriptEngine() noexcept;
        ~ScriptEngine() noexcept;

    public:
        template <typename T>
        void RegisterFunction(const String &decl, T *func_ptr) noexcept
        {
            PROFILER_SCOPE;
            
            auto res = m_engine->RegisterGlobalFunction(decl.c_str(), asFUNCTION(func_ptr), asCALL_CDECL);
            VERIFY(res >= 0, "Failed to RegisterFunction ");
        }

    public:
        ScriptModule &CreateModule(const String &name) noexcept;

    public:
        [[nodiscard]] const ScriptModule &operator[](const String &name) const noexcept;
        [[nodiscard]] ScriptModule &operator[](const String &name) noexcept;

    private:
        asIScriptEngine *m_engine{nullptr};

    private:
        Map<String, UniquePtr<ScriptModule>> m_modules;

    private:
        static ScriptModule s_empty_module;
    };

}