#pragma once

#include "common.hh"
#include "fetch.hh"
#include "tel.hh"
#include "tpl-cursor.hh"
#include "tpl-printer.hh"

namespace dhtml {
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

class Template {
    String _source;

  public:
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
    int emptyRepeatDepth = 0;

    tel::EvaluationContext exprContext;

    explicit TemplateEvaluationContext(TemplateCache *cache, const char *routeParamsJSON)
        : cache(cache) {
        exprContext.addGlobal("route", tel::parseJson(routeParamsJSON));
    }
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

    inline const char *getText() const {
        return _text.c_str();
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

    bool evaluatePartialStart(PartialElement *element);
    bool evaluatePartialEnd(PartialElement *element);

    bool evaluateRepeatStart(RepeatElement *element);
    bool evaluateRepeatEnd(RepeatElement *element);

    bool isInsideEmptyRepeat();

    bool error(const String &reason);
    bool error(const String &reason, TemplateLocation location);

    FetchResponse *fetch(const String &url);

  public:
    explicit TemplateEvaluator(const Template *tpl)
        : _source{tpl->getSource()}, _c{_source}, _p{_source.length()} {};

    TemplateEvaluationResult *evaluate(TemplateEvaluationContext *ctx);
};
} // namespace dhtml
