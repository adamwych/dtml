#include "../src/tpl.hh"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <memory>

dtml::TemplateEvaluationResult *evaluateTemplate(const String &text) {
    auto tpl = new dtml::Template(text);
    auto eval = new dtml::TemplateEvaluator(tpl);
    auto cache = new dtml::TemplateCache();
    auto evalCtx = new dtml::TemplateEvaluationContext(cache, "{}", "{}");
    return eval->evaluate(evalCtx);
}

void CHECK_OK(const String &tpl, const String &expectedOutput) {
    auto result = evaluateTemplate(tpl);
    CHECK(!result->isError());
    CHECK(result->getText() == expectedOutput);
}

void CHECK_FAIL(const String &tpl, const String &expectedError) {
    auto result = evaluateTemplate(tpl);
    CHECK(result->isError());
    CHECK(result->getText() == expectedError);
}

TEST_CASE("tpl: parse elements") {
    CHECK_OK("<empty></empty>", "<empty></empty>");
    CHECK_OK("<dashes-in-name> </dashes-in-name>", "<dashes-in-name> </dashes-in-name>");
    CHECK_OK("<preserve-space> </preserve-space>", "<preserve-space> </preserve-space>");
    CHECK_OK("<skip-extra-spaces    > </skip-extra-spaces>",
             "<skip-extra-spaces> </skip-extra-spaces>");
    CHECK_OK("<numbers-in-123></numbers-in-123>", "<numbers-in-123></numbers-in-123>");
    CHECK_OK("<namespace:foo></namespace:foo>", "<namespace:foo></namespace:foo>");
    CHECK_OK("<under_scored></under_scored>", "<under_scored></under_scored>");
    CHECK_OK("<foo><!-- ignore comments --></foo>", "<foo></foo>");
    CHECK_OK("<!-- <foo></foo> -->", "");
    CHECK_OK("<parent><child><childchild></childchild></child></parent>",
             "<parent><child><childchild></childchild></child></parent>");
    CHECK_OK("<newline>\nBar\n</newline>", "<newline>\nBar\n</newline>");
    CHECK_OK("<foo"
             "	one=\"one\""
             "	two=\"line1"
             "line2\""
             ">"
             "</foo>",
             "<foo one=\"one\" two=\"line1line2\"></foo>");
    CHECK_OK("<textarea readonly class=\"test\"></textarea>",
             "<textarea class=\"test\" readonly=\"true\"></textarea>");

    CHECK_FAIL("<foo>Bar", "element 'foo' is not closed");
    CHECK_FAIL("<foo attr=1223></foo>", "element attribute value must be quoted");
    CHECK_FAIL("<123></123>", "unexpected element end tag");
    CHECK_FAIL("<!--", "unexpected end of file while parsing comment");
    CHECK_FAIL("<element", "unexpected end of file while parsing element tag name");
    CHECK_FAIL("</end>", "unexpected element end tag");
    CHECK_FAIL("<b>foo <i>bar</b> baz</i>", "mismatched element end tag");
    CHECK_FAIL("<foo></bar>", "mismatched element end tag");
    CHECK_FAIL("<element></element", "unexpected end of file while parsing element tag name");
}

TEST_CASE("tpl: void elements") {
    // HTML defines a couple void elements which are allowed to use the shorter notation ...
    CHECK_OK("<meta charset=\"utf-8\" />", "<meta charset=\"utf-8\"/>");
    CHECK_OK("<link rel=\"stylesheet\" />", "<link rel=\"stylesheet\"/>");

    // ... but we also allow custom elements to use that notation, and we expand them to legal HTML.
    CHECK_OK("<expand-voids />", "<expand-voids></expand-voids>");
}

