#pragma once

#include "common.hh"
#include "third-party/utfcpp/source/utf8/core.h"

namespace dtml {
namespace detail {
bool isValidHTMLCodePoint(utf8::utfchar32_t codePoint) {
    if (codePoint > 0x10FFFF) {
        return false;
    }
    if (codePoint >= 0xD800 && codePoint <= 0xDFFF) {
        return false;
    }
    return true;
}

void appendHTMLReplacementCodePoint(String *out) {
    out->append("\xEF\xBF\xBD");
}

bool decodeHTMLNumericEntity(const String &entity, String *out) {
    if (entity.length() < 2 || entity[0] != '#') {
        return false;
    }

    size_t idx = 1;
    auto radix = 10;
    if (idx < entity.length() && (entity[idx] == 'x' || entity[idx] == 'X')) {
        radix = 16;
        idx++;
    }

    if (idx == entity.length()) {
        return false;
    }

    utf8::utfchar32_t codePoint = 0;
    for (; idx < entity.length(); idx++) {
        auto c = entity[idx];
        int value = -1;
        if (c >= '0' && c <= '9') {
            value = c - '0';
        } else if (radix == 16 && c >= 'a' && c <= 'f') {
            value = c - 'a' + 10;
        } else if (radix == 16 && c >= 'A' && c <= 'F') {
            value = c - 'A' + 10;
        }

        if (value < 0 || value >= radix) {
            return false;
        }

        codePoint = codePoint * radix + static_cast<utf8::utfchar32_t>(value);
        if (codePoint > 0x10FFFF) {
            appendHTMLReplacementCodePoint(out);
            return true;
        }
    }

    if (!isValidHTMLCodePoint(codePoint)) {
        appendHTMLReplacementCodePoint(out);
        return true;
    }

    utf8::internal::append(codePoint, std::back_inserter(*out));
    return true;
}

bool decodeHTMLNamedEntity(const String &entity, String *out) {
    /* clang-format off */
    if (entity == "amp")  { out->push_back('&');  return true; }
    if (entity == "quot") { out->push_back('"');  return true; }
    if (entity == "apos") { out->push_back('\''); return true; }
    if (entity == "lt")   { out->push_back('<');  return true; }
    if (entity == "gt")   { out->push_back('>');  return true; }
    /* clang-format on */
    return false;
}
} // namespace detail

static String encodeHTML(const String &data) {
    String buffer;
    buffer.reserve(data.size() * 1.1);
    for (auto pos = 0; pos != data.size(); pos++) {
        /* clang-format off */
        switch (data[pos]) {
            // case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
        /* clang-format on */
    }
    return buffer;
}

static String decodeHTML(const String &data) {
    String buffer;
    buffer.reserve(data.size());

    for (size_t pos = 0; pos < data.size();) {
        if (data[pos] != '&') {
            buffer.push_back(data[pos]);
            pos++;
            continue;
        }

        auto semicolon = data.find(';', pos + 1);
        if (semicolon == String::npos) {
            buffer.push_back(data[pos]);
            pos++;
            continue;
        }

        auto entity = data.substr(pos + 1, semicolon - pos - 1);
        String decoded;
        auto didDecode = detail::decodeHTMLNamedEntity(entity, &decoded) ||
                         detail::decodeHTMLNumericEntity(entity, &decoded);
        if (didDecode) {
            buffer.append(decoded);
            pos = semicolon + 1;
        } else {
            buffer.append(data, pos, semicolon - pos + 1);
            pos = semicolon + 1;
        }
    }

    return buffer;
}
} // namespace dtml
