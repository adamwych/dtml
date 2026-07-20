#include "tel-json.hh"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <memory>

TEST_CASE("parseJson parses a simple object") {
    std::unique_ptr<tel::Value> value(tel::parseJson(R"({
        "name": "Ada",
        "age": 42,
        "active": true,
        "tags": ["math", "logic"],
        "meta": null
    })"));

    REQUIRE(value->isRecord());

    auto record = value->asRecord();
    REQUIRE(record->properties.at("name")->isString());
    CHECK(record->properties.at("name")->asString()->value == "Ada");

    REQUIRE(record->properties.at("age")->isNumber());
    CHECK(std::fabs(record->properties.at("age")->asNumber()->value - 42.0) < 0.0001);

    REQUIRE(record->properties.at("active")->isBoolean());
    CHECK(record->properties.at("active")->asBoolean()->value);

    CHECK(record->properties.at("meta")->isNull());

    auto tags = record->properties.at("tags");
    REQUIRE(tags->isArray());
    REQUIRE(tags->asArray()->size() == 2);
    CHECK(tags->asArray()->at(0)->asString()->value == "math");
    CHECK(tags->asArray()->at(1)->asString()->value == "logic");
}

TEST_CASE("parseJson decodes unicode escapes") {
    std::unique_ptr<tel::Value> value(tel::parseJson(R"({
        "accented": "caf\u00E9",
        "emoji": "\uD83D\uDE00",
        "escaped": "\\u00E9",
        "newline": "one\ntwo"
    })"));

    REQUIRE(value->isRecord());

    auto record = value->asRecord();
    REQUIRE(record->properties.at("accented")->isString());
    CHECK(record->properties.at("accented")->asString()->value == "caf\xC3\xA9");

    REQUIRE(record->properties.at("emoji")->isString());
    CHECK(record->properties.at("emoji")->asString()->value == "\xF0\x9F\x98\x80");

    REQUIRE(record->properties.at("escaped")->isString());
    CHECK(record->properties.at("escaped")->asString()->value == "\\u00E9");

    REQUIRE(record->properties.at("newline")->isString());
    CHECK(record->properties.at("newline")->asString()->value == "one\ntwo");
}

TEST_CASE("parseJson repairs unsafe unicode string content") {
    auto replacement = String("\xEF\xBF\xBD");

    std::unique_ptr<tel::Value> value(tel::parseJson(R"({
        "lead": "\uD83D",
        "trail": "\uDE00",
        "nul": "\u0000",
        "escape": "\u001B",
        "tab": "\t"
    })"));

    REQUIRE(value->isRecord());

    auto record = value->asRecord();
    REQUIRE(record->properties.at("lead")->isString());
    CHECK(record->properties.at("lead")->asString()->value == replacement);

    REQUIRE(record->properties.at("trail")->isString());
    CHECK(record->properties.at("trail")->asString()->value == replacement);

    REQUIRE(record->properties.at("nul")->isString());
    CHECK(record->properties.at("nul")->asString()->value == replacement);

    REQUIRE(record->properties.at("escape")->isString());
    CHECK(record->properties.at("escape")->asString()->value == replacement);

    REQUIRE(record->properties.at("tab")->isString());
    CHECK(record->properties.at("tab")->asString()->value == "\t");
}

TEST_CASE("parseJson repairs invalid raw utf8 inside strings") {
    auto json = String(R"({"bad":")");
    json.push_back(static_cast<char>(0xFF));
    json.append(R"("})");

    std::unique_ptr<tel::Value> value(tel::parseJson(json));

    REQUIRE(value->isRecord());

    auto record = value->asRecord();
    REQUIRE(record->properties.at("bad")->isString());
    CHECK(record->properties.at("bad")->asString()->value == "\xEF\xBF\xBD");
}
