#include "frameworks/scripting/scripting.h"

#include "../shared/unit_test_shared.h"

using namespace Be;
using namespace Be::Framework::Scripting;

static constexpr auto script =
    "float calc(float a, float b)"
    "{"
    "    Print(\"Hello from Script\");"
    "    return (a + b);"
    "}";

static void PrintString(String &str)
{
    LOG_INFO("Script says: {}", str);
}

void UnitTest_Script()
{
    ScriptEngine engine{};

    engine.RegisterFunction("void Print(string &in)", PrintString);

    auto& module = engine.CreateModule("general");
    module.AddScript(script);
    module.Build();
    auto func = module.FindFunction("float calc(float a, float b)");
    auto res = module.ExecuteFunction<float>(func, 17.0f, 25.0f);

    TEST(res == 42.0f, "Script test failed");
    TEST_PASSED();
}

int main()
{
    UnitTest_Script();
    return 0;
}