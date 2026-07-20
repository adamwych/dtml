#include "common.hh"
#include "tpl.hh"

#ifdef TARGET_EMSCRIPTEN
#include <emscripten/emscripten.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE extern "C"
#else
#define EXPORT
#endif

namespace dhtml {

/* clang-format off */

// Template
EXPORT Template *DHTML_Template_New(const char *source) {
    return new Template(source);
}
EXPORT void DHTML_Template_Free(Template *self) {
    delete self;
}

// TemplateCache
EXPORT TemplateCache *DHTML_TemplateCache_New() {
    return new TemplateCache();
}
EXPORT void DHTML_TemplateCache_Free(TemplateCache *self) {
    delete self;
}
EXPORT void DHTML_TemplateCache_Clear(TemplateCache *self) {
    self->fetchCache.clear();
}

// TemplateEvaluationContext
EXPORT TemplateEvaluationContext *DHTML_TemplateEvaluationContext_New(TemplateCache *cache, const char *routeParamsJSON) {
    return new TemplateEvaluationContext(cache, routeParamsJSON);
}
EXPORT void DHTML_TemplateEvaluationContext_Free(TemplateEvaluationContext *self) {
    delete self;
}

// TemplateEvaluator
EXPORT TemplateEvaluator *DHTML_TemplateEvaluator_New(Template *tpl) {
    return new TemplateEvaluator(tpl);
}
EXPORT void DHTML_TemplateEvaluator_Free(TemplateEvaluator *self) {
    delete self;
}
EXPORT TemplateEvaluationResult *DHTML_TemplateEvaluator_Evaluate(TemplateEvaluator *self, TemplateEvaluationContext *ctx) {
    return self->evaluate(ctx);
}

// TemplateEvaluationResult
EXPORT void DHTML_TemplateEvaluationResult_Free(TemplateEvaluationResult *self) {
    delete self;
}
EXPORT const char *DHTML_TemplateEvaluationResult_GetText(TemplateEvaluationResult *self) {
    return self->getText();
}
EXPORT bool DHTML_TemplateEvaluationResult_IsError(TemplateEvaluationResult *self) {
    return self->isError();
}
EXPORT int DHTML_TemplateEvaluationResult_GetErrorLocationLine(TemplateEvaluationResult *self) {
    return self->getErrorLocation().line;
}
EXPORT int DHTML_TemplateEvaluationResult_GetErrorLocationColumn(TemplateEvaluationResult *self) {
    return self->getErrorLocation().column;
}

/* clang-format on */

} // namespace dhtml