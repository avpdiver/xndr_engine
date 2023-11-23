#include "frameworks/scripting/scripting.h"

namespace Be::Framework::Scripting
{

    ScriptModule::ScriptModule(asIScriptEngine *engine, asIScriptModule *module) noexcept
        : m_engine{engine},
          m_module{module}
    {
    }

    ScriptModule::~ScriptModule() noexcept
    {
        for (auto &[h, f] : m_functions)
        {
            m_module->RemoveFunction(f);
        }
        m_functions.clear();
    }

    void ScriptModule::AddScript(const String &code) noexcept
    {
        PROFILER_SCOPE;

        auto res = m_module->AddScriptSection("script", code.c_str(), code.length());
        VERIFY(res >= 0, "AddScript failed");
    }

    void ScriptModule::Build() noexcept
    {
        PROFILER_SCOPE;

        auto res = m_module->Build();
        VERIFY(res >= 0, "ScriptModule Build failed");
    }

    ScriptFunctionHandle ScriptModule::FindFunction(const String &decl) noexcept
    {
        PROFILER_SCOPE;
        
        ScriptFunctionHandle handle{decl};

        const auto it = m_functions.find(handle);
        if (it != m_functions.end())
        {
            return handle;
        }

        auto func = m_module->GetFunctionByDecl(decl.c_str());
        VERIFY(func != nullptr, "The script function '{}' was not found.", decl);

        m_functions[handle] = func;

        return handle;
    }

}