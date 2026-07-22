#include "tel.hh"
#include "tel-builtin.hh"
#include "tel-expr-parser.hh"

namespace tel {
Value *Interpreter::evaluate(const StringView &text) {
    auto parseResult = ExpressionParser{text}.parse();
    if (!parseResult.error.empty()) {
        printf("Failed to parse expression '%s': %s\n", String(text).c_str(),
               parseResult.error.c_str());
        return new NullValue();
    }

    return evaluate(parseResult.expression);
}

Vector<Value *> Interpreter::evaluate(const Vector<Expression *> exprs) {
    auto evaluated = Vector<Value *>{};
    evaluated.resize(exprs.size());
    for (auto i = 0; i < exprs.size(); i++) {
        evaluated[i] = evaluate(exprs[i]);
    }
    return evaluated;
}

Value *Interpreter::evaluateToken(const TokenExpression *expr) {
    auto token = expr->asToken()->token;
    switch (token.type) {
    case ExpressionTokenType::Identifier:
        if (_ctx.globals.count(token.value) != 0) {
            return _ctx.globals[token.value];
        }
        return new NullValue();
    case ExpressionTokenType::String:
        return new StringValue(token.value);
    case ExpressionTokenType::InterpolatedString:
        return evaluateInterpolatedString(token.value);
    case ExpressionTokenType::Number:
        return new NumberValue(std::stod(token.value));
    default:
        return new NullValue();
    }
}

Value *Interpreter::evaluateMemberAccess(const MemberAccessExpression *expr) {
    auto source = evaluate(expr->source);
    if (source->isRecord()) {
        auto sourceProps = source->asRecord()->value;
        if (sourceProps.count(expr->member) == 0) {
            return new NullValue();
        }

        return sourceProps[expr->member];
    }

    return new NullValue();
}

Value *Interpreter::evaluateFunctionCall(const FunctionCallExpression *expr) {
    return builtin::call(FunctionCall{
        .ctx = &_ctx,
        .name = expr->name,
        .args = evaluate(expr->args),
        .source = expr->source ? evaluate(expr->source) : nullptr,
    });
}

Value *Interpreter::evaluate(const Expression *expr) {
    switch (expr->getType()) {
    case ExpressionType::Token:
        return evaluateToken(expr->asToken());
    case ExpressionType::MemberAccess:
        return evaluateMemberAccess(expr->asMemberAccess());
    case ExpressionType::FunctionCall:
        return evaluateFunctionCall(expr->asFunctionCall());
    default:
        return new NullValue();
    }
}

static size_t findInterpolationEnd(const String &text, size_t exprStart) {
    auto depth = 1;

    for (auto idx = exprStart; idx < text.length(); idx++) {
        if (text[idx] == '{') {
            depth++;
        } else if (text[idx] == '}') {
            depth--;
            if (depth == 0) {
                return idx;
            }
        }
    }

    return String::npos;
}

Value *Interpreter::evaluateInterpolatedString(const String &text) {
    String out;
    size_t cursor = 0;

    while (cursor < text.length()) {
        auto interpolationStart = text.find("${", cursor);
        if (interpolationStart == String::npos) {
            out.append(text.substr(cursor));
            break;
        }

        out.append(text.substr(cursor, interpolationStart - cursor));

        auto expressionStart = interpolationStart + 2;
        auto interpolationEnd = findInterpolationEnd(text, expressionStart);
        if (interpolationEnd == String::npos) {
            printf("Failed to parse interpolated string '%s': missing closing '}'\n", text.c_str());
            return new NullValue();
        }

        auto expression = text.substr(expressionStart, interpolationEnd - expressionStart);
        auto value = evaluate(expression);
        out.append(printValueSafe(value));

        cursor = interpolationEnd + 1;
    }

    return new StringValue(out);
}
} // namespace tel
