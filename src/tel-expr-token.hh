#pragma once

#include "common.hh"

namespace tel {
enum class ExpressionTokenType {
    Null = 0,
    Identifier = 1,
    String = 2,
    InterpolatedString = 3,
    Error = 255,
};

struct ExpressionToken {
    ExpressionTokenType type;
    String value;

    /* clang-format off */
    inline bool isNull()               { return type == ExpressionTokenType::Null; }
	inline bool isIdentifier()         { return type == ExpressionTokenType::Identifier; }
	inline bool isString()             { return type == ExpressionTokenType::String; }
	inline bool isInterpolatedString() { return type == ExpressionTokenType::InterpolatedString; }
	inline bool isError()              { return type == ExpressionTokenType::Error; }
    /* clang-format on */
};
} // namespace tel