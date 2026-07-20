#include "tel-json.hh"
#include "utils-unicode.hh"

#include <iomanip>
#include <iostream>
#include <string>

#define JSMN_STATIC
#define JSMN_STRICT
#include "third-party/jsmn.h"

namespace tel {
struct JsonParseContext {
    StringView json;
    jsmntok_t *tokens;
    int pos;
    String error;
};

static Value *jsonParseError(JsonParseContext *ctx, const String &message) {
    ctx->error = message;
    return new NullValue();
}

static Value *parseJsonValue(JsonParseContext *ctx);

static Value *parseJsonArray(JsonParseContext *ctx) {
    auto array = new ArrayValue();

    auto elementCount = ctx->tokens[ctx->pos - 1].size;
    for (auto elementIdx = 0; elementIdx < elementCount; elementIdx++) {
        array->elements.push_back(parseJsonValue(ctx));
    }

    return array;
}

static Value *parseJsonObject(JsonParseContext *ctx) {
    auto record = new RecordValue();

    auto elementCount = ctx->tokens[ctx->pos - 1].size;
    for (auto elementIdx = 0; elementIdx < elementCount; elementIdx++) {
        auto propertyKeyValue = parseJsonValue(ctx);
        if (!propertyKeyValue->isString()) {
            return jsonParseError(ctx, "Record property key must be a string");
        }
        auto propertyKey = propertyKeyValue->asString();
        auto propertyValue = parseJsonValue(ctx);
        record->properties[propertyKey->value] = propertyValue;
    }

    return record;
}

static Value *parseJsonString(JsonParseContext *ctx) {
    auto token = ctx->tokens[ctx->pos - 1];
    auto tokenValue = String(ctx->json.substr(token.start, token.end - token.start));
    return new StringValue(decodeJsonStringEscapes(tokenValue));
}

static Value *parseJsonPrimitive(JsonParseContext *ctx) {
    auto token = ctx->tokens[ctx->pos - 1];

    auto tokenValue = ctx->json.substr(token.start, token.end - token.start);
    if (tokenValue == "true") {
        return new BooleanValue(true);
    } else if (tokenValue == "false") {
        return new BooleanValue(false);
    } else if (tokenValue == "null") {
        return new NullValue();
    } else {
        return new NumberValue(std::stof(String(tokenValue)));
    }

    return new NullValue();
}

static Value *parseJsonValue(JsonParseContext *ctx) {
    auto token = ctx->tokens[ctx->pos++];
    switch (token.type) {
    case JSMN_ARRAY:
        return parseJsonArray(ctx);
    case JSMN_OBJECT:
        return parseJsonObject(ctx);
    case JSMN_STRING:
        return parseJsonString(ctx);
    case JSMN_PRIMITIVE:
        return parseJsonPrimitive(ctx);
    default:
        return new NullValue();
    }
}

Value *parseJson(const String &json) {
    constexpr int tokenCount = 4096;

    jsmn_parser parser;
    jsmntok_t tokens[tokenCount];

    jsmn_init(&parser);

    auto code = jsmn_parse(&parser, json.c_str(), json.length(), tokens, tokenCount);
    if (code < 0) {
        printf("JSON parse error (1): %d\n", code);
        return new NullValue();
    }

    auto ctx = JsonParseContext{
        .json = json,
        .tokens = tokens,
        .pos = 0,
    };
    auto value = parseJsonValue(&ctx);

    if (ctx.error.length() > 0) {
        printf("JSON parse error (2): %s\n", ctx.error.c_str());
        return new NullValue();
    }

    return value;
}
} // namespace tel
