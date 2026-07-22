#include "tpl.hh"
#include "tpl-printer.hh"
#include "utils-enc-html.hh"
#include "utils.hh"

#include "tel.hh"

namespace dtml {
TemplateEvaluationResult *TemplateEvaluator::evaluate(TemplateEvaluationContext *ctx) {
    _error = nullptr;
    _ctx = ctx;

    auto elementStackSizeAtStart = _ctx->elementStack.size();

    if (_c.eat("<!doctype html>")) {
        _p.raw("<!doctype html>");
    }

    while (!_c.isAtEnd() && _error == nullptr) {
        if (_c.isContentWhitespace()) {
            _c.advance();
            _p.raw('\n');
            continue;
        }

        if (_c.eat("<")) {
            auto isComment = _c.eat("!--");
            if (isComment) {
                while (!_c.eat("-->")) {
                    if (!_c.advance()) {
                        error("unexpected end of file while parsing comment");
                        break;
                    }
                }
                continue;
            }

            auto isElementEnd = _c.eat("/");
            if (isElementEnd && !evaluateElementEnd()) {
                break;
            }

            auto isElementStart = _c.isAsciiLetter() || _c.isElementTagSpecialCharacter();
            if (isElementStart && !evaluateElementStart()) {
                break;
            }

            continue;
        }

        if (_c.is("${")) {
            if (!evaluateTextExpression()) {
                break;
            }

            continue;
        }

        _p.raw(_c.current());
        _c.advance();
    }

    if (_error != nullptr) {
        return _error;
    }

    // Context and stacks are shared when evaluating nested `<partial />` elements,
    // so we can't just check if `elementStack` is empty, but it should match the
    // size it was before parsing.
    auto elementStackSizeAtEnd = _ctx->elementStack.size();
    if (elementStackSizeAtStart != elementStackSizeAtEnd) {
        auto element = _ctx->elementStack.top();
        error(format("element '%s' is not closed", element->name.c_str()));
        return _error;
    }

    return new TemplateEvaluationResult(_p.getOutput());
}

bool TemplateEvaluator::readString(String *out) {
    auto start = _c.pos();

    while (!_c.is('"')) {
        if (!_c.advance()) {
            return error("unexpected end of file while parsing a string");
        }
    }

    auto end = _c.pos();

    *out = _source.substr(start, end - start);
    return true;
}

Option<StringView> TemplateEvaluator::readElementName() {
    auto start = _c.pos();

    while (true) {
        if (_c.peek(' ') || _c.peek('\t') || _c.peek('\n') || _c.peek('/') || _c.peek('>')) {
            break;
        }

        if (!_c.advance()) {
            error("unexpected end of file while parsing element tag name");
            return Nullopt;
        }

        if (!_c.isAsciiAlphanumeric() && !_c.isElementTagSpecialCharacter()) {
            error("unexpected character while parsing element tag name");
            return Nullopt;
        }
    }

    auto end = _c.pos();
    return _source.substr(start, end - start + 1);
}

Option<Map<StringView, String>> TemplateEvaluator::readElementAttributes() {
    Map<StringView, String> map;

    while (true) {
        // Skip whitespace between attributes e.g. `attrone="foo"     attrtwo="two"`
        if (!_c.eatWhitespace()) {
            error("unexpected end of file while parsing element attributes");
            return Nullopt;
        }

        auto nameStart = _c.pos() + 1;

        while (true) {
            if (!_c.advance()) {
                error("unexpected end of file while parsing element attribute name");
                return Nullopt;
            }

            if (_c.is(' ')) {
                _c.eatWhitespace();
                _c.advance();
                break;
            }

            if (_c.is('=') || _c.is('/') || _c.is('>')) {
                break;
            }
        }

        auto nameEnd = _c.pos();
        auto name = _source.substr(nameStart, nameEnd - nameStart);

        if (_c.is("=\"")) {
            _c.advance(2); // Skip over start `="`

            String value;
            if (!readString(&value)) {
                return Nullopt;
            }

            map[name] = value;

            continue;
        } else if (_c.is('/') || _c.is('>')) {
            if (name.length() > 0) {
                map[name] = "true";
            }

            break;
        }

        error("unexpected character while parsing element attribute");
        return Nullopt;
    }

    return map;
}

tel::EvaluationContext TemplateEvaluator::createExpressionEvaluationContext() {
    auto evalContext = tel::EvaluationContext{};

    if (!_ctx->partialStack.empty()) {
        auto partial = _ctx->partialStack.top();
        evalContext.addGlobal("props", partial->attributesRecord);
    }

    if (!_ctx->repeatStack.empty()) {
        auto repeat = _ctx->repeatStack.top();
        auto item = repeat->array->at(repeat->arrayItemIndex);
        if (item) {
            evalContext.addGlobal(repeat->attributes["foreach"], item);
        }
    }

    evalContext.mergeFrom(_ctx->exprContext);

    return evalContext;
}

bool TemplateEvaluator::evaluateTextExpression() {
    auto parentElement = _ctx->elementStack.size() > 0 ? _ctx->elementStack.top() : nullptr;
    if (parentElement) {
        if (parentElement->name == "script" || parentElement->name == "style") {
            _p.raw("$");
            _c.advance();
            return true;
        }
    }

    auto start = _c.pos();

    auto depth = 0;
    while (true) {
        if (!_c.advance()) {
            return error("unexpected end of file while parsing text expression");
        }

        if (_c.is('{')) {
            depth++;
        } else if (_c.is('}')) {
            depth--;

            if (depth <= 0) {
                _c.advance();
                break;
            }
        }
    }

    auto end = _c.pos();
    auto text = String(_source.substr(start, end - start));

    if (!isInsideSkippedBlock()) {
        auto interpreter = tel::Interpreter{createExpressionEvaluationContext()};
        auto evaluatedValue = interpreter.evaluateInterpolatedString(text);
        _p.raw(encodeHTML(tel::toJson(evaluatedValue, /*quoteStrings*/ false)));
    }

    return true;
}

bool TemplateEvaluator::evaluateElementAttributes(Map<StringView, String> *attributes) {
    auto interpreter = tel::Interpreter{createExpressionEvaluationContext()};

    for (auto [name, textValue] : *attributes) {
        auto evaluatedValue = interpreter.evaluateInterpolatedString(textValue);

        if (evaluatedValue->isArray() || evaluatedValue->isRecord()) {
            printf("Arrays and Records are not automatically serialized.\n");
        }

        (*attributes)[name] = encodeHTML(tel::toJson(evaluatedValue, /*quoteStrings*/ false));
    }

    return true;
}

static Element *instantiateElement(const StringView &name) {
    if (name == "repeat") {
        return new RepeatElement();
    }
    if (name == "partial") {
        return new PartialElement();
    }
    if (name == "if") {
        return new IfElement();
    }
    return new Element();
}

bool TemplateEvaluator::evaluateElementStart() {
    auto elementName = readElementName();
    if (!elementName) {
        return false;
    }

    auto elementAttributes = readElementAttributes();
    if (!elementAttributes) {
        return false;
    }

    auto insideSkippedBlock = isInsideSkippedBlock();

    if (!insideSkippedBlock) {
        evaluateElementAttributes(&*elementAttributes);
    }

    auto element = instantiateElement(*elementName);
    element->name = *elementName;
    element->attributes = *elementAttributes;
    element->parent = _ctx->elementStack.empty() ? nullptr : _ctx->elementStack.top();
    if (element->parent) {
        element->parent->children.push_back(element);
    }

    auto isSelfClosing = _c.eat("/>");
    if (!isSelfClosing && _c.eat(">")) {
        _ctx->elementStack.push(element);
    }

    if (element->name == "partial") {
        if (insideSkippedBlock) {
            return true;
        }

        return evaluatePartial(static_cast<PartialElement *>(element));
    }

    if (element->name == "repeat") {
        // A self-closing `<repeat />` has no effect.
        // @todo: Log a warning?
        if (isSelfClosing) {
            delete element;
            return true;
        }

        if (insideSkippedBlock) {
            return true;
        }

        return evaluateRepeatStart(static_cast<RepeatElement *>(element));
    }

    if (element->name == "if") {
        // A self-closing `<if />` has no effect.
        // @todo: Log a warning?
        if (isSelfClosing) {
            delete element;
            return true;
        }

        if (insideSkippedBlock) {
            return true;
        }

        return evaluateIfStart(static_cast<IfElement *>(element));
    }

    _p.printElementSignature(element->name, element->attributes, isSelfClosing);
    return true;
}

bool TemplateEvaluator::evaluateElementEnd() {
    auto elementName = readElementName();
    if (!elementName) {
        return false;
    }

    _c.advance();
    _c.eat(">");

    if (_ctx->elementStack.empty()) {
        return error("unexpected element end tag");
    }

    auto element = _ctx->elementStack.top();
    if (element->name != *elementName) {
        return error("mismatched element end tag");
    }

    if (!_ctx->repeatStack.empty()) {
        auto topRepeat = _ctx->repeatStack.top();
        if (topRepeat == element && evaluateRepeatEnd(topRepeat)) {
            return true;
        }
    }

    if (!_ctx->ifStack.empty()) {
        auto topIf = _ctx->ifStack.top();
        if (topIf == element && evaluateIfEnd(topIf)) {
            return true;
        }
    }

    if (element->name != "repeat" && element->name != "if") {
        _p.printElementClose(*elementName);
    }

    _ctx->elementStack.pop();
    delete element;

    return true;
}

bool TemplateEvaluator::evaluatePartial(PartialElement *element) {
    String srcJson;
    if (element->attributes.count("of") != 0) {
        srcJson = decodeHTML(element->attributes["of"]);
    } else if (element->attributes.count("src") != 0) {
        auto srcResponse = fetch(element->attributes["src"]);
        if (srcResponse && srcResponse->isOk()) {
            srcJson = srcResponse->text;
        }
    }

    if (srcJson.empty()) {
        return true;
    }

    // `into` makes the value available to runtime JavaScript by putting it
    // into an existing JavaScript variable.
    if (element->attributes.count("into") != 0) {
        auto into = element->attributes["into"];
        _p.raw("<script>");
        _p.raw(into);
        _p.raw(" = ");
        _p.raw(srcJson);
        _p.raw(";</script>");
        return true;
    }

    // `as` makes the value available to both runtime JavaScript and TEL expressions.
    if (element->attributes.count("as") != 0) {
        auto as = element->attributes["as"];
        _p.raw("<script>const ");
        _p.raw(as);
        _p.raw(" = ");
        _p.raw(srcJson);
        _p.raw(";</script>");
        _ctx->exprContext.addGlobal(as, tel::fromJson(srcJson));
        return true;
    }

    element->attributesRecord = new tel::RecordValue();
    for (auto [name, value] : element->attributes) {
        element->attributesRecord->properties[String(name)] = new tel::StringValue(value);
    }

    auto tpl = new Template(srcJson.c_str());
    auto evaluator = new TemplateEvaluator(tpl);

    _ctx->partialStack.push(element);
    auto result = evaluator->evaluate(_ctx);
    _ctx->partialStack.pop();

    delete evaluator;
    delete tpl;

    if (result->isError()) {
        return error(result->getText());
    }

    _p.raw(result->getText());

    return true;
}

bool TemplateEvaluator::evaluateRepeatStart(RepeatElement *element) {
    String srcJson;
    if (element->attributes.count("of") > 0) {
        srcJson = decodeHTML(element->attributes["of"]);
    } else if (element->attributes.count("in") > 0) {
        auto srcResponse = fetch(element->attributes["in"]);
        if (srcResponse && srcResponse->isOk()) {
            srcJson = srcResponse->text;
        }
    }

    // Try to parse the response and extract relevant items array from the value.
    auto src = tel::fromJson(srcJson);
    if (src->isArray()) {
        element->array = src->asArray();
    } else if (src->isRecord()) {
        // Fallback for when the server can't directly respond with an array.
        // For example our API IDL requires all endpoints to return Records.
        auto record = src->asRecord();
        if (record->contains("items")) {
            auto items = record->get("items");
            if (items->isArray()) {
                element->array = items->asArray();
            }
        }
    }

    // Be nice and don't panic if the source did not turn out to be a valid array.
    // @todo: We should probably log a warning in this case.
    if (!element->array) {
        element->array = new tel::ArrayValue();
    }

    element->childrenStartPos = _c.pos();

    _ctx->repeatStack.push(element);

    // Even if there's no items, the parser still has to parse child elements to find the closing
    // `</repeat>`. Disable printing to prevent the template from leaking.
    if (element->array->size() == 0) {
        _ctx->emptyRepeatDepth++;
        _p.disablePrinting();
    }

    return true;
}

bool TemplateEvaluator::evaluateRepeatEnd(RepeatElement *element) {
    if (element->array->size() == 0) {
        _ctx->emptyRepeatDepth--;
        _p.enablePrinting();
        _ctx->repeatStack.pop();
        return false;
    }

    element->arrayItemIndex++;

    if (element->arrayItemIndex < element->array->size()) {
        _c.seek(element->childrenStartPos);
        return true;
    }

    _ctx->repeatStack.pop();

    return false;
}

bool TemplateEvaluator::evaluateIfStart(IfElement *element) {
    if (element->attributes.count("_") == 0) {
        return error("`if` element has no condition expression");
    }

    auto interpreter = tel::Interpreter{createExpressionEvaluationContext()};
    auto condition = interpreter.evaluate(decodeHTML(element->attributes["_"]));

    // We do not implicitly cast Arrays/Records/etc. into Booleans.
    element->conditionIsTrue = condition->isBoolean() ? condition->asBoolean()->value : false;

    if (!element->conditionIsTrue) {
        _ctx->falseIfDepth++;
        _p.disablePrinting();
    }

    _ctx->ifStack.push(element);

    return true;
}

bool TemplateEvaluator::evaluateIfEnd(IfElement *element) {
    if (!element->conditionIsTrue) {
        _ctx->falseIfDepth--;
        _p.enablePrinting();
    }

    _ctx->ifStack.pop();

    return false;
}

bool TemplateEvaluator::isInsideEmptyRepeat() {
    return _ctx->emptyRepeatDepth > 0;
}

bool TemplateEvaluator::isInsideFalseIf() {
    return _ctx->falseIfDepth > 0;
}

bool TemplateEvaluator::isInsideSkippedBlock() {
    return isInsideEmptyRepeat() || isInsideFalseIf();
}

bool TemplateEvaluator::error(const String &reason) {
    _error = new TemplateEvaluationResult(reason, _c.getLocation());
    return false;
}

bool TemplateEvaluator::error(const String &reason, TemplateLocation location) {
    _error = new TemplateEvaluationResult(reason, location);
    return false;
}

FetchResponse *TemplateEvaluator::fetch(const String &src) {
    auto url = replaceCustomProtocol(src, _ctx->customProtocols);
    auto canBeCached = stringEndsWith(url, ".html") || stringEndsWith(url, ".json");
    if (!canBeCached || _ctx->cache->fetchCache.count(url) == 0) {
        uint8_t *responseData;
        int responseDataLen;
        int errorCode = -1;

        auto separatorIndex = url.find_first_of('?');
        if (separatorIndex != String::npos) {
            String origin = url.substr(0, separatorIndex);
            String params = url.substr(separatorIndex + 1);
            errorCode =
                doFetchPOST(origin.c_str(), params.c_str(), &responseData, &responseDataLen);
        } else {
            errorCode = doFetchGET(url.c_str(), &responseData, &responseDataLen);
        }

        if (errorCode < 0) {
            printf("Fetch failed: %d\n", errorCode);
            return nullptr;
        }

        _ctx->cache->fetchCache[url] = FetchResponse{
            .statusCode = 200,
            .text = String(reinterpret_cast<const char *>(responseData), responseDataLen),
        };
    }

    return &_ctx->cache->fetchCache[url];
}
} // namespace dtml
