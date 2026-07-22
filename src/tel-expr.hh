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

struct TokenExpression;
struct MemberAccessExpression;
struct FunctionCallExpression;

struct Expression {
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

struct TokenExpression : public Expression {
    ExpressionToken token;

    virtual ExpressionType getType() const {
        return ExpressionType::Token;
    }
};

struct MemberAccessExpression : public Expression {
    Expression *source;
    String member;

    virtual ExpressionType getType() const {
        return ExpressionType::MemberAccess;
    }
};

struct FunctionCallExpression : public Expression {
    Expression *source = nullptr;
    String name;
    Vector<Expression *> args;

    virtual ExpressionType getType() const {
        return ExpressionType::FunctionCall;
    }
};
} // namespace tel
