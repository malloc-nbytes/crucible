#include "keyword.hpp"

int
is_keyword(const std::string &s)
{
        static const char *kwds[] = KEYWORD_CPL;

        for (size_t i = 0; i < sizeof(kwds)/sizeof(*kwds); ++i) {
                if (s == kwds[i])
                        return 1;
        }

        return 0;
}
