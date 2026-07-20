#include "tpl-cursor.hh"

namespace {
static int utf8CharLength(unsigned char ch) {
    if ((ch & 0x80) == 0)
        return 1;
    if ((ch & 0xE0) == 0xC0)
        return 2;
    if ((ch & 0xF0) == 0xE0)
        return 3;
    if ((ch & 0xF8) == 0xF0)
        return 4;
    return 1; // invalid UTF-8 fallback
}
} // namespace

namespace dhtml {
bool TemplateCursor::advance() {
    _pos++;

    if (is('\n')) {
        _line = 0;
        _column = 0;
    } else if (is('\t')) {
        _column += 4;
    } else {
        _column++;
    }

    return _pos < _source.length();
}

bool TemplateCursor::advance(int amount) {
    for (int i = 0; i < amount; i++) {
        if (!advance()) {
            return false;
        }
    }
    return true;
}

bool TemplateCursor::advanceUnicode(int amount) {
    for (int i = 0; i < amount; i++) {
        if (!advance(utf8CharLength(current()))) {
            return false;
        }
    }
    return true;
}

bool TemplateCursor::eat(const StringView &needle) {
    if (is(needle)) {
        advance(needle.length());
        return true;
    }
    return false;
}

bool TemplateCursor::eatWhitespace() {
    while (peek(' ') || peek('\t') || peek('\n') || peek('\r')) {
        advance();
    }
    return true;
}

bool TemplateCursor::is(char ch) {
    return current() == ch;
}

bool TemplateCursor::is(const StringView &needle) {
    if (needle.length() == 1) {
        return current() == needle[0];
    }

    auto outOfBounds = _pos + needle.length() > _source.length();
    if (outOfBounds) {
        return false;
    }

    auto slice = _source.substr(_pos, needle.length());
    return slice == needle;
}

bool TemplateCursor::isAsciiLetter() {
    auto c = current();
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool TemplateCursor::isAsciiDigit() {
    return current() >= 48 && current() <= 57;
}

bool TemplateCursor::isAsciiAlphanumeric() {
    return isAsciiDigit() || isAsciiLetter();
}

bool TemplateCursor::isElementTagSpecialCharacter() {
    return current() == '_' || current() == ':' || current() == '-';
}

bool TemplateCursor::isContentWhitespace() {
    return current() == '\n' || current() == '\r';
}

bool TemplateCursor::isWhitespace() {
    return current() == ' ' || current() == '\t' || current() == '\n' || current() == '\r';
}

bool TemplateCursor::isAtEnd() {
    return _pos >= _source.length();
}
} // namespace dhtml
