#include "location.hpp"

#include <string>

location
location_from(size_t      r,
              size_t      c,
              std::string path)
{
        return location {
                .r    = r,
                .c    = c,
                .path = path,
        };
}

std::string
location_to_string(location l)
{
        return l.path + ":" + std::to_string(l.r) + ":" + std::to_string(l.r);
}
