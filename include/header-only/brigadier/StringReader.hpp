#pragma once

#include <string>
#include <string_view>
#include <sstream>

namespace brigadier
{
    class StringReader
    {
    private:
        static constexpr char SYNTAX_ESCAPE       = '\\';
        static constexpr char SYNTAX_SINGLE_QUOTE = '\'';
        static constexpr char SYNTAX_DOUBLE_QUOTE = '"';

    public:
        StringReader(std::string_view string) : string(string) {}
        StringReader() {}

        inline std::string_view GetString()           const { return string; }
        inline void             SetCursor(int cursor)       { this->cursor = cursor; }
        inline int              GetRemainingLength()  const { return string.length() - cursor; }
        inline int              GetTotalLength()      const { return string.length(); }
        inline int              GetCursor()           const { return cursor; }
        inline std::string_view GetRead()             const { return string.substr(0, cursor); }
        inline std::string_view GetRemaining()        const { return string.substr(cursor); }
        inline bool             CanRead(int length)   const { return (size_t)(cursor + length) <= string.length(); }
        inline bool             CanRead()             const { return CanRead(1); }
        inline char             Peek()                const { return Peek(0); }
        inline char             Peek(int offset)      const { return string.at(cursor + offset); }
        inline char             Read()                      { return string.at(cursor++); }
        inline void             Skip()                      { cursor++; }

        inline static bool IsQuotedStringStart(char c)
        {
            return c == SYNTAX_DOUBLE_QUOTE || c == SYNTAX_SINGLE_QUOTE;
        }

        inline void SkipWhitespace()
        {
            while (CanRead() && std::isspace(Peek())) {
                Skip();
            }
        }

        template<typename T>
        inline T ReadValue();
        template<typename T>
        inline T ReadValueUntil(char terminator);
        template<typename T>
        inline T ReadValueUntilOneOf(const char* terminators);

        inline static bool IsAllowedInUnquotedString(char c)
        {
            return (c >= '0' && c <= '9')
                || (c >= 'A' && c <= 'Z')
                || (c >= 'a' && c <= 'z')
                || (c == '_' || c == '-')
                || (c == '.' || c == '+');
        }

        template<bool allow_float = true, bool allow_negative = true>
        inline static bool IsAllowedNumber(char c)
        {
            return c >= '0' && c <= '9' || (allow_float && c == '.') || (allow_negative && c == '-');
        }

        inline std::string_view ReadUnquotedString();
        inline std::string      ReadQuotedString();
        inline std::string      ReadStringUntil(char terminator);
        inline std::string      ReadStringUntilOneOf(const char* terminators);
        inline std::string      ReadString();
        inline void             Expect(char c);

    private:
        std::string_view string;
        int cursor = 0;
    };
}

#include "Exceptions/Exceptions.hpp"

namespace brigadier
{
    std::string_view StringReader::ReadUnquotedString()
    {
        int start = cursor;
        while (CanRead() && IsAllowedInUnquotedString(Peek())) {
            Skip();
        }
        return string.substr(start, cursor - start);
    }

    std::string StringReader::ReadQuotedString()
    {
        if (!CanRead()) {
            return {};
        }
        char next = Peek();
        if (!IsQuotedStringStart(next)) {
            throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedStartOfQuote(*this);
        }
        Skip();
        return ReadStringUntil(next);
    }

    std::string StringReader::ReadStringUntil(char terminator)
    {
        std::string result;
        result.reserve(GetRemainingLength());

        bool escaped = false;
        while (CanRead()) {
            char c = Read();
            if (escaped) {
                if (c == terminator || c == SYNTAX_ESCAPE) {
                    result += c;
                    escaped = false;
                }
                else {
                    SetCursor(GetCursor() - 1);
                    throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidEscape(*this, c);
                }
            }
            else if (c == SYNTAX_ESCAPE) {
                escaped = true;
            }
            else if (c == terminator) {
                return result;
            }
            else {
                result += c;
            }
        }

        throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedEndOfQuote(*this);
    }

    std::string StringReader::ReadStringUntilOneOf(const char* terminators)
    {
        std::string result;
        result.reserve(GetRemainingLength());

        bool escaped = false;
        while (CanRead()) {
            char c = Read();
            if (escaped) {
                if (c == SYNTAX_ESCAPE) {
                    result += c;
                    escaped = false;
                }
                if (escaped) {
                    for (const char* t = terminators; *t != 0; t++) {
                        if (c == *t) {
                            result += c;
                            escaped = false;
                            break;
                        }
                    }
                }
                if (escaped) {
                    SetCursor(GetCursor() - 1);
                    throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidEscape(*this, c);
                }
            }
            else if (c == SYNTAX_ESCAPE) {
                escaped = true;
            }
            else {
                for (const char* t = terminators; *t != 0; t++) {
                    if (c == *t) {
                        return result;
                    }
                }
                result += c;
            }
        }

        throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedOneOf(*this, terminators);
    }

    std::string StringReader::ReadString()
    {
        if (!CanRead()) {
            return {};
        }
        char next = Peek();
        if (IsQuotedStringStart(next)) {
            Skip();
            return ReadStringUntil(next);
        }
        return std::string(ReadUnquotedString());
    }

    void StringReader::Expect(char c)
    {
        if (!CanRead() || Peek() != c) {
            throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedSymbol(*this, c);
        }
        Skip();
    }

    template<typename T>
    T StringReader::ReadValue()
    {
        int start = cursor;
        std::string value;
        if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
        {
            while (CanRead() && IsAllowedNumber<std::is_floating_point_v<T>, std::is_signed_v<T>>(Peek())) {
                Skip();
            }
            value = string.substr(start, cursor - start);
        }
        else
        {
            value = ReadString();
        }

        if (value.empty()) {
            throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedValue(*this);
        }

        if constexpr (std::is_same_v<T, bool>)
        {
            /**/ if (value == "true")
                return true;
            else if (value == "false")
                return false;
            else
            {
                cursor = start;
                throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidValue(*this, value);
            }
        }
        else
        {
            T ret{};
            std::istringstream s(value);
            s >> ret;

            if (s.eof() && !s.bad() && !s.fail())
                return ret;
            else
            {
                cursor = start;
                throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidValue(*this, value);
            }
        }
    }
}
