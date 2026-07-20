#pragma once

#include "common.hh"

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