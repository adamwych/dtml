#pragma once

#include "common.hh"
#include "tel.hh"
#include "utils.hh"

namespace tel {
namespace builtin {

#define REQUIRE_THIS_ANY()                                                                         \
    if (!call.source) {                                                                            \
        printf("%s\n",                                                                             \
               format("`.%s()` can only be called on a value", __builtin_func_name).c_str());      \
        return new NullValue();                                                                    \
    }                                                                                              \
    auto this##Value = call.source;

#define REQUIRE_THIS(typename)                                                                     \
    if (!call.source || !call.source->is##typename()) {                                            \
        printf(                                                                                    \
            "%s\n",                                                                                \
            format("`.%s()` can only be called on %s", __builtin_func_name, #typename).c_str());   \
        return new NullValue();                                                                    \
    }                                                                                              \
    auto this##typename##Value = call.source->as##typename();                                      \
    auto this##typename = call.source->as##typename()->value;

// ============

#define REQUIRE_ARG_ANY(index)                                                                     \
    if (call.args.size() < (index + 1)) {                                                          \
        printf(                                                                                    \
            "%s\n",                                                                                \
            format("`.%s()` argument #%d must be provided", __builtin_func_name, index).c_str());  \
        return new NullValue();                                                                    \
    }                                                                                              \
    auto arg##index##Value = call.args[index];

#define REQUIRE_ARG(index, typename)                                                               \
    if (call.args.size() < (index + 1) || !call.args[index]->is##typename()) {                     \
        printf("%s\n",                                                                             \
               format("`.%s()` argument #%d must be a %s", __builtin_func_name, index, #typename)  \
                   .c_str());                                                                      \
        return new NullValue();                                                                    \
    }                                                                                              \
    auto arg##index##Value = call.args[index]->as##typename();                                     \
    auto arg##index = call.args[index]->as##typename()->value;

// ============

#define BUILTIN(thisTypeName, name, body)                                                          \
    static Value *name##_(const FunctionCall &call) {                                              \
        const char *__builtin_func_name = #name;                                                   \
        REQUIRE_THIS(thisTypeName)                                                                 \
        body                                                                                       \
    }

#define BUILTIN_STANDALONE(name, body)                                                             \
    static Value *name##_(const FunctionCall &call) {                                              \
        const char *__builtin_func_name = #name;                                                   \
        REQUIRE_THIS_ANY()                                                                         \
        body                                                                                       \
    }

} // namespace builtin
} // namespace tel
