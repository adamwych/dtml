#pragma once

#include "common.hh"

namespace dtml {
struct FetchResponse {
    int statusCode;
    String text;

    inline bool isOk() const {
        return statusCode >= 200 && statusCode <= 299;
    }
};
} // namespace dtml

extern "C" int doFetchGET(const char *url, uint8_t **out_ptr, int *out_len);
extern "C" int doFetchPOST(const char *url, const char *body, uint8_t **out_ptr, int *out_len);