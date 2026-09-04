#include "type.hpp"

int
is_type(std::string &s)
{
        static const char *types[] = TYPE_CPL;

        for (size_t i = 0; i < sizeof(types)/sizeof(*types); ++i) {
                if (s == types[i])
                        return 1;
        }

        return 0;
}
