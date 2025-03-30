#include <algorithm>
#include <cmath>
#include <cstddef>
#include <format>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

enum class ErrorKind {
    LexError,
    ParseError,
};

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

struct SourceSpan {
    size_t line;
    size_t column;
    size_t length;

    SourceSpan(size_t line, size_t column, size_t length)
        : line(line), column(column), length(length) {}
};

struct Error {
    ErrorKind kind;
    std::string messageShort;
    std::string messageLong;
    std::string slice;
    SourceSpan span;
    std::string code;
    std::vector<std::string> hints;

    Error(ErrorKind kind, std::string message, std::string slice, SourceSpan span)
        : kind(kind), messageShort(message), messageLong(message), slice(slice), span(span) {}
    Error(ErrorKind kind, std::string message, std::string slice, size_t line, size_t column,
          size_t length)
        : kind(kind), messageShort(message), messageLong(message), slice(slice),
          span(line, column, length) {}
    Error(ErrorKind kind, std::string messageShort, std::string messageLong, std::string slice,
          SourceSpan span)
        : kind(kind), messageShort(messageShort), messageLong(messageLong), slice(slice),
          span(span) {}
    Error(ErrorKind kind, std::string messageShort, std::string messageLong, std::string slice,
          size_t line, size_t column, size_t length)
        : kind(kind), messageShort(messageShort), messageLong(messageLong), slice(slice),
          span(line, column, length) {}

    std::string getSource() {
        // the error line is stored in slice - however, we have to highlight
        // the relevant issue.
        auto slice = this->slice;
        auto lineDigits = span.line == 0 ? 1 : static_cast<size_t>(std::log10(span.line)) + 1;

        // colors
        slice.insert(span.column - 1 + span.length, "\033[00m");
        slice.insert(span.column - 1, "\033[31m");

        // arrow
        slice.append("\n ");
        for (int i = 0; i < lineDigits; ++i) {
            slice.append(" ");
        }
        slice.append(" | ");
        for (int i = 0; i < span.column - 1; ++i) {
            slice.append(" ");
        }
        slice.append("\033[31m");
        for (int i = 0; i < span.length; ++i) {
            slice.append("^");
        }
        slice.append(" " + messageShort);
        slice.append("\033[00m");
        slice.append("\n ");
        for (int i = 0; i < lineDigits; ++i) {
            slice.append(" ");
        }
        slice.append(" |\n");

        for (int i = 0; i < hints.size(); ++i) {
            // space it out
            slice.append(" ");
            for (int i = 0; i < lineDigits; ++i) {
                slice.append(" ");
            }
            slice.append(" +-> \033[1mhint\033[00m: " + hints[i]);
        }

        // line number formatting (arrow line is done)
        std::string numPlaceholder;
        for (int i = 0; i < lineDigits; ++i) {
            numPlaceholder.append(" ");
        }
        return std::format(" {} |\n {} | {}", numPlaceholder, span.line, slice);
    }

    Error with_code(std::string code) {
        this->code = code;
        return *this;
    }

    Error with_hint(std::string help) {
        this->hints.push_back(help);
        return *this;
    }
};

template <typename T> class Result {
    std::variant<T, Error> inner;

  public:
    Result(T ok) : inner(std::move(ok)) {};
    Result(Error e) : inner(e) {};

    bool is_ok() const { return std::holds_alternative<T>(inner); }
    bool is_err() const { return !is_ok(); }

    T unwrap() {
        if (is_err()) {
            auto e = std::get<Error>(inner);
            constexpr auto message = "\033[31merror{}\033[0m: {} on L{}C{}.\n"
                                     "{}";

            auto code = e.code.empty() ? "" : "(" + e.code + ")";

            std::cout << std::format(message, code, e.messageLong, e.span.line, e.span.column,
                                     e.getSource());
            exit(1);
        }
        return std::get<T>(inner);
    }
};
