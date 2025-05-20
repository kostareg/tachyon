#pragma once

#include <cmath>
#include <format>
#include <string>
#include <vector>

/**
 * @brief Kind of errors that can occur.
 */
enum class ErrorKind {
    LexError,
    ParseError,
};

/**
 * @brief Given a string and a position index, get its whole line.
 */
inline std::string get_line_at(const std::string &s, size_t pos) {
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
struct SourceSpan {
    /// Indexed position, range [0, characters-1].
    size_t pos;
    /// Line number, range [1, lines].
    size_t line;
    /// Column number, range [1, columns].
    size_t column;
    /// Length of span, range [0, characters-pos].
    size_t length;
};

struct Error {
    ErrorKind kind;
    std::string messageShort;
    std::string messageLong;
    std::string source; // string_view?
    SourceSpan span;
    std::string code;
    std::vector<std::string> hints;

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

    std::string getSource() {
        // the error line is stored in source - however, we have to highlight
        // the relevant issue.
        auto source = this->source;
        auto lineDigits =
            span.line == 0 ? 1 : static_cast<size_t>(std::log10(span.line)) + 1;

        // colors
        source.insert(span.column - 1 + span.length, "\033[00m");
        source.insert(span.column - 1, "\033[31m");

        // arrow
        source.append("\n ");
        for (size_t i = 0; i < lineDigits; ++i) {
            source.append(" ");
        }
        source.append(" | ");
        for (size_t i = 0; i < span.column - 1; ++i) {
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

        for (size_t i = 0; i < hints.size(); ++i) {
            // space it out
            source.append(" ");
            for (size_t i = 0; i < lineDigits; ++i) {
                source.append(" ");
            }
            source.append(" +-> \033[1mhint\033[00m: " + hints[i] + "\n");
        }

        // line number formatting (arrow line is done)
        std::string numPlaceholder;
        for (size_t i = 0; i < lineDigits; ++i) {
            numPlaceholder.append(" ");
        }
        return std::format(" {} |\n {} | {}", numPlaceholder, span.line,
                           source);
    }

    Error with_code(std::string code) {
        this->code = code;
        return *this;
    }

    Error with_hint(std::string hint) {
        this->hints.push_back(hint);
        return *this;
    }
};
