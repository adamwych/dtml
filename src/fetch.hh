#pragma once

#include "common.hh"

namespace dhtml {
struct FetchResponse {
    int statusCode;
    String text;
};
} // namespace dhtml

extern "C" int doFetchGET(const char *url, uint8_t **out_ptr, int *out_len);
extern "C" int doFetchPOST(const char *url, const char *body, uint8_t **out_ptr, int *out_len);