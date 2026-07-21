#include "utils.hh"
#include "utils-enc-html.hh"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <memory>

TEST_CASE("replaceCustomProtocol") {
	Map<String, String> customProtocols;
	customProtocols["example://"] = "https://example.com";
	
	auto result = replaceCustomProtocol("example://test.html", customProtocols);
	CHECK(result == "https://example.com/test.html");
}

TEST_CASE("decodeHTML decodes named entities") {
    auto result = dtml::decodeHTML("&lt;span title=&quot;Tom &amp; Jerry&apos;s&quot;&gt;Hi&lt;/span&gt;");
    CHECK(result == "<span title=\"Tom & Jerry's\">Hi</span>");
}

TEST_CASE("decodeHTML decodes numeric entities") {
    CHECK(dtml::decodeHTML("caf&#233;") == "caf\xC3\xA9");
    CHECK(dtml::decodeHTML("caf&#xE9;") == "caf\xC3\xA9");
    CHECK(dtml::decodeHTML("face: &#x1F600;") == "face: \xF0\x9F\x98\x80");
}

TEST_CASE("decodeHTML preserves unknown and incomplete entities") {
    CHECK(dtml::decodeHTML("&madeup; &amp &") == "&madeup; &amp &");
}

TEST_CASE("decodeHTML repairs invalid numeric code points") {
    auto replacement = String("\xEF\xBF\xBD");
    CHECK(dtml::decodeHTML("bad: &#xD800;") == "bad: " + replacement);
    CHECK(dtml::decodeHTML("bad: &#x110000;") == "bad: " + replacement);
}
