#pragma once

#include "common.hh"

namespace dhtml {
struct TemplateLocation {
    int line;
    int column;
};

class TemplateCursor {
    StringView _source;
    int _pos;
    int _line;
    int _column;

  public:
    explicit TemplateCursor(StringView source) : _source(source), _pos(0), _line(0), _column(0) {
    }

    bool is(char ch);
    bool is(const StringView &needle);
    bool isAsciiLetter();
    bool isAsciiDigit();
    bool isAsciiAlphanumeric();
    bool isElementTagSpecialCharacter();
    bool isContentWhitespace();
    bool isWhitespace();
    bool isAtEnd();

    bool advance();
    bool advance(int amount);
    bool advanceUnicode(int amount);

    bool eat(const StringView &needle);
    bool eatWhitespace();

    inline unsigned char current() {
        return _source[_pos];
    }

    inline bool peek(char needle) {
        return peek() == needle;
    }

    inline unsigned char peek() {
        return _source[_pos + 1];
    }

    inline void seek(int pos) {
        _pos = pos;
    }

    inline int pos() {
        return _pos;
    }

    inline TemplateLocation getLocation() {
        return TemplateLocation{.line = _line, .column = _column};
    }
};
} // namespace dhtml