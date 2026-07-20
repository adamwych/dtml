#pragma once

#include "common.hh"
#include "tel-expr.hh"

namespace tel {
struct ExpressionParseResult {
    String error;
    Expression *expression;
};

class ExpressionParser {
    const StringView &_text;
    ExpressionCursor _c;
    ExpressionToken _tokens[128];
    int _tokenIndex = 0;

  public:
    explicit ExpressionParser(const StringView &text) : _text(text), _c{_text} {
    }

    ExpressionParseResult parse() {
        return parse1();
    }

  private:
    ExpressionParseResult parseFunctionCall(Expression *source, const String &name);
    ExpressionToken parseToken();
    ExpressionParseResult parse1();
    ExpressionParseResult parse2();
    ExpressionParseResult parse9();
};
} // namespace tel
