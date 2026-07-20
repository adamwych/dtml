if (typeof createDHTMLModule !== 'function') {
    importScripts('@dhtml/.build/emscripten/dhtml.js')
}

async function createDHTML() {
    const m = await createDHTMLModule({
        locateFile(path) {
            return `@dhtml/.build/emscripten/${path}`
        }
    })

    const templateNew = m.cwrap('DHTML_Template_New', 'number', ['string'])
    const templateEvaluationContextNew = m.cwrap('DHTML_TemplateEvaluationContext_New', 'number', ['number', 'string'])
    const templateEvaluatorEvaluate = m.cwrap(
        'DHTML_TemplateEvaluator_Evaluate',
        'number',
        ['number', 'number', 'string'],
        {
            async: true
        }
    )

    const cachePtr = m._DHTML_TemplateCache_New()

    return {
        clearTemplateCache() {
            m._DHTML_TemplateCache_Clear(cachePtr)
        },

        async evaluateTemplate(text, routeParams) {
            let result = {}

            const templatePtr = templateNew(text)
            const evaluatorPtr = m._DHTML_TemplateEvaluator_New(templatePtr)
            const contextPtr = templateEvaluationContextNew(cachePtr, JSON.stringify(routeParams))
            const resultPtr = await templateEvaluatorEvaluate(evaluatorPtr, contextPtr)

            const isError = m._DHTML_TemplateEvaluationResult_IsError(resultPtr)
            if (isError) {
                result = {
                    error: m.UTF8ToString(m._DHTML_TemplateEvaluationResult_GetText(resultPtr)),
                    location: {
                        line: m._DHTML_TemplateEvaluationResult_GetErrorLocationLine(resultPtr),
                        column: m._DHTML_TemplateEvaluationResult_GetErrorLocationColumn(resultPtr)
                    }
                }
            } else {
                result = {
                    error: false,
                    text: m.UTF8ToString(m._DHTML_TemplateEvaluationResult_GetText(resultPtr))
                }
            }

            m._DHTML_TemplateEvaluationResult_Free(resultPtr)
            m._DHTML_TemplateEvaluationContext_Free(contextPtr)
            m._DHTML_TemplateEvaluator_Free(evaluatorPtr)
            m._DHTML_Template_Free(templatePtr)

            return result
        }
    }
}
