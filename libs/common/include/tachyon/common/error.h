#pragma once

#include "source_span.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief kind of errors that can occur
 */
enum class ErrorKind
{
    LexError,
    ParseError,
    InferenceError,
    BytecodeGenerationError,
    // TODO: special note for internal error
    InternalError,
};

/**
 * @brief given a string and a position index, get its whole line
 */
inline std::string getLineAt(const std::string &s, size_t pos)
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
 * @brief an error
 *
 * Stores any kind of error that occurs during (this program's) runtime. The source is kept blank
 * until just before error reporting, where it is filled in by the caller before calling format.
 *
 * @see ErrorKind
 */
struct [[nodiscard]] Error final
{
    ErrorKind kind;
    SourceSpan span;
    std::string message_short;
    std::string message_long;
    std::string message_code;
    std::vector<std::string> hints;
    std::vector<Error> additional_errors; // TODO: not sure
    std::string_view source;

  private:
    /**
     * @brief error type. source is always kept blank.
     * @param kind error kind
     * @param span source span
     * @param message_short short message
     * @param message_long long message
     * @param message_code message code
     * @param hints list of hint
     * @param additional_errors list of additional errors
     * @see SourceSpan
     */
    Error(ErrorKind kind, SourceSpan span, std::string message_short, std::string message_long,
          std::string message_code, std::vector<std::string> hints,
          std::vector<Error> additional_errors)
        : kind(kind), span(span), message_short(std::move(message_short)),
          message_long(std::move(message_long)), message_code(std::move(message_code)),
          hints(std::move(hints)), additional_errors(std::move(additional_errors)), source()
    {
    }

    /**
     * @brief get name of error kind
     * @param kind kind
     * @return name
     */
    static std::string getNameOfErrorKind(const ErrorKind &kind)
    {
        switch (kind)
        {
        case ErrorKind::LexError:
            return "lexical";
        case ErrorKind::ParseError:
            return "parsing";
        case ErrorKind::InferenceError:
            return "type inference";
        case ErrorKind::BytecodeGenerationError:
            return "bytecode generation";
        case ErrorKind::InternalError:
            return "internal";
        default:
            return "unknown";
        }
    }

    /**
     * @brief 1-indexed line number
     *
     * Counts the number of newline characters are in the source until the next character after
     * position (ie including position itself), then adds 1 for the first line.
     *
     * @param position byte offset
     * @return line offset
     */
    size_t getLineNumber(size_t position) const
    {
        return std::count(source.begin(), source.begin() + position + 1, '\n') + 1;
    }

    /**
     * @brief number of digits of the position's line number
     * @param position byte offset
     * @return digits of line offset
     */
    size_t getLineNumberDigits(size_t position) const
    {
        size_t n = getLineNumber(position);
        if (n == 0)
            return 1;
        size_t digits = 0;
        while (n > 0)
        {
            ++digits;
            n /= 10;
        }
        return digits;
    }

  public:
    /**
     * @brief format an error to a given stream
     * @param os write stream
     */
    void format(std::ostream &os) const
    {
        // wind position to beginning and end of the lines.
        size_t start = span.position;
        size_t end = span.position + span.length;
        while (start > 0 && source[start] != '\n')
            --start;
        while (end < source.size() && source[end] != '\n')
            ++end;

        // print the message code and short form, then the line numbers | source.
        if (!additional_errors.empty())
            os << "\033[31m1.\033[0m ";
        os << "\033[31merror:\033[0m ";
        os << "encountered " << getNameOfErrorKind(kind) << " error on L" << getLineNumber(start)
           << "C" << span.position - start << ": " << std::endl;
        if (!message_code.empty())
            os << message_code << ": ";
        os << message_short << std::endl;
        for (size_t i = start; i < end; ++i)
        {
            char byte = source[i];
            if (i == 0)
            {
                os << "\n" << getLineNumber(i) << " | " << byte;
            }
            else if (byte == '\n')
            {
                os << "\n" << getLineNumber(i) << " | ";
            }
            else
            {
                os << byte;
            }
        }

        // we can only highlight values if there is just one line of source (how would we underline
        // something that has a line of source under it?).
        if (getLineNumber(start) + 1 == getLineNumber(end))
        {
            os << std::endl;
            // print spaces up to the line numbers + position, then red carets up to end of length.
            os << "   "; // " | "
            for (size_t i = 0; i < getLineNumberDigits(end); ++i)
                os << " ";
            for (size_t i = start + 1; i < span.position; ++i)
                os << " ";
            os << "\033[31m";
            for (size_t i = 0; i < span.length; ++i)
                os << "^";
            os << "\033[0m";
        }

        os << std::endl;

        // print the long message and any hints
        if (!message_long.empty())
            os << std::endl << message_long << std::endl;
        for (std::string hint : hints)
        {
            os << " |-> \033[34mhint:\033[0m " << hint << std::endl;
        }

        if (kind == ErrorKind::InternalError)
        {
            os << std::endl
               << "\033[31mTHIS IS AN INTERNAL ERROR AND IS A BUG IN TACHYON. PLEASE REPORT THIS AT"
                  " www.github.com/kostareg/tachyon/issues.\033[0m"
               << std::endl;
        }

        // print additional errors
        for (size_t i = 0; i < additional_errors.size(); ++i)
        {
            auto additional_error = additional_errors[i];
            additional_error.source = source;
            os << std::endl << "\033[31m" << i + 2 << ".\033[0m " << additional_error;
        }
    };

    static Error create(ErrorKind kind, SourceSpan span, std::string message_short)
    {
        return {kind, span, std::move(message_short), "", "", {}, {}};
    }

    static Error createMultiple(std::vector<Error> errors)
    {
        if (errors.empty())
            throw std::range_error("cannot initialize list of errors with no errors");

        // inherit the first error, pop it from the front.
        Error e = std::move(errors.at(0));
        std::vector<Error> es(errors.begin() + 1, errors.end());
        e.additional_errors = std::move(es);
        return e;
    }

    Error &withLongMessage(const std::string &message_long)
    {
        this->message_long = message_long;
        return *this;
    }

    Error &withCode(const std::string &message_code)
    {
        this->message_code = message_code;
        return *this;
    }

    Error &withHint(const std::string &hint)
    {
        this->hints.push_back(hint);
        return *this;
    }

    Error &withAdditional(const Error &additional_error)
    {
        this->additional_errors.push_back(additional_error);
        return *this;
    }

    /**
     * @brief format to stream
     *
     * This is declared here and as a friend so that the additional errors can be printed in the
     * format function with `os << additional_error`.
     *
     * @param os stream
     * @param s error
     * @return stream
     * @see Error::format
     */
    friend std::ostream &operator<<(std::ostream &os, const Error &s);
};

inline std::ostream &operator<<(std::ostream &os, const Error &s)
{
    s.format(os);
    return os;
}
