#include "utils.hh"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <memory>

TEST_CASE("replaceCustomProtocol") {
	Map<String, String> customProtocols;
	customProtocols["example://"] = "https://example.com";
	
	auto result = replaceCustomProtocol("example://test.html", customProtocols);
	CHECK(result == "https://example.com/test.html");
}
