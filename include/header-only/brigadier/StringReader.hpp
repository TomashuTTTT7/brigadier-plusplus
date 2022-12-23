#pragma once

#include <string>
#include <string_view>
#include <istream>
#include <streambuf>
#include "Common.hpp"

namespace brigadier
{
    template<typename CharT, typename traits = std::char_traits<CharT>>
    class basic_stringviewbuf : public std::basic_streambuf<CharT, traits>
    {
        using base = std::basic_streambuf<CharT, traits>;
    protected:
        typename base::pos_type seekoff(typename base::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override
        {
            if (dir == std::ios_base::cur)
                this->gbump((int)off);
            else if (dir == std::ios_base::end)
                this->setg(this->eback(), this->egptr() + off, this->egptr());
            else if (dir == std::ios_base::beg)
                this->setg(this->eback(), this->eback() + off, this->egptr());
            return this->gptr() - this->eback();
        }

        typename base::pos_type seekpos(typename base::pos_type sp, std::ios_base::openmode which) override {
            return seekoff(sp - typename base::pos_type(typename base::off_type(0)), std::ios_base::beg, which);
        }
    public:
        basic_stringviewbuf(CharT const* s, size_t count) {
            auto p = const_cast<CharT*>(s);
            this->setg(p, p, p + count);
        }
        basic_stringviewbuf(std::basic_string_view<CharT> s) : basic_stringviewbuf(s.data(), s.size()) {}
    };
    using stringviewbuf = basic_stringviewbuf<char>;
    using wstringviewbuf = basic_stringviewbuf<wchar_t>;

    template<typename CharT, typename traits = std::char_traits<CharT>>
    class basic_istringviewstream : public std::basic_istream<CharT, traits> {
    public:
        basic_istringviewstream(std::basic_string_view<CharT> s) : std::basic_istream<CharT, traits>(&buf), buf(s) {}
    private:
        basic_stringviewbuf<CharT, traits> buf;
    };
    using istringviewstream = basic_istringviewstream<char>;
    using wistringviewstream = basic_istringviewstream<wchar_t>;


    template<typename CharT>
    class StringReader
    {
    private:
        static constexpr CharT SYNTAX_ESCAPE       = CharT('\\');
        static constexpr CharT SYNTAX_SINGLE_QUOTE = CharT('\'');
        static constexpr CharT SYNTAX_DOUBLE_QUOTE = CharT('\"');
    public:
        StringReader(std::basic_string_view<CharT> string) : string(string) {}
        StringReader() {}

        inline std::basic_string_view<CharT> GetString()                const { return string; }
        inline void                          SetCursor(size_t cursor)         { this->cursor = cursor; }
        inline ptrdiff_t                     GetRemainingLength()       const { return string.length() - cursor; }
        inline size_t                        GetTotalLength()           const { return string.length(); }
        inline size_t                        GetCursor()                const { return cursor; }
        inline std::basic_string_view<CharT> GetRead()                  const { return string.substr(0, cursor); }
        inline std::basic_string_view<CharT> GetRemaining()             const { return string.substr(cursor); }
        inline bool                          CanRead(size_t length = 1) const { return (cursor + length) <= string.length(); }
        inline CharT                         Peek(size_t offset = 0)    const { return string.at(cursor + offset); }
        inline CharT                         Read()                           { return string.at(cursor++); }
        inline void                          Skip()                           { cursor++; }
    public:
        inline void SkipWhitespace() {
            while (CanRead() && std::isspace(Peek())) {
                Skip();
            }
        }
    public:
        inline std::basic_string_view<CharT> ReadUnquotedString();
        inline std::basic_string_view<CharT> ReadUnquotedStringUntil(CharT terminator); // Terminator is skipped
        inline std::basic_string_view<CharT> ReadUnquotedStringUntilOneOf(CharT const* terminators); // Terminator is skipped
        inline std::basic_string<CharT>      ReadString();
        inline std::basic_string<CharT>      ReadStringUntil(CharT terminator); // Terminator is skipped
        inline std::basic_string<CharT>      ReadStringUntilOneOf(CharT const* terminators); // Terminator is skipped
        inline std::basic_string<CharT>      ReadQuotedString();
        inline void                          Expect(CharT c);
    private:
        template<typename T>
        T ParseValue(std::basic_string_view<CharT> value, size_t start);
    public:
        template<typename T>
        T ReadValue();
    public:
        inline static bool IsAllowedInUnquotedString(CharT c) {
            return (c >= CharT('0') && c <= CharT('9'))
                || (c >= CharT('A') && c <= CharT('Z'))
                || (c >= CharT('a') && c <= CharT('z'))
                || (c == CharT('_') || c == CharT('-'))
                || (c == CharT('.') || c == CharT('+'));
        }
        inline static bool IsQuotedStringStart(CharT c) {
            return c == SYNTAX_DOUBLE_QUOTE || c == SYNTAX_SINGLE_QUOTE;
        }
        template<bool allow_float = true, bool allow_negative = true>
        inline static bool IsAllowedNumber(CharT c) {
            return (c >= CharT('0') && c <= CharT('9')) || (allow_float && c == CharT('.')) || (allow_negative && c == CharT('-'));
        }
    private:
        std::basic_string_view<CharT> string;
        size_t cursor = 0;
    };
    BRIGADIER_SPECIALIZE_BASIC(StringReader);
}

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
