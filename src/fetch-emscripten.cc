#ifndef TARGET_EMSCRIPTEN
#error "This file should only be included when compiling using Emscripten."
#endif

#include "fetch.hh"
#include <emscripten.h>

EM_ASYNC_JS(int, doFetchGET, (const char *url, uint8_t **out_ptr, int *out_len), {
    try {
        const response = await fetch(UTF8ToString(url), {method : 'GET', credentials : 'include'});
        if (!response.ok) {
            return -response.status; // e.g. -404, -500
        }

        const buf = new Uint8Array(await response.arrayBuffer());
        const p = _malloc(buf.length);
        HEAPU8.set(buf, p);
        setValue(out_ptr, p, 'i32');
        setValue(out_len, buf.length, 'i32');
        return 0;
    } catch (e) {
        out("fetch failed:", e);
        return 1;
    }
});

EM_ASYNC_JS(
    int, doFetchPOST, (const char *url, const char *body, uint8_t **out_ptr, int *out_len), {
        try {
            const response = await fetch(UTF8ToString(url), {
                method : 'POST',
                body : JSON.stringify(Object.fromEntries(new URLSearchParams(UTF8ToString(body)))),
                credentials : 'include',
                headers : {'content-type' : 'application/json'}
            });
            if (!response.ok) {
                return -response.status; // e.g. -404, -500
            }

            const buf = new Uint8Array(await response.arrayBuffer());
            const p = _malloc(buf.length);
            HEAPU8.set(buf, p);
            setValue(out_ptr, p, 'i32');
            setValue(out_len, buf.length, 'i32');
            return 0;
        } catch (e) {
            out("fetch failed:", e);
            return 1;
        }
    });