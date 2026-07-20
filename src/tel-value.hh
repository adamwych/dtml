#pragma once

#include "common.hh"

namespace tel {
enum class ValueType {
    Null = 0,
    Array = 1,
    Record = 2,
    String = 3,
    Number = 4,
    Boolean = 5,
};

class NullValue;
class ArrayValue;
class RecordValue;
class StringValue;
class NumberValue;
class BooleanValue;

class Value {
  public:
    virtual ~Value() = default;

    /// Gets human-readable representation of this value.
    virtual String print() = 0;

    /// Gets the type disciminator of this value.
    virtual ValueType getType() = 0;

    /* clang-format off */
    inline bool isNull()    { return getType() == ValueType::Null; }
    inline bool isArray()   { return getType() == ValueType::Array; }
    inline bool isRecord()  { return getType() == ValueType::Record; }
    inline bool isString()  { return getType() == ValueType::String; }
    inline bool isNumber()  { return getType() == ValueType::Number; }
    inline bool isBoolean() { return getType() == ValueType::Boolean; }

    NullValue *asNull()       { return (NullValue *)this; }
    ArrayValue *asArray()     { return (ArrayValue *)this; }
    RecordValue *asRecord()   { return (RecordValue *)this; }
    StringValue *asString()   { return (StringValue *)this; }
    NumberValue *asNumber()   { return (NumberValue *)this; }
    BooleanValue *asBoolean() { return (BooleanValue *)this; }
    /* clang-format on */
};

class NullValue : public Value {
  public:
    virtual String print() {
        return "null";
    }
    virtual ValueType getType() {
        return ValueType::Null;
    }
};

class ArrayValue : public Value {
  public:
    Vector<Value *> elements;

    inline Value *at(int index) {
        return elements[index];
    }

    inline int size() {
        return elements.size();
    }

    virtual String print() {
        auto out = String();
        out.append("[");
        for (auto elementIdx = 0; elementIdx < elements.size(); elementIdx++) {
            auto element = elements[elementIdx];
            out.append(element->print());
            if (elementIdx != elements.size() - 1) {
                out.append(", ");
            }
        }
        out.append("]");
        return out;
    }
    virtual ValueType getType() {
        return ValueType::Array;
    }
};

class RecordValue : public Value {
  public:
    Map<String, Value *> properties;

    virtual String print() {
        auto out = String();
        out.append("{");
        for (auto [name, value] : properties) {
            out.append(name);
            out.append(" = ");
            out.append(value->print());
            out.append(", ");
        }
        out.append("}");
        return out;
    }
    virtual ValueType getType() {
        return ValueType::Record;
    }
};

class StringValue : public Value {
  public:
    String value;

    explicit StringValue(const char *value) : value(value) {
    }
    explicit StringValue(String value) : value(value) {
    }
    explicit StringValue(StringView value) : value(value) {
    }

    virtual String print() {
        auto out = String();
        out.append("\"");
        out.append(value);
        out.append("\"");
        return out;
    }
    virtual ValueType getType() {
        return ValueType::String;
    }
};

class NumberValue : public Value {
  public:
    double value;

    explicit NumberValue(double value) : value(value) {
    }

    virtual String print() {
        return std::to_string(value);
    }
    virtual ValueType getType() {
        return ValueType::Number;
    }
};

class BooleanValue : public Value {
  public:
    bool value;

    explicit BooleanValue(bool value) : value(value) {
    }

    virtual String print() {
        return value ? "true" : "false";
    }
    virtual ValueType getType() {
        return ValueType::Boolean;
    }
};

/// Gets human-readable representation of a `tel::Value`s.
/// Returns an empty string for containers (arrays and records).
static String printValueSafe(Value *value) {
    switch (value->getType()) {
    case ValueType::Null:
        return "null";
    case ValueType::String:
        return value->asString()->value;
    case ValueType::Number:
        return std::to_string(value->asNumber()->value);
    case ValueType::Boolean:
        return value->asBoolean()->value ? "true" : "false";
    default:
        return String();
    }
}
} // namespace tel