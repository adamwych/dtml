#pragma once

#include "common.hh"
#include "tel.hh"
#include "utils.hh"

namespace tel {
namespace builtin {

/// [{a: 1, b: 2}, {a: 3, b: 4}, {a: 5, b: 6}].map('a') = [1, 3, 5]
static Value *map(const FunctionCall &call) {
    if (!call.source || !call.source->isArray()) {
        printf("`.map()` can only be called on an array\n");
        return new NullValue();
    }

    if (call.args.size() < 1 || !call.args[0]->isString()) {
        printf("`.map()` argument #0 must be a string\n");
        return new NullValue();
    }

    auto elements = call.source->asArray()->elements;
    auto key = call.args[0]->asString()->value;

    auto result = new ArrayValue();
    result->elements.reserve(elements.size());

    for (auto element : elements) {
        if (!element || !element->isRecord()) {
            result->elements.push_back(new NullValue());
            continue;
        }

        auto props = element->asRecord()->properties;
        if (props.count(key) == 0) {
            result->elements.push_back(new NullValue());
            continue;
        }

        result->elements.push_back(props[key]);
    }

    return result;
}

/// ['a', 'b', 'c'].join(', ') = 'a, b, c'
static Value *join(const FunctionCall &call) {
    if (!call.source || !call.source->isArray()) {
        printf("`.join()` can only be called on an array\n");
        return new NullValue();
    }

    if (call.args.size() < 1 || !call.args[0]->isString()) {
        printf("`.join()` argument #0 must be a string\n");
        return new NullValue();
    }

    auto elements = call.source->asArray()->elements;
    auto separator = call.args[0]->asString()->value;

    auto result = String();
    for (auto i = 0; i < elements.size(); i++) {
        result.append(printValueSafe(elements[i]));
        if (i != elements.size() - 1) {
            result.append(separator);
        }
    }
    return new StringValue(result);
}

static Value *stringify(const FunctionCall &call) {
    if (!call.source) {
        printf("`.stringify()` can only called be on a value\n");
        return new NullValue();
    }

    return new StringValue(tel::toJson(call.source));
}

static Value *round(const FunctionCall &call) {
    if (!call.source || !call.source->isNumber()) {
        printf("`.round()` can only be called on a number\n");
        return new NullValue();
    }

    return new StringValue(
        std::to_string(static_cast<int>(::round(call.source->asNumber()->value))));
}

static Value *is(const FunctionCall &call) {
    if (call.args.size() != 1) {
        return new NullValue();
    }

    return new BooleanValue(call.source->equals(call.args[0]));
}

static Value *isNot(const FunctionCall &call) {
    if (call.args.size() != 1) {
        return new NullValue();
    }

    return new BooleanValue(!call.source->equals(call.args[0]));
}

static Value *orr(const FunctionCall &call) {
    if (!call.source->isBoolean()) {
        printf("`.or()` can only be called on a boolean\n");
        return new NullValue();
    }

    if (call.args.size() != 1 || !call.args[0]->isBoolean()) {
        printf("`.or()` argument #0 must be a boolean\n");
        return new NullValue();
    }

    return new BooleanValue(call.source->asBoolean()->value || call.args[0]->asBoolean()->value);
}

static Value *call(const FunctionCall &call) {
#define TRY(func)                                                                                  \
    if (call.name == #func)                                                                        \
    return func(call)
#define TRY_AS(func, alias)                                                                        \
    if (call.name == alias)                                                                        \
    return func(call)

    TRY(map);
    TRY(join);
    TRY(stringify);
    TRY(round);
    TRY(is);
    TRY_AS(isNot, "not");
    TRY_AS(orr, "or");

#undef TRY

    printf("%s\n", format("Call to undefined function '%s'", call.name.c_str()).c_str());
    return new NullValue();
}

} // namespace builtin
} // namespace tel
