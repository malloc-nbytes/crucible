#define SCOPE_IMPLEMENTATION
#include "scope.hpp"
#undef SCOPE_IMPLEMENTATION

#include <cassert>
#include <utility>

template <typename T> scope<T>
scope_create(void)
{
        return scope<T> {};
}

template <typename T> void
scope_push(scope<T> *s)
{
        s->frames.emplace_back();
}

template <typename T> void
scope_pop(scope<T> *s)
{
        assert(s->frames.size() > 0);
        s->frames.pop_back();
}

template <typename T> void
scope_insert(scope<T>           *s,
             std::string         id,
             T                   value)
{
        assert(!s->frames.empty());
        s->frames.back().push_back(typename scope<T>::entry {
                .id = std::move(id),
                .value = std::move(value),
        });
}

template <typename T> T *
scope_get(scope<T> *s, const std::string &k)
{
        for (auto f = s->frames.rbegin(); f != s->frames.rend(); ++f) {
                for (auto e = f->rbegin(); e != f->rend(); ++e) {
                        if (e->id == k)
                                return &e->value;
                }
        }

        return nullptr;
}

template <typename T> int
scope_contains(scope<T> *s, const std::string &k)
{
        return scope_get(s, k) != nullptr;
}
