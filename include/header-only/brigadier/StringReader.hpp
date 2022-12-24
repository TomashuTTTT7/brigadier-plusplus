#pragma once

// see StringReader.hxx for class body

#include "Exceptions/Exceptions.hpp"

namespace brigadier
{
    template<typename CharT>
    std::basic_string_view<CharT> StringReader<CharT>::ReadUnquotedString()
    {
        size_t start = cursor;
        while (CanRead() && IsAllowedInUnquotedString(Peek())) {
            Skip();
        }
        return string.substr(start, cursor - start);
    }

    template<typename CharT>
    std::basic_string_view<CharT> StringReader<CharT>::ReadUnquotedStringUntil(CharT terminator)
    {
        size_t start = cursor;
        while (CanRead()) {
            CharT c = Peek();
            if (!IsAllowedInUnquotedString(c))
                break;
            if (c == terminator)
                return string.substr(start, (cursor++) - start);
            Skip();
        }
        return string.substr(start, cursor - start);
    }

    template<typename CharT>
    std::basic_string_view<CharT> StringReader<CharT>::ReadUnquotedStringUntilOneOf(CharT const* terminators)
    {
        size_t start = cursor;
        while (CanRead()) {
            CharT c = Peek();
            if (!IsAllowedInUnquotedString(c))
                break;
            for (CharT const* t = terminators; *t != 0; t++) {
                if (c == *t) {
                    return string.substr(start, (cursor++) - start);
                }
            }
            Skip();
        }
        return string.substr(start, cursor - start);
    }

    template<typename CharT>
    std::basic_string<CharT> StringReader<CharT>::ReadString()
    {
        if (!CanRead()) {
            return {};
        }
        CharT next = Peek();
        if (IsQuotedStringStart(next)) {
            Skip();
            return ReadStringUntil(next);
        }
        return std::basic_string<CharT>(ReadUnquotedString());
    }

    template<typename CharT>
    std::basic_string<CharT> StringReader<CharT>::ReadStringUntil(CharT terminator)
    {
        std::basic_string<CharT> result;
        result.reserve(GetRemainingLength());

        bool escaped = false;
        while (CanRead()) {
            CharT c = Read();
            if (escaped) {
                if (c == terminator || c == SYNTAX_ESCAPE) {
                    result += c;
                    escaped = false;
                }
                else {
                    SetCursor(GetCursor() - 1);
                    throw exceptions::ReaderInvalidEscape(*this, c);
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

        if (IsQuotedStringStart(terminator)) {
            throw exceptions::ReaderExpectedEndOfQuote(*this);
        }
        else {
            throw exceptions::ReaderExpectedSymbol(*this, terminator);
        }
    }

    template<typename CharT>
    std::basic_string<CharT> StringReader<CharT>::ReadStringUntilOneOf(CharT const* terminators)
    {
        std::basic_string<CharT> result;
        result.reserve(GetRemainingLength());

        bool escaped = false;
        while (CanRead()) {
            CharT c = Read();
            if (escaped) {
                if (c == SYNTAX_ESCAPE) {
                    result += c;
                    escaped = false;
                }
                else for (CharT const* t = terminators; *t != 0; t++) {
                    if (c == *t) {
                        result += c;
                        escaped = false;
                        break;
                    }
                }
                if (escaped) {
                    SetCursor(GetCursor() - 1);
                    throw exceptions::ReaderInvalidEscape(*this, c);
                }
            }
            else if (c == SYNTAX_ESCAPE) {
                escaped = true;
            }
            else {
                for (CharT const* t = terminators; *t != 0; t++) {
                    if (c == *t) {
                        return result;
                    }
                }
                result += c;
            }
        }

        throw exceptions::ReaderExpectedOneOf(*this, terminators);
    }

    template<typename CharT>
    std::basic_string<CharT> StringReader<CharT>::ReadQuotedString()
    {
        if (!CanRead()) {
            return {};
        }
        CharT next = Peek();
        if (!IsQuotedStringStart(next)) {
            throw exceptions::ReaderExpectedStartOfQuote(*this);
        }
        Skip();
        return ReadStringUntil(next);
    }

    template<typename CharT>
    void StringReader<CharT>::Expect(CharT c)
    {
        if (!CanRead() || Peek() != c) {
            throw exceptions::ReaderExpectedSymbol(*this, c);
        }
        Skip();
    }

    template<typename CharT>
    template<typename T>
    T StringReader<CharT>::ParseValue(std::basic_string_view<CharT> value, size_t start)
    {
        if (value.empty()) {
            throw exceptions::ReaderExpectedValue(*this);
        }

        if constexpr (std::is_same_v<std::remove_cv_t<T>, bool>)
        {
            /**/ if (value == BRIGADIER_LITERAL(CharT, "true"))
                return true;
            else if (value == BRIGADIER_LITERAL(CharT, "false"))
                return false;
            else {
                cursor = start;
                throw exceptions::ReaderInvalidValue(*this, value);
            }
        }
        else
        {
            T ret{};
            //std::basic_istringstream<CharT> s(std::basic_string<CharT>(value));
            basic_istringviewstream<CharT> s(value);
            s >> ret;

            if (s.eof() && !s.bad() && !s.fail())
                return ret;
            else {
                cursor = start;
                throw exceptions::ReaderInvalidValue(*this, value);
            }
        }
    }

    template<typename CharT>
    template<typename T>
    T StringReader<CharT>::ReadValue()
    {
        if (!CanRead()) {
            throw exceptions::ReaderExpectedValue(*this);
        }

        size_t start = cursor;

        if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
        {
            while (CanRead() && IsAllowedNumber<std::is_floating_point_v<T>, std::is_signed_v<T>>(Peek())) {
                Skip();
            }
            return ParseValue<T>(string.substr(start, cursor - start), start);
        }
        else
        {
            CharT next = Peek();
            if (IsQuotedStringStart(next)) {
                Skip();
                return ParseValue<T>(ReadStringUntil(next), start);
            }
            return ParseValue<T>(ReadUnquotedString(), start);
        }
    }
}
