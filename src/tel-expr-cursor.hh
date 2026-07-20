#pragma once

#include "common.hh"

namespace tel {
class ExpressionCursor {
    const StringView &_source;
    int _pos;

  public:
    explicit ExpressionCursor(const StringView &source) : _source(source) {
        _pos = 0;
    }

    bool is(char ch) {
        return current() == ch;
    }
    bool isAsciiLetter() {
        auto c = current();
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
    bool isAsciiDigit() {
        return current() >= 48 && current() <= 57;
    }
    bool isAsciiAlphanumeric() {
        return isAsciiDigit() || isAsciiLetter();
    }
    bool isWhitespace() {
        return current() == ' ' || current() == '\t' || current() == '\n' || current() == '\r';
    }
    bool isAtEnd() {
        return _pos >= _source.length();
    }

    bool eat(char ch) {
        if (is(ch)) {
            advance();
            return true;
        }
        return false;
    }

    bool advance() {
        _pos++;
        return _pos < _source.length();
    }

    inline unsigned char current() {
        return _source[_pos];
    }

    inline unsigned char peek() {
        return _source[_pos + 1];
    }

    inline int pos() {
        return _pos;
    }
};
} // namespace tel