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
    const ExpressionType type;

  protected:
    explicit Expression(ExpressionType type) : type(type) {}

  public:
    virtual ~Expression() = default;

    Expression(const Expression &) = delete;
    Expression &operator=(const Expression &) = delete;
    Expression(Expression &&) = delete;
    Expression &operator=(Expression &&) = delete;

    /* clang-format off */
    inline bool isToken()        const { return type == ExpressionType::Token; }
    inline bool isMemberAccess() const { return type == ExpressionType::MemberAccess; }
    inline bool isFunctionCall() const { return type == ExpressionType::FunctionCall; }

	TokenExpression *asToken()               { return (TokenExpression *)this; }
	MemberAccessExpression *asMemberAccess() { return (MemberAccessExpression *)this; }
    FunctionCallExpression *asFunctionCall() { return (FunctionCallExpression *)this; }

	const TokenExpression *asToken()               const { return (const TokenExpression *)this; }
	const MemberAccessExpression *asMemberAccess() const { return (const MemberAccessExpression *)this; }
    const FunctionCallExpression *asFunctionCall() const { return (const FunctionCallExpression *)this; }
    /* clang-format on */
};

struct TokenExpression : public Expression {
    ExpressionToken token;

    explicit TokenExpression() : Expression(ExpressionType::Token) {}
};

struct MemberAccessExpression : public Expression {
    Expression *source;
    String member;

    explicit MemberAccessExpression() : Expression(ExpressionType::MemberAccess) {}
};

struct FunctionCallExpression : public Expression {
    Expression *source = nullptr;
    String name;
    Vector<Expression *> args;

    explicit FunctionCallExpression() : Expression(ExpressionType::FunctionCall) {}
};
} // namespace tel
