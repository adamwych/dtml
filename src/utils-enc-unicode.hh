#pragma once

#include "common.hh"
#include "third-party/utfcpp/source/utf8/core.h"

#include <iterator>

namespace dtml {
namespace detail {
int hexDigitValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

bool parseUnicodeEscape(const String &value, size_t escapeStart, utf8::utfchar32_t *codePoint) {
    if (escapeStart > value.length() || value.length() - escapeStart < 6 ||
        value[escapeStart] != '\\' || value[escapeStart + 1] != 'u') {
        return false;
    }

    utf8::utfchar32_t parsed = 0;
    for (size_t idx = escapeStart + 2; idx < escapeStart + 6; idx++) {
        auto digit = hexDigitValue(value[idx]);
        if (digit < 0) {
            return false;
        }
        parsed = (parsed << 4) | static_cast<utf8::utfchar32_t>(digit);
    }

    *codePoint = parsed;
    return true;
}

void appendReplacementCodePoint(String *out) {
    out->append("\xEF\xBF\xBD");
}

bool isLeadSurrogate(utf8::utfchar32_t codePoint) {
    return codePoint >= 0xD800 && codePoint <= 0xDBFF;
}

bool isTrailSurrogate(utf8::utfchar32_t codePoint) {
    return codePoint >= 0xDC00 && codePoint <= 0xDFFF;
}

bool isUnsafeControlCodePoint(utf8::utfchar32_t codePoint) {
    if (codePoint == '\n' || codePoint == '\r' || codePoint == '\t') {
        return false;
    }
    return codePoint < 0x20 || (codePoint >= 0x7F && codePoint <= 0x9F);
}

void appendUnicodeCodePoint(String *out, utf8::utfchar32_t codePoint) {
    if (isUnsafeControlCodePoint(codePoint)) {
        appendReplacementCodePoint(out);
        return;
    }

    utf8::internal::append(codePoint, std::back_inserter(*out));
}

void skipInvalidUtf8Sequence(String::const_iterator *it, String::const_iterator end,
                             utf8::internal::utf_error errCode) {
    switch (errCode) {
    case utf8::internal::NOT_ENOUGH_ROOM:
        *it = end;
        break;
    case utf8::internal::INVALID_LEAD:
        ++(*it);
        break;
    case utf8::internal::INCOMPLETE_SEQUENCE:
    case utf8::internal::OVERLONG_SEQUENCE:
    case utf8::internal::INVALID_CODE_POINT:
        ++(*it);
        while (*it != end && utf8::internal::is_trail(**it)) {
            ++(*it);
        }
        break;
    case utf8::internal::UTF8_OK:
        break;
    }
}

String sanitizeUtf8Text(const String &value) {
    String out;
    out.reserve(value.length());

    auto it = value.begin();
    while (it != value.end()) {
        auto sequenceStart = it;
        utf8::utfchar32_t codePoint = 0;
        auto errCode = utf8::internal::validate_next(it, value.end(), codePoint);
        if (errCode == utf8::internal::UTF8_OK) {
            appendUnicodeCodePoint(&out, codePoint);
        } else {
            appendReplacementCodePoint(&out);
            it = sequenceStart;
            skipInvalidUtf8Sequence(&it, value.end(), errCode);
        }
    }

    return out;
}

void decodeUnicodeEscape(const String &value, size_t escapeStart, String *out,
                         size_t *escapeLength) {
    utf8::utfchar32_t codePoint = 0;
    if (!parseUnicodeEscape(value, escapeStart, &codePoint)) {
        appendReplacementCodePoint(out);
        *escapeLength = value.length() - escapeStart < 6 ? value.length() - escapeStart : 6;
        return;
    }

    *escapeLength = 6;

    if (isLeadSurrogate(codePoint)) {
        utf8::utfchar32_t trailSurrogate = 0;
        if (parseUnicodeEscape(value, escapeStart + 6, &trailSurrogate) &&
            isTrailSurrogate(trailSurrogate)) {
            codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (trailSurrogate - 0xDC00);
            *escapeLength = 12;
        } else {
            appendReplacementCodePoint(out);
            return;
        }
    } else if (isTrailSurrogate(codePoint)) {
        appendReplacementCodePoint(out);
        return;
    }

    appendUnicodeCodePoint(out, codePoint);
}
} // namespace detail

static String decodeJsonStringEscapes(const String &value) {
    String out;
    out.reserve(value.length());

    for (size_t idx = 0; idx < value.length();) {
        if (value[idx] != '\\' || idx + 1 >= value.length()) {
            out.push_back(value[idx]);
            idx++;
            continue;
        }

        auto escaped = value[idx + 1];
        switch (escaped) {
        case '"':
        case '\\':
        case '/':
            out.push_back(escaped);
            idx += 2;
            break;
        case 'b':
            out.push_back('\b');
            idx += 2;
            break;
        case 'f':
            out.push_back('\f');
            idx += 2;
            break;
        case 'n':
            out.push_back('\n');
            idx += 2;
            break;
        case 'r':
            out.push_back('\r');
            idx += 2;
            break;
        case 't':
            out.push_back('\t');
            idx += 2;
            break;
        case 'u': {
            size_t escapeLength = 0;
            detail::decodeUnicodeEscape(value, idx, &out, &escapeLength);
            idx += escapeLength;
            break;
        }
        default:
            out.push_back('\\');
            out.push_back(escaped);
            idx += 2;
            break;
        }
    }

    return detail::sanitizeUtf8Text(out);
}
} // namespace dtml
