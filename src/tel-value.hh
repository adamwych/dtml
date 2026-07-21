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

struct NullValue;
struct ArrayValue;
struct RecordValue;
struct StringValue;
struct NumberValue;
struct BooleanValue;

struct Value {
    virtual ~Value() = default;

    /// Gets human-readable representation of this value.
    virtual String print() const = 0;

    /// Gets the type disciminator of this value.
    virtual ValueType getType() const = 0;

    /* clang-format off */
    inline bool isNull()      const { return getType() == ValueType::Null; }
    inline bool isArray()     const { return getType() == ValueType::Array; }
    inline bool isRecord()    const { return getType() == ValueType::Record; }
    inline bool isString()    const { return getType() == ValueType::String; }
    inline bool isNumber()    const { return getType() == ValueType::Number; }
    inline bool isBoolean()   const { return getType() == ValueType::Boolean; }

    NullValue *asNull()       const { return (NullValue *)this; }
    ArrayValue *asArray()     const { return (ArrayValue *)this; }
    RecordValue *asRecord()   const { return (RecordValue *)this; }
    StringValue *asString()   const { return (StringValue *)this; }
    NumberValue *asNumber()   const { return (NumberValue *)this; }
    BooleanValue *asBoolean() const { return (BooleanValue *)this; }
    /* clang-format on */
};

struct NullValue : public Value {
    virtual String print() const {
        return "null";
    }
    virtual ValueType getType() const {
        return ValueType::Null;
    }
};

struct ArrayValue : public Value {
    Vector<Value *> elements;

    inline Value *at(int index) {
        return elements[index];
    }

    inline int size() {
        return elements.size();
    }

    virtual String print() const {
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
    virtual ValueType getType() const {
        return ValueType::Array;
    }
};

struct RecordValue : public Value {
    Map<String, Value *> properties;

    bool contains(const String &key) {
        return properties.count(key) != 0;
    }

    Value *get(const String &key) {
        return properties[key];
    }

    virtual String print() const {
        auto idx = 0;
        auto out = String();
        out.append("{");
        for (auto [name, value] : properties) {
            out.append(name);
            out.append(" = ");
            out.append(value->print());
            if (idx++ < properties.size() - 1) {
                out.append(", ");
            }
        }
        out.append("}");
        return out;
    }
    virtual ValueType getType() const {
        return ValueType::Record;
    }
};

struct StringValue : public Value {
    String value;

    explicit StringValue(const char *value) : value(value) {
    }
    explicit StringValue(String value) : value(value) {
    }
    explicit StringValue(StringView value) : value(value) {
    }

    virtual String print() const {
        auto out = String();
        out.append("\"");
        out.append(value);
        out.append("\"");
        return out;
    }
    virtual ValueType getType() const {
        return ValueType::String;
    }
};

struct NumberValue : public Value {
    double value;

    explicit NumberValue(double value) : value(value) {
    }

    virtual String print() const {
        return std::to_string(value);
    }
    virtual ValueType getType() const {
        return ValueType::Number;
    }
};

struct BooleanValue : public Value {
    bool value;

    explicit BooleanValue(bool value) : value(value) {
    }

    virtual String print() const {
        return value ? "true" : "false";
    }
    virtual ValueType getType() const {
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
