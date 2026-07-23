#include "tpl-printer.hh"

namespace dtml {
static Vector<String> voidTags = {"area",  "base", "br",   "col",   "embed",  "hr",    "img",
                                  "input", "link", "meta", "param", "source", "track", "wbr"};

void TemplatePrinter::enablePrinting() {
    if (_disabledDepth > 0) {
        _disabledDepth--;
    }
}

void TemplatePrinter::disablePrinting() {
    _disabledDepth++;
}

void TemplatePrinter::raw(char ch) {
    if (_disabledDepth > 0) {
        return;
    }

    _output += ch;
}

void TemplatePrinter::raw(const StringView &str) {
    if (_disabledDepth > 0) {
        return;
    }

    _output += str;
}

void TemplatePrinter::printElementSignature(const StringView &name,
                                            const Map<StringView, String> &attributes,
                                            bool isSelfClosing) {
    if (_disabledDepth > 0) {
        return;
    }

    raw('<');
    raw(name);
    printElementAttributes(attributes);

    if (isSelfClosing) {
        auto isVoidTag = std::find(voidTags.begin(), voidTags.end(), name) != voidTags.end();
        if (isVoidTag) {
            raw("/>");
        } else {
            raw("></");
            raw(name);
            raw(">");
        }
    } else {
        raw(">");
    }
}

void TemplatePrinter::printElementAttributes(const Map<StringView, String> &attributes) {
    if (_disabledDepth > 0) {
        return;
    }

    for (auto [attributeName, attributeValue] : attributes) {
        if (attributeValue == "false") {
            continue;
        }

        raw(' ');
        raw(attributeName);
        raw('=');
        raw('"');
        raw(attributeValue);
        raw('"');
    }
}

void TemplatePrinter::printElementClose(const StringView &name) {
    if (_disabledDepth > 0) {
        return;
    }

    raw("</");
    raw(name);
    raw(">");
}
} // namespace dtml
