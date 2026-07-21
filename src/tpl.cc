#include "tpl.hh"
#include "tpl-printer.hh"
#include "utils.hh"

#include "tel.hh"

namespace dtml {
TemplateEvaluationResult *TemplateEvaluator::evaluate(TemplateEvaluationContext *ctx) {
    _error = nullptr;
    _ctx = ctx;

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
                        error("unexpected end-of-file while parsing comment");
                        break;
                    }
                }
                continue;
            }

            auto isElementEnd = _c.eat("/");
            if (isElementEnd) {
                if (!evaluateElementEnd()) {
                    break;
                }
            }

            auto isElementStart = _c.isAsciiLetter() || _c.isElementTagSpecialCharacter();
            if (isElementStart) {
                if (!evaluateElementStart()) {
                    break;
                }
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

    return new TemplateEvaluationResult(_p.getOutput());
}

bool TemplateEvaluator::readString(String *out) {
    auto start = _c.pos();

    while (!_c.peek('"')) {
        if (!_c.advance()) {
            return error("Unexpected end of file while parsing a string");
        }
    }

    auto end = _c.pos();

    *out = _source.substr(start, end - start + 1);
    return true;
}

Option<StringView> TemplateEvaluator::readElementName() {
    auto start = _c.pos();

    while (true) {
        if (_c.peek(' ') || _c.peek('\t') || _c.peek('\n') || _c.peek('/') || _c.peek('>')) {
            break;
        }

        if (!_c.advance()) {
            error("Unexpected end of file while parsing element start tag");
            return Nullopt;
        }

        if (!_c.isAsciiAlphanumeric() && !_c.isElementTagSpecialCharacter()) {
            error("Unexpected character while parsing element start tag");
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
            error("Unexpected end of file while parsing element attributes");
            return Nullopt;
        }

        auto nameStart = _c.pos() + 1;

        while (true) {
            if (!_c.advance()) {
                error("Unexpected end of file while parsing element attribute name");
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

            _c.advance(); // Skip over end `"`

            map[name] = value;

            continue;
        } else if (_c.is('/') || _c.is('>')) {
            if (name.length() > 0) {
                map[name] = "true";
            }

            break;
        }

        error("Unexpected character while parsing element attribute");
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
            return error("Unexpected end-of-file while parsing text expression");
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

    auto interpreter = tel::Interpreter{createExpressionEvaluationContext()};
    auto evaluatedValue = interpreter.evaluateInterpolatedString(text);
    _p.raw(encodeHTML(printValueSafe(evaluatedValue)));

    return true;
}

bool TemplateEvaluator::evaluateElementAttributes(Map<StringView, String> *attributes) {
    auto interpreter = tel::Interpreter{createExpressionEvaluationContext()};

    for (auto [name, textValue] : *attributes) {
        auto evaluatedValue = interpreter.evaluateInterpolatedString(textValue);

        if (evaluatedValue->isArray() || evaluatedValue->isRecord()) {
            printf("Arrays and Records are not automatically serialized.\n");
        }

        (*attributes)[name] = encodeHTML(printValueSafe(evaluatedValue));
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

    auto insideEmptyRepeat = isInsideEmptyRepeat();

    if (!insideEmptyRepeat) {
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
        if (insideEmptyRepeat) {
            return true;
        }

        return evaluatePartial(static_cast<PartialElement *>(element));
    } else if (element->name == "repeat") {
        if (isSelfClosing) {
            delete element;
            return true;
        }

        if (insideEmptyRepeat) {
            return true;
        }

        return evaluateRepeatStart(static_cast<RepeatElement *>(element));
    } else {
        _p.printElementSignature(element->name, element->attributes, isSelfClosing);
    }

    return true;
}

bool TemplateEvaluator::evaluateElementEnd() {
    auto elementName = readElementName();
    if (!elementName) {
        return false;
    }

    _c.advance();
    _c.eat(">"); // Skip over '>'

    if (_ctx->elementStack.empty()) {
        return error("Unexpected element end tag");
    }

    auto element = _ctx->elementStack.top();
    if (element->name != *elementName) {
        return error("Mismatched element end tag");
    }

    if (!_ctx->repeatStack.empty()) {
        auto topRepeat = _ctx->repeatStack.top();
        if (topRepeat == element && evaluateRepeatEnd(topRepeat)) {
            return true;
        }
    }

    if (element->name != "repeat") {
        _p.printElementClose(*elementName);
    }

    _ctx->elementStack.pop();
    delete element;

    return true;
}

bool TemplateEvaluator::evaluatePartial(PartialElement *element) {
    auto templateResponse = fetch(element->attributes["src"]);
    if (!templateResponse || templateResponse->statusCode != 200) {
        return true;
    }

    if (element->attributes.count("into") != 0) {
        auto into = element->attributes["into"];
        _p.raw("<script>");
        _p.raw(into);
        _p.raw(" = ");
        _p.raw(templateResponse->text);
        _p.raw(";</script>");
        return true;
    }

    if (element->attributes.count("as") != 0) {
        auto as = element->attributes["as"];
        _p.raw("<script>const ");
        _p.raw(as);
        _p.raw(" = ");
        _p.raw(templateResponse->text);
        _p.raw(";</script>");

        auto value = tel::fromJson(templateResponse->text);
        _ctx->exprContext.addGlobal(as, value);

        return true;
    }

    element->attributesRecord = new tel::RecordValue();
    for (auto [name, value] : element->attributes) {
        element->attributesRecord->properties[String(name)] = new tel::StringValue(value);
    }

    auto tpl = new Template(templateResponse->text.c_str());
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
    element->childrenStartPos = _c.pos();

    auto srcResponse = fetch(element->attributes["in"]);
    if (srcResponse && srcResponse->statusCode == 200) {
        auto value = tel::fromJson(srcResponse->text);
        if (value->isArray()) {
            element->array = value->asArray();
        } else if (value->isRecord()) {
            auto record = value->asRecord();
            if (record->properties.count("items") != 0) {
                element->array = record->properties["items"]->asArray();
            }
        }
    }

    if (!element->array) {
        // A bit wasteful, but it makes the code cleaner.
        element->array = new tel::ArrayValue();
    }

    _ctx->repeatStack.push(element);

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

bool TemplateEvaluator::isInsideEmptyRepeat() {
    return _ctx->emptyRepeatDepth > 0;
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
