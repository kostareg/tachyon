module;

#include <cmath>
#include <format>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

export module error;

/**
 * @brief Kind of errors that can occur.
 */
export enum class ErrorKind {
    LexError,
    ParseError,
    InferenceError,
};

/**
 * @brief Given a string and a position index, get its whole line.
 */
export inline std::string get_line_at(const std::string &s, size_t pos) {
    size_t start = s.rfind('\n', pos);
    size_t end = s.find('\n', pos);
    if (start == std::string::npos)
        start = 0;
    else
        ++start;
    if (end == std::string::npos)
        end = s.size();
    return s.substr(start, end - start);
}

/**
 * @brief Represents a span of source code.
 *
 * TODO: consider moving to span.hpp.
 */
export struct SourceSpan {
    /// Indexed position, range [0, characters-1].
    size_t pos;
    /// Line number, range [1, lines].
    size_t line;
    /// Column number, range [1, columns].
    size_t column;
    /// Length of span, range [0, characters-pos].
    size_t length;
};

export struct Error {
    ErrorKind kind;
    std::string messageShort;
    std::string messageLong;
    std::string source; // string_view?
    SourceSpan span;
    std::string code;
    std::vector<std::string> hints;
    std::vector<Error> additional;

    Error(ErrorKind kind, std::string message, SourceSpan span)
        : kind(kind), messageShort(message), messageLong(message), span(span) {}
    Error(ErrorKind kind, std::string message, size_t pos, size_t line,
          size_t column, size_t length)
        : kind(kind), messageShort(message), messageLong(message),
          span(pos, line, column, length) {}
    Error(ErrorKind kind, std::string messageShort, std::string messageLong,
          SourceSpan span)
        : kind(kind), messageShort(messageShort), messageLong(messageLong),
          span(span) {}
    Error(ErrorKind kind, std::string messageShort, std::string messageLong,
          size_t pos, size_t line, size_t column, size_t length)
        : kind(kind), messageShort(messageShort), messageLong(messageLong),
          span(pos, line, column, length) {}
    explicit Error(const std::vector<Error> &additionals) {
        if (additionals.empty()) {
            throw std::runtime_error("ice: 0 errors given");
        } else {
            // inherit first element
            auto e = additionals[0];
            this->kind = e.kind;
            this->messageShort = e.messageShort;
            this->messageLong = e.messageLong;
            this->source = e.source;
            this->span = e.span;
            this->code = e.code;
            this->hints = e.hints;
            this->additional = e.additional;

            // add the rest
            for (size_t i = 1; i < additionals.size(); ++i) {
                this->add_additional(additionals[i]);
            }
        }
    }

    std::string getSource() {
        // the error line is stored in source - however, we have to highlight
        // the relevant issue.
        auto source = this->source;
        auto lineDigits =
            span.line == 0 ? 1 : static_cast<size_t>(std::log10(span.line)) + 1;

        // colors
        // source.insert(span.column - 1 + span.length, "\033[00m");
        // source.insert(span.column - 1, "\033[31m");

        // arrow
        source.append("\n ");
        for (size_t i = 0; i < lineDigits; ++i) {
            source.append(" ");
        }
        source.append(" | ");
        for (size_t i = 0; i < span.column; ++i) {
            source.append(" ");
        }
        source.append("\033[31m");
        for (size_t i = 0; i < span.length; ++i) {
            source.append("^");
        }
        source.append(" " + messageShort);
        source.append("\033[00m");
        source.append("\n ");
        for (size_t i = 0; i < lineDigits; ++i) {
            source.append(" ");
        }
        source.append(" |\n");

        for (const auto &hint : hints) {
            // space it out
            source.append(" ");
            for (size_t i = 0; i < lineDigits; ++i) {
                source.append(" ");
            }
            source.append(" +-> \033[1mhint\033[00m: " + hint + "\n");
        }

        // line number formatting (arrow line is done)
        std::string numPlaceholder;
        for (size_t i = 0; i < lineDigits; ++i) {
            numPlaceholder.append(" ");
        }
        return std::format(" {} |\n {} | {}", numPlaceholder, span.line,
                           source);
    }

    Error with_code(const std::string &code) {
        this->code = code;
        return *this;
    }

    Error with_hint(const std::string &hint) {
        this->hints.push_back(hint);
        return *this;
    }

    void add_additional(const Error &error) {
        this->additional.push_back(error);
    }

    Error with_additional(const Error &error) {
        this->add_additional(error);
        return *this;
    }
};
