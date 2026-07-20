#pragma once

#include "common.hh"

namespace dtml {
class TemplatePrinter {
    String _output;
    int _disabledDepth = 0;

  public:
    explicit TemplatePrinter(size_t reservedLength) {
        _output.reserve(reservedLength);
    }

    void enablePrinting();
    void disablePrinting();

    void raw(char ch);
    void raw(const StringView &str);

    void printElementSignature(const StringView &name, const Map<StringView, String> &attributes,
                               bool isSelfClosing);
    void printElementAttributes(const Map<StringView, String> &attributes);
    void printElementClose(const StringView &name);

    const String &getOutput() const {
        return _output;
    }
};
} // namespace dtml
