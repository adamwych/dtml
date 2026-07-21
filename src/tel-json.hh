#pragma once

#include "common.hh"
#include "tel-expr.hh"
#include "tel-value.hh"

namespace tel {
Value *fromJson(const String &json);
String toJson(const Value *value, bool quoteStrings = true);
} // namespace tel