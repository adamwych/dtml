#include "common.hh"
#include "tpl.hh"

#ifdef TARGET_EMSCRIPTEN
#include <emscripten/emscripten.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE extern "C"
#else
#define EXPORT
#endif

namespace dtml {

/* clang-format off */

// Template
EXPORT Template *dtml_Template_New(const char *source) {
    return new Template(source);
}
EXPORT void dtml_Template_Free(Template *self) {
    delete self;
}

// TemplateCache
EXPORT TemplateCache *dtml_TemplateCache_New() {
    return new TemplateCache();
}
EXPORT void dtml_TemplateCache_Free(TemplateCache *self) {
    delete self;
}
EXPORT void dtml_TemplateCache_Add(TemplateCache *self, const char* key, const char *text) {
    self->fetchCache[key] = FetchResponse {
		.statusCode = 200,
		.text = text
	};
}
EXPORT void dtml_TemplateCache_Clear(TemplateCache *self) {
    self->fetchCache.clear();
}

// TemplateEvaluationContext
EXPORT TemplateEvaluationContext *dtml_TemplateEvaluationContext_New(
	TemplateCache *cache,
	const char *routeParamsJSON,
	const char *customProtocolsJSON
) {
    return new TemplateEvaluationContext(cache, routeParamsJSON, customProtocolsJSON);
}
EXPORT void dtml_TemplateEvaluationContext_Free(TemplateEvaluationContext *self) {
    delete self;
}

// TemplateEvaluator
EXPORT TemplateEvaluator *dtml_TemplateEvaluator_New(Template *tpl) {
    return new TemplateEvaluator(tpl);
}
EXPORT void dtml_TemplateEvaluator_Free(TemplateEvaluator *self) {
    delete self;
}
EXPORT TemplateEvaluationResult *dtml_TemplateEvaluator_Evaluate(TemplateEvaluator *self, TemplateEvaluationContext *ctx) {
    return self->evaluate(ctx);
}

// TemplateEvaluationResult
EXPORT void dtml_TemplateEvaluationResult_Free(TemplateEvaluationResult *self) {
    delete self;
}
EXPORT const char *dtml_TemplateEvaluationResult_GetText(TemplateEvaluationResult *self) {
    return self->getText().c_str();
}
EXPORT bool dtml_TemplateEvaluationResult_IsError(TemplateEvaluationResult *self) {
    return self->isError();
}
EXPORT int dtml_TemplateEvaluationResult_GetErrorLocationLine(TemplateEvaluationResult *self) {
    return self->getErrorLocation().line;
}
EXPORT int dtml_TemplateEvaluationResult_GetErrorLocationColumn(TemplateEvaluationResult *self) {
    return self->getErrorLocation().column;
}

/* clang-format on */

} // namespace dtml