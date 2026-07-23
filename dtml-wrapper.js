if (typeof createDTMLModule !== 'function') {
    importScripts('/lib/dtml/.build/emscripten/dtml.js')
}

async function createDTML() {
    const m = await createDTMLModule({
        locateFile(path) {
            return `/lib/dtml/.build/emscripten/${path}`
        }
    })

    const templateNew = m.cwrap('dtml_Template_New', 'number', ['string'])
    const templateEvaluationContextNew = m.cwrap('dtml_TemplateEvaluationContext_New', 'number', [
        'number',
        'string',
        'string'
    ])
    const templateEvaluatorEvaluate = m.cwrap(
        'dtml_TemplateEvaluator_Evaluate',
        'number',
        ['number', 'number', 'string'],
        {
            async: true
        }
    )
    const templateCacheAdd = m.cwrap('dtml_TemplateCache_Add', 'void', ['number', 'string', 'string'])

    const cachePtr = m._dtml_TemplateCache_New()

    return {
        addToTemplateCache(url, text) {
            templateCacheAdd(cachePtr, url, text)
        },
        clearTemplateCache() {
            m._dtml_TemplateCache_Clear(cachePtr)
        },

        async evaluateTemplate(text, routeParams, customProtocols) {
            let result = {}

            const templatePtr = templateNew(text)
            const evaluatorPtr = m._dtml_TemplateEvaluator_New(templatePtr)
            const contextPtr = templateEvaluationContextNew(
                cachePtr,
                JSON.stringify(routeParams),
                JSON.stringify(customProtocols)
            )
            const resultPtr = await templateEvaluatorEvaluate(evaluatorPtr, contextPtr)

            const isError = m._dtml_TemplateEvaluationResult_IsError(resultPtr)
            if (isError) {
                result = {
                    error: m.UTF8ToString(m._dtml_TemplateEvaluationResult_GetText(resultPtr)),
                    location: {
                        line: m._dtml_TemplateEvaluationResult_GetErrorLocationLine(resultPtr),
                        column: m._dtml_TemplateEvaluationResult_GetErrorLocationColumn(resultPtr)
                    }
                }
            } else {
                result = {
                    error: false,
                    text: m.UTF8ToString(m._dtml_TemplateEvaluationResult_GetText(resultPtr))
                }
            }

            m._dtml_TemplateEvaluationResult_Free(resultPtr)
            m._dtml_TemplateEvaluationContext_Free(contextPtr)
            m._dtml_TemplateEvaluator_Free(evaluatorPtr)
            m._dtml_Template_Free(templatePtr)

            return result
        }
    }
}
