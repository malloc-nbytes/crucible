#ifndef SCOPE_H_INCLUDED
#define SCOPE_H_INCLUDED

#include <string>
#include <vector>

template <typename T>
struct scope {
        struct entry {
                std::string id;
                T value;
        };

        using frame = std::vector<entry>;

        std::vector<frame> frames;
};

template <typename T>
scope<T> scope_create(void);

template <typename T>
void scope_push(scope<T> *s);

template <typename T>
void scope_pop(scope<T> *s);

template <typename T>
void scope_insert(scope<T> *s, std::string id, T value);

template <typename T>
T *scope_get(scope<T> *s, const std::string &k);

template <typename T>
int scope_contains(scope<T> *s, const std::string &k);

#endif // SCOPE_H_INCLUDED

#ifndef SCOPE_IMPLEMENTATION
#include "../scope.cpp"
#endif
