#pragma once

#include "common.hh"
#include "fetch.hh"
#include "tel.hh"
#include "tpl-cursor.hh"
#include "tpl-printer.hh"

namespace dtml {
struct Element {
    virtual ~Element() = default;

    String name;
    Map<StringView, String> attributes;
    Element *parent = nullptr;
    Vector<Element *> children;
};

struct PartialElement : public Element {
    tel::RecordValue *attributesRecord;
};

struct RepeatElement : public Element {
    tel::ArrayValue *array;
    int arrayItemIndex = 0;
    int childrenStartPos;
};

struct IfElement : public Element {
    bool conditionIsTrue;
};

class Template {
    String _source;

  public:
    explicit Template(const String &source) {
        _source = String(source);
    }
    explicit Template(const char *source) {
        _source = String(source);
    }
    const StringView getSource() const {
        return _source;
    }
};

struct TemplateCache {
    Map<String, FetchResponse> fetchCache;
};

struct TemplateEvaluationContext {
    TemplateCache *cache;
    Stack<Element *> elementStack;
    Stack<PartialElement *> partialStack;
    Stack<RepeatElement *> repeatStack;
    Stack<IfElement *> ifStack;
    int emptyRepeatDepth = 0;
    int falseIfDepth = 0;

    Map<String, String> customProtocols;

    tel::EvaluationContext exprContext;

    /* clang-format off */
    explicit TemplateEvaluationContext(
		TemplateCache *cache,
		const char *routeParamsJSON,
        const char *customProtocolsJSON
	) : cache(cache)
	{
        exprContext.addGlobal("$route", tel::fromJson(routeParamsJSON));

		auto customProtocolsRecord = tel::fromJson(customProtocolsJSON)->asRecord();
		for (auto [protocol, replacement] : customProtocolsRecord->value) {
			customProtocols[protocol + "://"] = replacement->asString()->value;
		}
    }
    /* clang-format on */
};

class TemplateEvaluationResult {
    bool _isError;
    TemplateLocation _errorLocation;
    String _text;

  public:
    explicit TemplateEvaluationResult(String evaluatedSource) {
        _isError = false;
        _text = evaluatedSource;
    }

    explicit TemplateEvaluationResult(String errorMessage, TemplateLocation errorLocation) {
        _isError = true;
        _errorLocation = errorLocation;
        _text = errorMessage;
    }

    inline const String &getText() const {
        return _text;
    }
    inline bool isError() const {
        return _isError;
    }
    inline TemplateLocation getErrorLocation() const {
        return _errorLocation;
    }
};

class TemplateEvaluator {
    StringView _source;
    TemplateEvaluationResult *_error;
    TemplateEvaluationContext *_ctx;

    TemplateCursor _c;
    TemplatePrinter _p;

    bool readString(String *out);
    Option<StringView> readElementName();
    Option<Map<StringView, String>> readElementAttributes();

    tel::EvaluationContext createExpressionEvaluationContext();

    bool evaluateTextExpression();

    bool evaluateElementAttributes(Map<StringView, String> *attributes);
    bool evaluateElementStart();
    bool evaluateElementEnd();

    bool evaluatePartial(PartialElement *element);

    bool evaluateRepeatStart(RepeatElement *element);
    bool evaluateRepeatEnd(RepeatElement *element);

    bool evaluateIfStart(IfElement *element);
    bool evaluateIfEnd(IfElement *element);

    bool isInsideEmptyRepeat();
    bool isInsideFalseIf();
    bool isInsideSkippedBlock();

    bool error(const String &reason);
    bool error(const String &reason, TemplateLocation location);

    FetchResponse *fetch(const String &url);

  public:
    /* clang-format off */
    explicit TemplateEvaluator(const Template *tpl) :
		_source{tpl->getSource()},
		_c{_source},
		_p{_source.length()}
	{};
    /* clang-format on */

    TemplateEvaluationResult *evaluate(TemplateEvaluationContext *ctx);
};
} // namespace dtml
