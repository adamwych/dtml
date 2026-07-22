#include "tel-json.hh"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <memory>

TEST_CASE("expr: fromJson parses a simple object") {
    std::unique_ptr<tel::Value> value(tel::fromJson(R"({
        "name": "Ada",
        "age": 42,
        "active": true,
        "tags": ["math", "logic"],
        "meta": null
    })"));

    REQUIRE(value->isRecord());

    auto record = value->asRecord();
    REQUIRE(record->value.at("name")->isString());
    CHECK(record->value.at("name")->asString()->value == "Ada");

    REQUIRE(record->value.at("age")->isNumber());
    CHECK(std::fabs(record->value.at("age")->asNumber()->value - 42.0) < 0.0001);

    REQUIRE(record->value.at("active")->isBoolean());
    CHECK(record->value.at("active")->asBoolean()->value);

    CHECK(record->value.at("meta")->isNull());

    auto tags = record->value.at("tags");
    REQUIRE(tags->isArray());
    REQUIRE(tags->asArray()->size() == 2);
    CHECK(tags->asArray()->at(0)->asString()->value == "math");
    CHECK(tags->asArray()->at(1)->asString()->value == "logic");
}

TEST_CASE("expr: fromJson decodes unicode escapes") {
    std::unique_ptr<tel::Value> value(tel::fromJson(R"({
        "accented": "caf\u00E9",
        "emoji": "\uD83D\uDE00",
        "escaped": "\\u00E9",
        "newline": "one\ntwo"
    })"));

    REQUIRE(value->isRecord());

    auto record = value->asRecord();
    REQUIRE(record->value.at("accented")->isString());
    CHECK(record->value.at("accented")->asString()->value == "caf\xC3\xA9");

    REQUIRE(record->value.at("emoji")->isString());
    CHECK(record->value.at("emoji")->asString()->value == "\xF0\x9F\x98\x80");

    REQUIRE(record->value.at("escaped")->isString());
    CHECK(record->value.at("escaped")->asString()->value == "\\u00E9");

    REQUIRE(record->value.at("newline")->isString());
    CHECK(record->value.at("newline")->asString()->value == "one\ntwo");
}

TEST_CASE("expr: fromJson repairs unsafe unicode string content") {
    auto replacement = String("\xEF\xBF\xBD");

    std::unique_ptr<tel::Value> value(tel::fromJson(R"({
        "lead": "\uD83D",
        "trail": "\uDE00",
        "nul": "\u0000",
        "escape": "\u001B",
        "tab": "\t"
    })"));

    REQUIRE(value->isRecord());

    auto record = value->asRecord();
    REQUIRE(record->value.at("lead")->isString());
    CHECK(record->value.at("lead")->asString()->value == replacement);

    REQUIRE(record->value.at("trail")->isString());
    CHECK(record->value.at("trail")->asString()->value == replacement);

    REQUIRE(record->value.at("nul")->isString());
    CHECK(record->value.at("nul")->asString()->value == replacement);

    REQUIRE(record->value.at("escape")->isString());
    CHECK(record->value.at("escape")->asString()->value == replacement);

    REQUIRE(record->value.at("tab")->isString());
    CHECK(record->value.at("tab")->asString()->value == "\t");
}

TEST_CASE("expr: fromJson repairs invalid raw utf8 inside strings") {
    auto json = String(R"({"bad":")");
    json.push_back(static_cast<char>(0xFF));
    json.append(R"("})");

    std::unique_ptr<tel::Value> value(tel::fromJson(json));

    REQUIRE(value->isRecord());

    auto record = value->asRecord();
    REQUIRE(record->value.at("bad")->isString());
    CHECK(record->value.at("bad")->asString()->value == "\xEF\xBF\xBD");
}
