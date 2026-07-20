#ifndef TARGET_NATIVE
#error "This file should only be included when compiling to native."
#endif

#include "fetch.hh"

int doFetchGET(const char *url, uint8_t **out_ptr, int *out_len) {
    return 1;
}

int doFetchPOST(const char *url, const char *body, uint8_t **out_ptr, int *out_len) {
    return 1;
}