module;

#include <cmath>
#include <format>
#include <iostream>
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
    BytecodeGenerationError,
    InternalError, // TODO: special note for internal error
};

/**
 * @brief Given a string and a position index, get its whole line.
 */
export inline std::string get_line_at(const std::string &s, size_t pos)
{
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
export struct SourceSpan
{
    /// Indexed position, range [0, characters-1].
    size_t pos;
    /// Line number, range [1, lines].
    size_t line;
    /// Column number, range [1, columns].
    size_t column;
    /// Length of span, range [0, characters-pos].
    size_t length;
};

export struct Error
{
    std::string message_short;
    std::string message_long;
    std::string error_code;
    ErrorKind kind;
    SourceSpan span;
    std::vector<std::string> hints;
    std::vector<Error> additional;
    std::string code;
    std::string source;

    Error(ErrorKind kind, std::string message, SourceSpan span)
        : kind(kind), message_short(message), message_long(message), span(span)
    {
    }
    Error(ErrorKind kind, std::string message, size_t pos, size_t line, size_t column,
          size_t length)
        : kind(kind), message_short(message), message_long(message), span(pos, line, column, length)
    {
    }
    Error(ErrorKind kind, std::string messageShort, std::string messageLong, SourceSpan span)
        : kind(kind), message_short(messageShort), message_long(messageLong), span(span)
    {
    }
    Error(ErrorKind kind, std::string messageShort, std::string messageLong, size_t pos,
          size_t line, size_t column, size_t length)
        : kind(kind), message_short(messageShort), message_long(messageLong),
          span(pos, line, column, length)
    {
    }
    explicit Error(const std::vector<Error> &additionals)
    {
        if (additionals.empty())
        {
            throw std::runtime_error("ice: 0 errors given");
        }
        else
        {
            // inherit first element
            auto e = additionals[0];
            this->kind = e.kind;
            this->message_short = e.message_short;
            this->message_long = e.message_long;
            this->source = e.source;
            this->span = e.span;
            this->code = e.code;
            this->hints = e.hints;
            this->additional = e.additional;

            // add the rest
            for (size_t i = 1; i < additionals.size(); ++i)
            {
                this->addAdditional(additionals[i]);
            }
        }
    }

    std::string getSource()
    {
        // the error line is stored in source - however, we have to highlight
        // the relevant issue.
        auto source = this->source;
        auto line_digits = span.line == 0 ? 1 : static_cast<size_t>(std::log10(span.line)) + 1;

        // colors
        // source.insert(span.column - 1 + span.length, "\033[00m");
        // source.insert(span.column - 1, "\033[31m");

        // arrow
        source.append("\n ");
        for (size_t i = 0; i < line_digits; ++i)
        {
            source.append(" ");
        }
        source.append(" | ");
        for (size_t i = 0; i < span.column; ++i)
        {
            source.append(" ");
        }
        source.append("\033[31m");
        for (size_t i = 0; i < span.length; ++i)
        {
            source.append("^");
        }
        source.append(" " + message_short);
        source.append("\033[00m");
        source.append("\n ");
        for (size_t i = 0; i < line_digits; ++i)
        {
            source.append(" ");
        }
        source.append(" |\n");

        for (const auto &hint : hints)
        {
            // space it out
            source.append(" ");
            for (size_t i = 0; i < line_digits; ++i)
            {
                source.append(" ");
            }
            source.append(" +-> \033[1mhint\033[00m: " + hint + "\n");
        }

        // line number formatting (arrow line is done)
        std::string num_placeholder;
        for (size_t i = 0; i < line_digits; ++i)
        {
            num_placeholder.append(" ");
        }
        return std::format(" {} |\n {} | {}", num_placeholder, span.line, source);
    }

    Error withCode(const std::string &code)
    {
        this->code = code;
        return *this;
    }

    Error withHint(const std::string &hint)
    {
        this->hints.push_back(hint);
        return *this;
    }

    void addAdditional(const Error &error) { this->additional.push_back(error); }

    Error withAdditional(const Error &error)
    {
        this->addAdditional(error);
        return *this;
    }
};
