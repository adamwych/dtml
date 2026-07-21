#include "tel-expr-parser.hh"
#include "tel-expr-cursor.hh"

namespace tel {
static ExpressionToken tokenError(const String &message) {
    return ExpressionToken{.type = ExpressionTokenType::Error, .value = message};
}

static ExpressionParseResult exprError(const String &message) {
    return ExpressionParseResult{.error = message, .expression = nullptr};
}

static ExpressionParseResult exprOk(Expression *expr) {
    return ExpressionParseResult{.expression = expr};
}

ExpressionParseResult ExpressionParser::parseFunctionCall(Expression *source, const String &name) {
    Vector<Expression *> args;

    while (true) {
        auto arg = parse2();
        if (!arg.expression) {
            return arg;
        }

        args.push_back(arg.expression);

        if (_c.eat(',')) {
            continue;
        }

        if (_c.eat(')')) {
            break;
        }

        return exprError("unexpected character in function call arguments list");
    }

    auto expr = new FunctionCallExpression();
    expr->source = source;
    expr->name = name;
    expr->args = args;
    return exprOk(expr);
}

ExpressionToken ExpressionParser::parseToken() {
    // Identifier
    if (_c.isAsciiLetter() || _c.is('_') || _c.is('$')) {
        auto start = _c.pos();
        while (_c.isAsciiAlphanumeric() || _c.is('_') || _c.is('$')) {
            if (!_c.advance()) {
                break;
            }
        }
        auto end = _c.pos();

        return ExpressionToken{
            .type = ExpressionTokenType::Identifier,
            .value = String(_text.substr(start, end - start)),
        };
    }

    // String
    if (_c.eat('\'')) {
        auto start = _c.pos();
        while (!_c.eat('\'')) {
            if (!_c.advance()) {
                return tokenError("unexpected end-of-file when parsing a string");
            }
        }
        auto end = _c.pos() - 1;

        return ExpressionToken{
            .type = ExpressionTokenType::String,
            .value = String(_text.substr(start, end - start)),
        };
    }

    // Interpolated string
    if (_c.eat('`')) {
        auto start = _c.pos();
        while (!_c.eat('`')) {
            if (!_c.advance()) {
                return tokenError("unexpected end-of-file when parsing an interpolated string");
            }
        }
        auto end = _c.pos() - 1;

        return ExpressionToken{
            .type = ExpressionTokenType::InterpolatedString,
            .value = String(_text.substr(start, end - start)),
        };
    }

    // Number
    if (_c.isAsciiDigit()) {
        auto start = _c.pos();
        while (_c.isAsciiDigit()) {
            if (!_c.advance()) {
                break;
            }
        }
        auto end = _c.pos();

        return ExpressionToken{
            .type = ExpressionTokenType::Number,
            .value = String(_text.substr(start, end - start)),
        };
    }

    return ExpressionToken{.type = ExpressionTokenType::Null};
}

ExpressionParseResult ExpressionParser::parse1() {
    auto left = parse2();
    if (!left.expression) {
        return left;
    }

    return left;
}

ExpressionParseResult ExpressionParser::parse2() {
    auto left = parse9();
    if (!left.expression) {
        return left;
    }

    while (true) {
        if (left.expression->isToken() && left.expression->asToken()->token.isIdentifier() &&
            _c.eat('(')) {
            left = parseFunctionCall(nullptr, left.expression->asToken()->token.value);
            continue;
        }

        if (_c.eat('.')) {
            auto right = parseToken();
            if (!right.isIdentifier()) {
                return exprError("expected identifier after member access operator");
            }

            if (_c.eat('(')) {
                left = parseFunctionCall(left.expression, right.value);
                continue;
            }

            auto expr = new MemberAccessExpression();
            expr->source = left.expression;
            expr->member = right.value;
            left = exprOk(expr);
            continue;
        }

        break;
    }

    return left;
}

ExpressionParseResult ExpressionParser::parse9() {
    auto token = parseToken();
    auto expr = new TokenExpression();
    expr->token = token;
    return exprOk(expr);
}
} // namespace tel
