#pragma once

#include "common.hh"
#include "tel.hh"

namespace tel {
namespace builtin {
static Value *tr(const FunctionCallExpression *expr) {
    if (expr->args.size() != 1 || !expr->args[0]->isToken()) {
        return new NullValue();
    }

    auto key = expr->args[0];
    return new StringValue(key->asToken()->token.value);
}

static Value *map(const FunctionCallExpression *expr, Value *source) {
    if (!source || !source->isArray() || expr->args.size() != 1 || !expr->args[0]->isToken()) {
        return new NullValue();
    }

    auto key = expr->args[0]->asToken()->token;
    if (!key.isString()) {
        return new NullValue();
    }

    auto out = new ArrayValue();
    for (auto element : source->asArray()->elements) {
        if (!element || !element->isRecord()) {
            out->elements.push_back(new NullValue());
            continue;
        }

        auto props = element->asRecord()->properties;
        if (props.count(key.value) == 0) {
            out->elements.push_back(new NullValue());
            continue;
        }

        out->elements.push_back(props[key.value]);
    }

    return out;
}

static Value *join(const FunctionCallExpression *expr, Value *source) {
    if (!source || !source->isArray()) {
        printf("not an array: %s\n", source->print().c_str());
    }

    if (!source || !source->isArray() || expr->args.size() != 1 || !expr->args[0]->isToken()) {
        return new NullValue();
    }

    auto separator = expr->args[0]->asToken()->token;
    if (!separator.isString()) {
        return new NullValue();
    }

    auto str = String();

    auto elements = source->asArray()->elements;
    for (auto i = 0; i < elements.size(); i++) {
        str.append(printValueSafe(elements[i]));
        if (i != elements.size() - 1) {
            str.append(separator.value);
        }
    }
    return new StringValue(str);
}

static Value *call(const FunctionCallExpression *expr, Value *source) {
    auto name = expr->name;

    if (name == "tr") {
        return tr(expr);
    } else if (name == "map") {
        return map(expr, source);
    } else if (name == "join") {
        return join(expr, source);
    }

    return new NullValue();
}
} // namespace builtin
} // namespace tel
