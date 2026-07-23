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
    const ValueType type;

  protected:
    explicit Value(ValueType type) : type(type) {}

  public:
    virtual ~Value() = default;

    Value(const Value &) = delete;
    Value &operator=(const Value &) = delete;
    Value(Value &&) = delete;
    Value &operator=(Value &&) = delete;

    /* clang-format off */
    inline bool isNull()    const { return type == ValueType::Null; }
    inline bool isArray()   const { return type == ValueType::Array; }
    inline bool isRecord()  const { return type == ValueType::Record; }
    inline bool isString()  const { return type == ValueType::String; }
    inline bool isNumber()  const { return type == ValueType::Number; }
    inline bool isBoolean() const { return type == ValueType::Boolean; }

    NullValue *asNull()       { return (NullValue *)this; }
    ArrayValue *asArray()     { return (ArrayValue *)this; }
    RecordValue *asRecord()   { return (RecordValue *)this; }
    StringValue *asString()   { return (StringValue *)this; }
    NumberValue *asNumber()   { return (NumberValue *)this; }
    BooleanValue *asBoolean() { return (BooleanValue *)this; }

    const NullValue *asNull()       const { return (const NullValue *)this; }
    const ArrayValue *asArray()     const { return (const ArrayValue *)this; }
    const RecordValue *asRecord()   const { return (const RecordValue *)this; }
    const StringValue *asString()   const { return (const StringValue *)this; }
    const NumberValue *asNumber()   const { return (const NumberValue *)this; }
    const BooleanValue *asBoolean() const { return (const BooleanValue *)this; }
    /* clang-format on */
};

/// Represents lack of a value.
struct NullValue : public Value {
    explicit NullValue() : Value(ValueType::Null) {}
};

/// Represents an untyped list of values.
struct ArrayValue : public Value {
    Vector<Value *> value;

    explicit ArrayValue(Vector<Value *> value = {})
        : Value(ValueType::Array),
          value(std::move(value)) {}

    bool contains(const Value *key) const;

    Value *at(int index) {
        return value.at(index);
    }

    const Value *at(int index) const {
        return value.at(index);
    }

    int size() const {
        return value.size();
    }
};

/// Represents a collection of key-value pairs sorted by key.
struct RecordValue : public Value {
    Map<String, Value *> value;

    explicit RecordValue(Map<String, Value *> value = {})
        : Value(ValueType::Record),
          value(std::move(value)) {}

    bool contains(const String &key) const {
        return value.count(key) != 0;
    }

    Value *get(const String &key) {
        return value.at(key);
    }

    const Value *get(const String &key) const {
        return value.at(key);
    }
};

/// Represents a text value.
struct StringValue : public Value {
    String value;

    explicit StringValue(const char *value) : Value(ValueType::String), value(value) {}
    explicit StringValue(StringView value) : Value(ValueType::String), value(value) {}
    explicit StringValue(String value = {}) : Value(ValueType::String), value(std::move(value)) {}
};

/// Represents a 64-bit floating-point decimal.
struct NumberValue : public Value {
    double value;

    explicit NumberValue(double value = {}) : Value(ValueType::Number), value(value) {}
};

/// Represents a boolean `true`/`false` state.
struct BooleanValue : public Value {
    bool value;

    explicit BooleanValue(bool value = {}) : Value(ValueType::Boolean), value(value) {}
};

struct ValueComparator {
    bool equals(const Value *a, const Value *b) const {
        switch (a->type) {
        case ValueType::Null:
            return b->isNull();

        case ValueType::Array:
        case ValueType::Record:
            return false;

        case ValueType::String:
            return b->isString() && a->asString()->value == b->asString()->value;
        case ValueType::Number:
            return b->isNumber() && a->asNumber()->value == b->asNumber()->value;
        case ValueType::Boolean:
            return b->isBoolean() && a->asBoolean()->value == b->asBoolean()->value;

        default:
            return false;
        }
    }
};

struct ValuePrinter {
    /// If set, Arrays and Records will not be printed.
    bool skipContainers = false;
    /// If set, String values will be wrapped in quotes (`"string"` vs `string`).
    bool quoteStrings = true;

    void print(const Value *value, String &out) const {
        switch (value->type) {
        case ValueType::Null:
            out.append("null");
            break;

        case ValueType::Array: {
            if (skipContainers) {
                break;
            }

            const auto &elements = value->asArray()->value;
            auto idx = 0;

            out.append("[");
            for (const auto &element : elements) {
                if (idx++ > 0) {
                    out.append(", ");
                }
                print(element, out);
            }
            out.append("]");

            break;
        }

        case ValueType::Record: {
            if (skipContainers) {
                break;
            }

            const auto &properties = value->asRecord()->value;
            auto idx = 0;

            out.append("{");
            for (const auto &[name, value] : properties) {
                if (idx++ > 0) {
                    out.append(", ");
                }
                out.append(name);
                out.append(" = ");
                print(value, out);
            }
            out.append("}");
            break;
        }

        case ValueType::String:
            if (quoteStrings) {
                out.append("\"");
                out.append(value->asString()->value);
                out.append("\"");
            } else {
                out.append(value->asString()->value);
            }
            break;

        case ValueType::Number:
            out.append(std::to_string(value->asNumber()->value));
            break;

        case ValueType::Boolean:
            out.append(value->asBoolean()->value ? "true" : "false");
            break;
        }
    }

    String print(const Value *value) const {
        String out;
        print(value, out);
        return out;
    }
};

inline bool ArrayValue::contains(const Value *key) const {
    ValueComparator comparator;
    for (auto element : value) {
        if (comparator.equals(element, key)) {
            return true;
        }
    }
    return false;
}
} // namespace tel
