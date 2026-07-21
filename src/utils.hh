#pragma once

#include "common.hh"

static bool stringStartsWith(const String &str, const String &with) {
    if (str.length() < with.length()) {
        return false;
    }

    auto slice = str.substr(0, with.length());
    return slice == with;
}

static bool stringEndsWith(const String &str, const String &with) {
    long pos = str.length() - with.length();
    if (pos < 0) {
        return false;
    }

    auto slice = str.substr(pos);
    return slice == with;
}

static String replaceCustomProtocol(const String &src, const Map<String, String> &replacements) {
    String url;
    for (auto [protocol, replacement] : replacements) {
        if (stringStartsWith(src, protocol)) {
            url.append(replacement);
            url.append("/");
            url.append(src.substr(protocol.length()));
            return url;
        }
    }
    return src;
}