TEST_CASE("tpl: evaluate elements") {
    CHECK_OK("<implicit-true attr>Bar</implicit-true>",
             "<implicit-true attr=\"true\">Bar</implicit-true>");
    CHECK_OK("<empty-attr attr=\"\">Bar</empty-attr>", "<empty-attr attr=\"\">Bar</empty-attr>");
    CHECK_OK("<foo attr=\"\">Bar</foo> <bar>Baz</bar>", "<foo attr=\"\">Bar</foo> <bar>Baz</bar>");
    CHECK_OK("<foo @attr=\"\">Bar</foo>", "<foo @attr=\"\">Bar</foo>");
    CHECK_OK("<foo _attr=\"\">Bar</foo>", "<foo _attr=\"\">Bar</foo>");
    CHECK_OK("<foo _=\"\">Bar</foo>", "<foo _=\"\">Bar</foo>");
}

TEST_CASE("tpl: evaluate expressions") {
    CHECK_OK("${'eval expr'}", "eval expr");
    CHECK_OK("<div>${'eval expr'}</div>", "<div>eval expr</div>");
    CHECK_OK("<div>{'regular text'}</div>", "<div>{'regular text'}</div>");
    CHECK_OK("<div>$regular</div>", "<div>$regular</div>");
    CHECK_OK("<foo attr=\"${'test'}\">Bar</foo>", "<foo attr=\"test\">Bar</foo>");
    CHECK_OK("<foo attr=\"${123}\">Bar</foo>", "<foo attr=\"123.000000\">Bar</foo>");
    CHECK_OK("<foo attr=\"${}\">Bar</foo>", "<foo attr=\"null\">Bar</foo>");
    CHECK_OK("<foo attr=\"{}\">Bar</foo>", "<foo attr=\"{}\">Bar</foo>");
}

TEST_CASE("tpl: <repeat /> element") {
    CHECK_OK("<repeat foreach=\"item\" of=\"[]\">${item.title}</repeat>", "");

    // <repeat foreach="item" of="[{ 'title': 'one' }]">
    //   ${item.title}
    // </repeat>
    //
    // bar
    CHECK_OK("<repeat foreach=\"item\" of=\"[{ &quot;title&quot;: &quot;bar&quot; "
             "}]\">${item.title}</repeat>",
             "bar");

    // <repeat foreach="item" of="[{ 'title': 'one' }, { 'title': 'two' }]">
    //   <span>${item.title}</span>
    // </repeat>
    //
    // <span>one</span><span>two</span>
    CHECK_OK("<repeat foreach=\"item\" of=\"[{ &quot;title&quot;: &quot;one&quot; }, { "
             "&quot;title&quot;: &quot;two&quot; }]\"><span>${item.title}</span></repeat>",
             "<span>one</span><span>two</span>");
}

TEST_CASE("tpl: <partial /> element") {
    // <partial of="{ 'title': 'bar' }" as="item" />
    // <div>${item.title}</div>
    //
    // <script>const item = { "title": "bar" };</script><div>bar</div>
    CHECK_OK("<partial of=\"{ &quot;title&quot;: &quot;bar&quot; }\" as=\"item\" "
             "/><div>${item.title}</div>",
             "<script>const item = { \"title\": \"bar\" };</script><div>bar</div>");
}

TEST_CASE("tpl: <if /> element") {
    CHECK_OK("<if _=\"123.is(123)\">test</if>", "test");
    CHECK_OK("<if _=\"123.is(0)\">test</if>", "");
    CHECK_OK("<if _=\"123.not(123)\">test</if>", "");
    CHECK_OK("<if _=\"123.not(0)\">test</if>", "test");
    CHECK_OK("<if _=\"'str'.is('str')\">test</if>", "test");
    CHECK_OK("<if _=\"variable.is('str')\">test</if>", "");
    CHECK_OK("<if _=\"variable.is(abc)\">test</if>", "test");
    CHECK_OK("<if _=\"123.is(0).or(123.not(0))\">test</if>", "test");
    CHECK_OK("<if _=\"0.is(1)\"> <if _=\"0.is(0)\">nested</if> nested2 </if>", "");
    CHECK_OK("<if _=\"0.is(0)\"> <if _=\"0.is(1)\">nested</if> nested2 </if>", "  nested2 ");
    CHECK_OK("<if _=\"0.is(1)\"> <if _=\"0.is(0)\">nested</if> nested2 </if> other", " other");
}
