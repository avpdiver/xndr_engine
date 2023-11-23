#pragma once

#include "angelscript.h"

namespace Be::Framework::Scripting
{

    class ScriptEngine;
    using ScriptFunctionHandle = NamedHandle<asIScriptFunction, 64, BE_OPTIMIZE_IDS>;

    class ScriptModule final : public Noncopyable
    {
        friend class ScriptEngine;

    public:
        ScriptModule() noexcept = default;
        ScriptModule(asIScriptEngine *engine, asIScriptModule *module) noexcept;

    public:
        ~ScriptModule() noexcept;

    public:
        void AddScript(const String &code) noexcept;
        void Build() noexcept;

    public:
        ScriptFunctionHandle FindFunction(const String &decl) noexcept;

    public:
        template <typename R, typename... Args>
        R ExecuteFunction(ScriptFunctionHandle handle, Args &&...args) noexcept
        {
            PROFILER_SCOPE;

            const auto it = m_functions.find(handle);
            VERIFY(it != m_functions.end(), "There is no script function");

            auto func = it->second;
            auto ctx = m_engine->CreateContext();
            VERIFY(ctx != nullptr, "Failed to create the script context.");

            ctx->Prepare(func);

            uint32_t arg_index{0};

            // clang-format off
            ([&arg_index, ctx](auto arg)
            {
                using ArgType = decltype(arg);
                if constexpr (IsInteger<ArgType>)
                {
                    if constexpr (sizeof(R) == sizeof(byte_t))
                    {
                        asBYTE val;
                        *((ArgType*)&val) = arg;
                        ctx->SetArgByte(arg_index, val);
                    }
                    else if constexpr (sizeof(R) == sizeof(uint16_t))
                    {
                        asWORD val;
                        *((ArgType*)&val) = arg;
                        ctx->SetArgWord(arg_index, val);
                    }
                    else if constexpr (sizeof(R) == sizeof(uint32_t))
                    {
                        asDWORD val;
                        *((ArgType*)&val) = arg;
                        ctx->SetArgDWord(arg_index, val);
                    }
                    else if constexpr (sizeof(R) == sizeof(uint64_t))
                    {
                        asQWORD val;
                        *((ArgType*)&val) = arg;
                        ctx->SetArgQWord(arg_index, val);
                    } 
                } 
                else if constexpr (IsFloat<ArgType>)
                {
                    if constexpr (sizeof(ArgType) == sizeof(float))
                    {
                        ctx->SetArgFloat(arg_index, arg);
                    }
                    else
                    {
                        ctx->SetArgDouble(arg_index, arg);
                    }
                }
                arg_index++;
            }(args), ...);
            // clang-format on

            auto res = ctx->Execute();
            R ret_value{};

            if (res != asEXECUTION_FINISHED)
            {
                if (res == asEXECUTION_ABORTED)
                {
                    LOG_ERROR("The script was aborted before it could finish. Probably it timed out.");
                }
                else if (res == asEXECUTION_EXCEPTION)
                {
                    auto ex_func = ctx->GetExceptionFunction();
                    LOG_ERROR("The script ended with an exception:\nModule: {}\nSection: {}\nFunc: {}\nLine: {}\nDesc: {}",
                              ex_func->GetModuleName(),
                              ex_func->GetScriptSectionName(),
                              ex_func->GetDeclaration(),
                              ctx->GetExceptionLineNumber(),
                              ctx->GetExceptionString());
                }
                else
                {
                    LOG_ERROR("The script ended for some unforeseen reason ({}).", res);
                }
            }
            else
            {
                if constexpr (IsInteger<R>)
                {
                    if constexpr (sizeof(R) == sizeof(byte_t))
                    {
                        *(asBYTE *)(&ret_value) = ctx->GetReturnByte();
                    }
                    else if constexpr (sizeof(R) == sizeof(uint16_t))
                    {
                        *(asWORD *)(&ret_value) = ctx->GetReturnWord();
                    }
                    else if constexpr (sizeof(R) == sizeof(uint32_t))
                    {
                        *(asDWORD *)(&ret_value) = ctx->GetReturnDWord();
                    }
                    else if constexpr (sizeof(R) == sizeof(uint64_t))
                    {
                        *(asQWORD *)(&ret_value) = ctx->GetReturnQWord();
                    }
                }
                if constexpr (IsFloat<R>)
                {
                    if constexpr (sizeof(R) == sizeof(float))
                    {
                        ret_value = ctx->GetReturnFloat();
                    }
                    else
                    {
                        ret_value = ctx->GetReturnDouble();
                    }
                }
            }

            // Release the context when you're done with it
            ctx->Release();
            return ret_value;
        }

    private:
        asIScriptEngine *m_engine{nullptr};
        asIScriptModule *m_module{nullptr};

    private:
        Map<ScriptFunctionHandle, asIScriptFunction *> m_functions;
    };
}