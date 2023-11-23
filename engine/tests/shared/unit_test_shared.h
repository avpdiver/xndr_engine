#pragma once

#define TEST(...) VERIFY(__VA_ARGS__);
#define TEST_PASSED() LOG_INFO("{} -  passed", BE_FUNCTION_NAME);