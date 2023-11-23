#include "frameworks/scripting/scripting.h"

#include "scriptstdstring/scriptstdstring.h"

namespace Be::Framework::Scripting
{
    ScriptModule ScriptEngine::s_empty_module{};

    static void MessageCallback(const asSMessageInfo *msg, void *param)
    {
        auto lvl = Logger::ELevel::eError;

        if (msg->type == asMSGTYPE_WARNING)
        {
            lvl = Logger::ELevel::eWarn;
        }
        else if (msg->type == asMSGTYPE_INFORMATION)
        {
            lvl = Logger::ELevel::eInfo;
        }

        Logger::Log(lvl, "{} ({}, {}) : {}", msg->section, msg->row, msg->col, msg->message);
    }

    ScriptEngine::ScriptEngine() noexcept
    {
        m_engine = asCreateScriptEngine();
        m_engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
        RegisterStdString(m_engine);
    }

    ScriptEngine::~ScriptEngine() noexcept
    {
        if (m_engine != nullptr)
        {
            m_modules.clear();
            m_engine->ShutDownAndRelease();
            m_engine = nullptr;
        }
    }

    ScriptModule &ScriptEngine::CreateModule(const String &name) noexcept
    {
        PROFILER_SCOPE;

        auto as_module = m_engine->GetModule(name.c_str(), asGM_ALWAYS_CREATE);
        auto res = m_modules.emplace(name, MakeUnique<ScriptModule>(m_engine, as_module));
        return *(res.first->second);
    }

    const ScriptModule &ScriptEngine::operator[](const String &name) const noexcept
    {
        PROFILER_SCOPE;

        auto it = m_modules.find(name);
        if (it == m_modules.end())
        {
            FATAL("There is no script module with name '{}'.", name);
            return s_empty_module;
        }
        else
        {
            return *(it->second);
        }
    }

    ScriptModule &ScriptEngine::operator[](const String &name) noexcept
    {
        PROFILER_SCOPE;

        auto it = m_modules.find(name);
        if (it == m_modules.end())
        {
            FATAL("There is no script module with name '{}'.", name);
            return s_empty_module;
        }
        else
        {
            return *(it->second);
        }
    }
}