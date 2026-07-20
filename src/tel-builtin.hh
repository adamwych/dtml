#pragma once

#include "common.hh"
#include "tel.hh"

namespace tel {
namespace builtin {
static Value *map(const EvaluationContext *ctx, const FunctionCallExpression *expr, Value *source) {
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

static Value *join(const EvaluationContext *ctx, const FunctionCallExpression *expr,
                   Value *source) {
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

static Value *call(const EvaluationContext *ctx, const FunctionCallExpression *expr,
                   Value *source) {
    auto name = expr->name;

    if (name == "map") {
        return map(ctx, expr, source);
    } else if (name == "join") {
        return join(ctx, expr, source);
    }

    return new NullValue();
}
} // namespace builtin
} // namespace tel
