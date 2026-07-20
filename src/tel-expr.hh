#pragma once

#include "common.hh"
#include "tel-expr-cursor.hh"
#include "tel-expr-token.hh"

namespace tel {
enum class ExpressionType {
    Token = 1,
    MemberAccess = 2,
    FunctionCall = 3,
};

class TokenExpression;
class MemberAccessExpression;
class FunctionCallExpression;

class Expression {
  public:
    virtual ~Expression() = default;
    virtual ExpressionType getType() const = 0;

    /* clang-format off */
    inline bool isToken() const         { return getType() == ExpressionType::Token; }
    inline bool isMemberAccess() const  { return getType() == ExpressionType::MemberAccess; }
    inline bool isFunctionCall() const  { return getType() == ExpressionType::FunctionCall; }

	TokenExpression *asToken() const               { return (TokenExpression *)this; }
	MemberAccessExpression *asMemberAccess() const { return (MemberAccessExpression *)this; }
    FunctionCallExpression *asFunctionCall() const { return (FunctionCallExpression *)this; }
    /* clang-format on */
};

class TokenExpression : public Expression {
  public:
    ExpressionToken token;

    virtual ExpressionType getType() const {
        return ExpressionType::Token;
    }
};

class MemberAccessExpression : public Expression {
  public:
    Expression *source;
    String member;

    virtual ExpressionType getType() const {
        return ExpressionType::MemberAccess;
    }
};

class FunctionCallExpression : public Expression {
  public:
    Expression *source = nullptr;
    String name;
    Vector<Expression *> args;

    virtual ExpressionType getType() const {
        return ExpressionType::FunctionCall;
    }
};
} // namespace tel
