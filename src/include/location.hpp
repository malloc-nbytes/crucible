#ifndef LOCATION_H_INCLUDED
#define LOCATION_H_INCLUDED

#include <stddef.h>
#include <string>

typedef struct {
        size_t      r;
        size_t      c;
        std::string path;
} location;

location location_from(size_t      r,
                       size_t      c,
                       std::string path);

std::string location_to_string(location l);

#endif // LOCATION_H_INCLUDED
