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
        static constexpr CharT SYNTAX_ESCAPE = CharT('\\');
        static constexpr CharT SYNTAX_SINGLE_QUOTE = CharT('\'');
        static constexpr CharT SYNTAX_DOUBLE_QUOTE = CharT('\"');
    public:
        StringReader(std::basic_string_view<CharT> string) : string(string) {}
        StringReader() {}

        inline std::basic_string_view<CharT> GetString()                const { return string; }
        inline void                          SetCursor(size_t cursor) { this->cursor = cursor; }
        inline ptrdiff_t                     GetRemainingLength()       const { return string.length() - cursor; }
        inline size_t                        GetTotalLength()           const { return string.length(); }
        inline size_t                        GetCursor()                const { return cursor; }
        inline std::basic_string_view<CharT> GetRead()                  const { return string.substr(0, cursor); }
        inline std::basic_string_view<CharT> GetRemaining()             const { return string.substr(cursor); }
        inline bool                          CanRead(size_t length = 1) const { return (cursor + length) <= string.length(); }
        inline CharT                         Peek(size_t offset = 0)    const { return string.at(cursor + offset); }
        inline CharT                         Read() { return string.at(cursor++); }
        inline void                          Skip() { cursor++; }
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