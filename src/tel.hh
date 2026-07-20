#pragma once

// Tiny Expression Language

#include "common.hh"
#include "tel-expr.hh"
#include "tel-json.hh"
#include "tel-value.hh"

namespace tel {
struct EvaluationContext {
    Map<String, Value *> globals;

    void addGlobal(const String &name, Value *value) {
        globals[name] = value;
    }

    void mergeFrom(const EvaluationContext &other) {
        for (auto [key, value] : other.globals) {
            globals[key] = value;
        }
    }
};

class Interpreter {
    EvaluationContext _ctx;

    Value *evaluateToken(const TokenExpression *expr);
    Value *evaluateMemberAccess(const MemberAccessExpression *expr);
    Value *evaluateFunctionCall(const FunctionCallExpression *expr);

    Value *evaluateInterpolatedString(const String &text);

  public:
    Interpreter(EvaluationContext ctx) : _ctx(ctx) {
    }

    Value *evaluate(const StringView &text);
    Value *evaluate(const Expression *expr);
};
} // namespace tel
