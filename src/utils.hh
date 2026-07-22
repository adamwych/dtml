#pragma once

#include "common.hh"

#include <memory>
#include <stdexcept>
#include <string>

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

template <typename... Args> std::string format(const std::string &fmt, Args... args) {
    int size_s = std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1; // Extra space for '\0'
    if (size_s <= 0) {
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, fmt.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
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
