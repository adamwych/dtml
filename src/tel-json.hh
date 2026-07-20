#pragma once

#include "common.hh"
#include "tel-expr.hh"
#include "tel-value.hh"

namespace tel {
Value *parseJson(const String &json);
} // namespace tel