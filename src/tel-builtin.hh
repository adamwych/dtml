#pragma once

#include "common.hh"
#include "tel-builtin-helper.hh"
#include "tel.hh"
#include "utils.hh"

namespace tel {
namespace builtin {

BUILTIN(Array, map, {
    REQUIRE_ARG(0, String)

    auto result = new ArrayValue();
    result->value.reserve(thisArray.size());

    for (auto element : thisArray) {
        if (!element || !element->isRecord()) {
            result->value.push_back(new NullValue());
            continue;
        }

        auto props = element->asRecord()->value;
        if (props.count(arg0) == 0) {
            result->value.push_back(new NullValue());
            continue;
        }

        result->value.push_back(props[arg0]);
    }

    return result;
})

BUILTIN(Array, join, {
    REQUIRE_ARG(0, String)

    ValuePrinter printer;
    printer.skipContainers = true;
    printer.quoteStrings = false;

    auto result = String();
    for (auto i = 0; i < thisArray.size(); i++) {
        if (i > 0) {
            result.append(arg0);
        }
        printer.print(thisArray[i], result);
    }
    return new StringValue(std::move(result));
})

/* clang-format off */
BUILTIN_STANDALONE(stringify, {
	return new StringValue(std::move(tel::toJson(thisValue)));
})

BUILTIN(Number, round, {
	return new StringValue(std::move(std::to_string(static_cast<int>(::round(thisNumber)))));
})

BUILTIN(Array, length, {
	return new NumberValue(thisArray.size());
})
/* clang-format on */

BUILTIN_STANDALONE(contains, {
    if (thisValue->isArray()) {
        REQUIRE_THIS(Array)
        REQUIRE_ARG_ANY(0)
        return new BooleanValue(thisArrayValue->contains(arg0Value));
    }

    if (thisValue->isRecord()) {
        REQUIRE_THIS(Record)
        REQUIRE_ARG(0, String)
        return new BooleanValue(thisRecordValue->contains(arg0));
    }

    printf("`.contains()` can only be called on an array or a record\n");
    return new NullValue();
})

BUILTIN_STANDALONE(is, {
    REQUIRE_ARG_ANY(0)
    return new BooleanValue(ValueComparator{}.equals(thisValue, arg0Value));
})

BUILTIN_STANDALONE(not, {
    REQUIRE_ARG_ANY(0)
    return new BooleanValue(!ValueComparator{}.equals(thisValue, arg0Value));
})

BUILTIN(Boolean, or, {
    REQUIRE_ARG(0, Boolean)
    return new BooleanValue(thisBoolean || arg0);
})

BUILTIN(Boolean, and, {
    REQUIRE_ARG(0, Boolean)
    return new BooleanValue(thisBoolean && arg0);
})

static Value *call(const FunctionCall &call) {
#define TRY(func, alias)                                                                           \
    if (call.name == alias)                                                                        \
    return func(call)

    TRY(map_, "map");
    TRY(join_, "join");
    TRY(stringify_, "stringify");
    TRY(round_, "round");
    TRY(contains_, "contains");
    TRY(length_, "length");
    TRY(is_, "is");
    TRY(not_, "not");
    TRY(or_, "or");
    TRY(and_, "and");

    printf("%s\n", format("Call to undefined function '%s'", call.name.c_str()).c_str());
    return new NullValue();
}

} // namespace builtin
} // namespace tel
