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
