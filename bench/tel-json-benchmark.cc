#include "tel-json.hh"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>

TEST_CASE("benchmark parseJson", "[!benchmark]") {
    const String json = R"({
        "name": "Ada",
        "age": 42,
        "active": true,
        "tags": ["math", "logic"],
        "meta": null
    })";

    BENCHMARK("parse simple object") {
        std::unique_ptr<tel::Value> value(tel::fromJson(json));
        return value->asRecord()->properties.at("tags")->asArray()->size();
    };
}
