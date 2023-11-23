#include "base/base.h"

namespace Be::StringUtils
{

    Array<String> Split(const String &str, char sep) noexcept
    {
        Array<String> tokens;
        String token;
        std::istringstream tokenStream(str);

        while (std::getline(tokenStream, token, sep))
        {
            tokens.emplace_back(token);
        }
        return tokens;
    }

}
