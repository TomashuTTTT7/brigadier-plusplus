/*
MIT License
Copyright (c) 2022 Tomasz Karpinski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* This file is generated from header-only version. Do not edit directly. */

#pragma once

#include <algorithm>
#include <cstring>
#include <future>
#include <istream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <streambuf>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#pragma once

#define BRIGADIER_PACK(...) __VA_ARGS__

namespace brigadier::detail
{
    template<size_t SZ0, size_t SZ1, size_t SZ2, size_t SZ3>
    constexpr inline auto literal_type(char c,
        const char(&s0)[SZ0],
        const wchar_t(&s1)[SZ1],
        const char16_t(&s2)[SZ2],
        const char32_t(&s3)[SZ3]
    ) -> const char(&)[SZ0]
    {
        return s0;
    };

    template<size_t SZ0, size_t SZ1, size_t SZ2, size_t SZ3>
    constexpr inline auto literal_type(wchar_t c,
        const char(&s0)[SZ0],
        const wchar_t(&s1)[SZ1],
        const char16_t(&s2)[SZ2],
        const char32_t(&s3)[SZ3]
    ) -> const wchar_t(&)[SZ1]
    {
        return s1;
    };

    template<size_t SZ0, size_t SZ1, size_t SZ2, size_t SZ3>
    constexpr inline auto literal_type(char16_t c,
        const char(&s0)[SZ0],
        const wchar_t(&s1)[SZ1],
        const char16_t(&s2)[SZ2],
        const char32_t(&s3)[SZ3]
    ) -> const char16_t(&)[SZ2]
    {
        return s2;
    };

    template<size_t SZ0, size_t SZ1, size_t SZ2, size_t SZ3>
    constexpr inline auto literal_type(char32_t c,
        const char(&s0)[SZ0],
        const wchar_t(&s1)[SZ1],
        const char16_t(&s2)[SZ2],
        const char32_t(&s3)[SZ3]
    ) -> const char32_t(&)[SZ3]
    {
        return s3;
    };

    constexpr inline auto literal_type(char c, char s0, wchar_t s1, char16_t s2, char32_t s3) -> char
    {
        return s0;
    };

    constexpr inline auto literal_type(wchar_t c, char s0, wchar_t s1, char16_t s2, char32_t s3) -> wchar_t
    {
        return s1;
    };

    constexpr inline auto literal_type(char16_t c, char s0, wchar_t s1, char16_t s2, char32_t s3) -> char16_t
    {
        return s2;
    };

    constexpr inline auto literal_type(char32_t c, char s0, wchar_t s1, char16_t s2, char32_t s3) -> char32_t
    {
        return s3;
    };
}
#define BRIGADIER_LITERAL(type, s_literal) brigadier::detail::literal_type(type(), s_literal, L##s_literal, u##s_literal, U##s_literal)

#ifdef UNICODE
#define BRIGADIER_SPECIALIZE_BASIC(type) \
using type##A = type<char>;              \
using type##T = type<wchar_t>;           \
using type##W = type<wchar_t>;

#define BRIGADIER_SPECIALIZE_BASIC_TEMPL(type)                 \
template<typename... Ts> using type##A = type<char, Ts...>;    \
template<typename... Ts> using type##T = type<wchar_t, Ts...>; \
template<typename... Ts> using type##W = type<wchar_t, Ts...>;

#define BRIGADIER_SPECIALIZE_BASIC_ALIAS(type, tepl_list, templ_spec) \
template<tepl_list> using type##A = type<char, templ_spec>;           \
template<tepl_list> using type##T = type<wchar_t, templ_spec>;        \
template<tepl_list> using type##W = type<wchar_t, templ_spec>;
#else
#define BRIGADIER_SPECIALIZE_BASIC(type) \
using type##A = type<char>;              \
using type##T = type<char>;              \
using type##W = type<wchar_t>;

#define BRIGADIER_SPECIALIZE_BASIC_TEMPL(type)               \
template<typename... Ts> using type##A = type<char, Ts...>;  \
template<typename... Ts> using type##T = type<char, Ts...>;  \
template<typename... Ts> using type##W = type<wchar_t, Ts...>;

#define BRIGADIER_SPECIALIZE_BASIC_ALIAS(type, tepl_list, templ_spec) \
template<tepl_list> using type##A = type<char, templ_spec>;           \
template<tepl_list> using type##T = type<char, templ_spec>;           \
template<tepl_list> using type##W = type<wchar_t, templ_spec>;
#endif
#pragma once

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
#pragma once

namespace brigadier
{
    template<typename CharT, typename E>
    class Exception {
    public:
        Exception() {}
        Exception(Exception&&) = default;
        Exception(Exception const& that) {
            this->message << that.message.str();
        }
    public:
        template<typename T>
        E&& operator<<(T&& arg)&& {
            message << arg;
            return std::move(*static_cast<E*>(this));
        }
    public:
        std::basic_string<CharT> What() const {
            return message.str();
        }
    protected:
        std::basic_ostringstream<CharT> message;
    };

    template<typename CharT>
    class RuntimeError : public Exception<CharT, RuntimeError<CharT>> {};
    BRIGADIER_SPECIALIZE_BASIC(RuntimeError);

    static constexpr size_t default_context_amount = 10;

    template<typename CharT>
    class CommandSyntaxException : public Exception<CharT, CommandSyntaxException<CharT>> {
    public:
        CommandSyntaxException(StringReader<CharT> context) : context(context) {}
        CommandSyntaxException(std::nullptr_t) {}
        CommandSyntaxException() {}
        CommandSyntaxException(CommandSyntaxException&&) = default;
        CommandSyntaxException(CommandSyntaxException const& that) {
            this->context = that.context;
        }
        //virtual ~CommandSyntaxException() = default;
    public:
        inline StringReader<CharT> const& GetContext() const { return context; }
        inline size_t GetCursor() const { return context.GetCursor(); }
        inline std::basic_string_view<CharT> GetString() const { return context.GetString(); }
    private:
        void DescribeContext(size_t context_amount) {
            if (!context.GetString().empty()) {
                size_t cursor = context->GetCursor();
                message << BRIGADIER_LITERAL(CharT, " at position ");
                message << cursor;
                message << BRIGADIER_LITERAL(CharT, ": ");
                if (cursor > context_amount)
                    message << BRIGADIER_LITERAL(CharT, "...");
                message << context.GetString().substr(cursor > context_amount ? cursor - context_amount : 0, context_amount);
                message << BRIGADIER_LITERAL(CharT, "<--[HERE]");
            }
        }
        //virtual void DescribeException() {}
    public:
        std::basic_string<CharT> What(size_t context_amount = default_context_amount) {
            //DescribeException();
            DescribeContext(context_amount);
            return Exception<CharT, CommandSyntaxException<CharT>>::What();
        }
    protected:
        StringReader<CharT> context;
    };
    BRIGADIER_SPECIALIZE_BASIC(CommandSyntaxException);

    namespace exceptions
    {
        template<typename CharT, typename T0, typename T1> static inline CommandSyntaxException<CharT> ValueTooLow                        (StringReader<CharT> ctx, T0 found, T1 min)    { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Value must not be less than ") << min << BRIGADIER_LITERAL(CharT, ", found ") << found; }
        template<typename CharT, typename T0, typename T1> static inline CommandSyntaxException<CharT> ValueTooHigh                       (StringReader<CharT> ctx, T0 found, T1 max)    { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Value must not be more than ") << max << BRIGADIER_LITERAL(CharT, ", found ") << found; }
        template<typename CharT, typename T0>              static inline CommandSyntaxException<CharT> LiteralIncorrect                   (StringReader<CharT> ctx, T0 const& expected)  { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected literal ") << expected; }
        template<typename CharT>                           static inline CommandSyntaxException<CharT> ReaderExpectedStartOfQuote         (StringReader<CharT> ctx)                      { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected quote to start a string"); }
        template<typename CharT>                           static inline CommandSyntaxException<CharT> ReaderExpectedEndOfQuote           (StringReader<CharT> ctx)                      { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Unclosed quoted string"); }
        template<typename CharT, typename T0>              static inline CommandSyntaxException<CharT> ReaderInvalidEscape                (StringReader<CharT> ctx, T0 const& character) { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Invalid escape sequence '") << character << BRIGADIER_LITERAL(CharT, "' in quoted string"); }
        template<typename CharT, typename T0>              static inline CommandSyntaxException<CharT> ReaderInvalidValue                 (StringReader<CharT> ctx, T0 const& value)     { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Invalid value '") << value << CharT('\''); }
        template<typename CharT>                           static inline CommandSyntaxException<CharT> ReaderExpectedValue                (StringReader<CharT> ctx)                      { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected value"); }
        template<typename CharT, typename T0>              static inline CommandSyntaxException<CharT> ReaderExpectedSymbol               (StringReader<CharT> ctx, T0 const& symbol)    { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected '") << symbol << CharT('\''); }
        template<typename CharT, typename T0>              static inline CommandSyntaxException<CharT> ReaderExpectedOneOf                (StringReader<CharT> ctx, T0 const& symbols)   { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected one of `") << symbols << CharT('`'); }
        template<typename CharT>                           static inline CommandSyntaxException<CharT> DispatcherUnknownCommand           (StringReader<CharT> ctx)                      { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Unknown command"); }
        template<typename CharT>                           static inline CommandSyntaxException<CharT> DispatcherUnknownArgument          (StringReader<CharT> ctx)                      { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Incorrect argument for command"); }
        template<typename CharT>                           static inline CommandSyntaxException<CharT> DispatcherExpectedArgumentSeparator(StringReader<CharT> ctx)                      { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected whitespace to end one argument, but found trailing data"); }
        template<typename CharT, typename T0>              static inline CommandSyntaxException<CharT> DispatcherParseException           (StringReader<CharT> ctx, T0 const& message)   { return CommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Could not parse command: ") << message; }
    }
}
#pragma once

// see StringReader.hxx for class body

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
#pragma once

namespace brigadier
{
    class StringRange
    {
    public:
        StringRange(size_t start, size_t end) : start(start), end(end) {}

        inline size_t GetStart() const { return start; }
        inline size_t GetEnd()   const { return end;   }

        inline static StringRange At(size_t pos) { return StringRange(pos, pos); }
        inline static StringRange Between(size_t start, size_t end) { return StringRange(start, end); }
        inline static StringRange Encompassing(StringRange const& a, StringRange const& b) {
            return StringRange((std::min)(a.GetStart(), b.GetStart()), (std::max)(a.GetEnd(), b.GetEnd()));
        }

        template<typename CharT>
        inline std::basic_string_view<CharT> Get(StringReader<CharT> reader) const {
            return reader.GetString().substr(start, end - start);
        }
        template<typename CharT>
        inline std::basic_string_view<CharT> Get(std::basic_string_view<CharT> string) const {
            return string.substr(start, end - start);
        }

        inline bool      IsEmpty()   const { return start == end; }
        inline ptrdiff_t GetLength() const { return end - start;  }

        inline bool operator==(StringRange const& other) const { return (start == other.start && end == other.end); }
    private:
        size_t start = 0;
        size_t end = 0;
    };
}
#pragma once

namespace brigadier
{
    template<typename CharT>
    class Suggestions;
    template<typename CharT>
    class SuggestionsBuilder;

    template<typename CharT>
    class Suggestion
    {
    public:
        Suggestion(StringRange range, std::basic_string_view<CharT> text, std::basic_string_view<CharT> tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        Suggestion(StringRange range, std::basic_string_view<CharT> text) : range(std::move(range)), text(std::move(text)) {}

        inline StringRange GetRange() const { return range; }
        inline std::basic_string<CharT> const& GetText() const { return text; }
        inline std::basic_string_view<CharT> GetTooltip() const { return tooltip; }

        std::basic_string<CharT> Apply(std::basic_string_view<CharT> input) const
        {
            if (range.GetStart() == 0 && range.GetEnd() == input.length()) {
                return text;
            }
            std::basic_string<CharT> result;
            result.reserve(range.GetStart() + text.length() + input.length() - (std::min)(range.GetEnd(), input.length()));
            if (range.GetStart() > 0) {
                result.append(input.substr(0, range.GetStart()));
            }
            result.append(text);
            if ((size_t)range.GetEnd() < input.length()) {
                result.append(input.substr(range.GetEnd()));
            }
            return result;
        }

        void Expand(std::basic_string_view<CharT> command, StringRange range)
        {
            if (this->range == range)
                return;

            if (range.GetStart() < this->range.GetStart()) {
                text.insert(0, command.substr(range.GetStart(), this->range.GetStart() - range.GetStart()));
            }
            if (range.GetEnd() > this->range.GetEnd()) {
                text.append(command.substr(this->range.GetEnd(), range.GetEnd() - this->range.GetEnd()));
            }

            this->range = range;
        }
    protected:
        friend class Suggestions<CharT>;
        friend class SuggestionsBuilder<CharT>;
        Suggestion<CharT>(std::basic_string<CharT> text, StringRange range, std::basic_string_view<CharT> tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        Suggestion<CharT>(std::basic_string<CharT> text, StringRange range) : range(std::move(range)), text(std::move(text)) {}
    private:
        StringRange range;
        std::basic_string<CharT> text;
        std::basic_string_view<CharT> tooltip;
    };
    BRIGADIER_SPECIALIZE_BASIC(Suggestion);

    template<typename CharT>
    struct CompareNoCase {
        inline bool operator() (Suggestion<CharT> const& a, Suggestion<CharT> const& b) const
        {
            CharT const* s1 = a.GetText().data();
            CharT const* s2 = b.GetText().data();

            for (size_t i = 0; s1[i] && s2[i]; ++i)
            {
                int diff = std::tolower(s1[i]) - std::tolower(s2[i]);
                if (diff) return diff < 0;
            }

            return false;
        }
    };
}
#pragma once

namespace brigadier
{
    template<typename CharT>
    class Suggestions
    {
    public:
        Suggestions(StringRange range, std::set<Suggestion<CharT>, CompareNoCase<CharT>> suggestions) : range(std::move(range)), suggestions(std::move(suggestions)) {}
        Suggestions() : Suggestions<CharT>(StringRange::At(0), {}) {}

        inline StringRange GetRange() const { return range; }
        inline std::set<Suggestion<CharT>, CompareNoCase<CharT>> const& GetList() const { return suggestions; }
        inline bool IsEmpty() { return suggestions.empty(); }

        static inline std::future<Suggestions<CharT>> Empty()
        {
            std::promise<Suggestions<CharT>> f;
            f.set_value(Suggestions<CharT>());
            return f.get_future();
        }

        static inline Suggestions<CharT> Merge(std::basic_string_view<CharT> command, std::vector<Suggestions<CharT>> const& input, bool* cancel = nullptr)
        {
            /**/ if (input.empty()) return {};
            else if (input.size() == 1) return input.front();

            std::vector<Suggestion<CharT>> suggestions;

            for (auto& sugs : input)
            {
                suggestions.insert(suggestions.end(), sugs.GetList().begin(), sugs.GetList().end());
            }
            return Suggestions<CharT>::Create(command, suggestions, cancel);
        }
        static inline Suggestions<CharT> Create(std::basic_string_view<CharT> command, std::vector<Suggestion<CharT>>& suggestions, bool* cancel = nullptr)
        {
            Suggestions<CharT> ret;
            if (suggestions.empty()) return ret;
            size_t start = std::numeric_limits<size_t>::max();
            size_t end = std::numeric_limits<size_t>::min();
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return ret;
                start = (std::min)(suggestion.GetRange().GetStart(), start);
                end = (std::max)(suggestion.GetRange().GetEnd(), end);
            }
            ret.range = StringRange(start, end);
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return ret;
                suggestion.Expand(command, ret.range);
            }
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return ret;
                ret.suggestions.insert(std::move(suggestion));
            }
            return ret;
        }
    private:
        StringRange range;
        std::set<Suggestion<CharT>, CompareNoCase<CharT>> suggestions;
    };
    BRIGADIER_SPECIALIZE_BASIC(Suggestions);
}
#pragma once

namespace brigadier
{
    template<typename CharT>
    class SuggestionsBuilder
    {
    public:
        SuggestionsBuilder(std::basic_string_view<CharT> input, std::basic_string_view<CharT> inputLowerCase, size_t start, bool* cancel = nullptr) : start(start), input(input), inputLowerCase(inputLowerCase), remaining(input.substr(start)), remainingLowerCase(inputLowerCase.substr(start)), cancel(cancel) {}

        inline int GetStart() const { return start; }
        inline std::basic_string_view<CharT> GetInput() const { return input; }
        inline std::basic_string_view<CharT> GetInputLowerCase() const { return inputLowerCase; }
        inline std::basic_string_view<CharT> GetRemaining() const { return remaining; }
        inline std::basic_string_view<CharT> GetRemainingLowerCase() const { return remainingLowerCase; }

        Suggestions<CharT> Build(bool* cancel = nullptr)
        {
            auto ret = Suggestions<CharT>::Create(input, result, cancel);
            if (cancel != nullptr) *cancel = false;
            result.clear();
            return ret;
        }
        inline std::future<Suggestions<CharT>> BuildFuture()
        {
            return std::async(std::launch::async, &SuggestionsBuilder<CharT>::Build, this, this->cancel);
        }

        inline SuggestionsBuilder<CharT>& Suggest(std::basic_string_view<CharT> text)
        {
            if (text == remaining) return *this;

            result.emplace_back(StringRange::Between(start, input.length()), text);
            return *this;
        }
        inline SuggestionsBuilder<CharT>& Suggest(std::basic_string_view<CharT> text, std::basic_string_view<CharT> tooltip)
        {
            if (text == remaining) return *this;

            result.emplace_back(StringRange::Between(start, input.length()), text, tooltip);
            return *this;
        }
        template<typename T>
        SuggestionsBuilder<CharT>& Suggest(T value)
        {
            result.emplace_back(std::move(std::to_string(value)), StringRange::Between(start, input.length()));
            return *this;
        }
        template<typename T>
        SuggestionsBuilder<CharT>& Suggest(T value, std::basic_string_view<CharT> tooltip)
        {
            result.emplace_back(std::move(std::to_string(value)), StringRange::Between(start, input.length()), tooltip);
            return *this;
        }
        inline int AutoSuggest(std::basic_string_view<CharT> text, std::basic_string_view<CharT> input)
        {
            if (text.rfind(input.substr(0, text.length()), 0) == 0)
            {
                Suggest(text);
                return 1;
            }
            return 0;
        }
        inline int AutoSuggest(std::basic_string_view<CharT> text, std::basic_string_view<CharT> tooltip, std::basic_string_view<CharT> input)
        {
            if (text.rfind(input.substr(0, text.length()), 0) == 0)
            {
                Suggest(text, tooltip);
                return 1;
            }
            return 0;
        }
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        int AutoSuggest(T value, std::basic_string_view<CharT> input)
        {
            std::basic_string<CharT> val = std::to_string(value);
            if (val.rfind(input.substr(0, val.length()), 0) == 0)
            {
                result.emplace_back(std::move(val), StringRange::Between(start, input.length()));
                return 1;
            }
            return 0;
        }
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        int AutoSuggest(T value, std::basic_string_view<CharT> tooltip, std::basic_string_view<CharT> input)
        {
            std::basic_string<CharT> val = std::to_string(value);

            if (val.rfind(input.substr(0, val.length()), 0) == 0)
            {
                result.emplace_back(std::move(val), StringRange::Between(start, input.length()), tooltip);
                return 1;
            }
            return 0;
        }
        template<typename Container>
        inline int AutoSuggest(Container const& init)
        {
            int counter = 0;
            for (auto& val : init)
            {
                counter += AutoSuggest(val, GetRemaining());
            }
            return counter;
        }
        template<typename Container>
        inline int AutoSuggestLowerCase(Container const& init)
        {
            int counter = 0;
            for (auto& val : init)
            {
                counter += AutoSuggest(val, GetRemainingLowerCase());
            }
            return counter;
        }

        inline SuggestionsBuilder<CharT>& Add(SuggestionsBuilder<CharT> const& other)
        {
            result.insert(result.end(), other.result.begin(), other.result.end());
            return *this;
        }

        inline void SetOffset(size_t start)
        {
            this->start = start;
            remaining = input.substr(start);
            remainingLowerCase = inputLowerCase.substr(start);

            Restart();
        }
        inline void Restart()
        {
            result.clear();
        }

        ~SuggestionsBuilder<CharT>() = default;
    private:
        size_t start = 0;
        std::basic_string_view<CharT> input;
        std::basic_string_view<CharT> inputLowerCase;
        std::basic_string_view<CharT> remaining;
        std::basic_string_view<CharT> remainingLowerCase;
        std::vector<Suggestion<CharT>> result;
        bool* cancel = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC(SuggestionsBuilder);
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class CommandNode;
    template<typename CharT, typename S>
    class CommandContext;

    template<typename... Ts>
    using Predicate = bool(*)(Ts&&... args);
    template<typename CharT, typename S>
    using AmbiguityConsumer = void(*)(CommandNode<CharT, S>* parent, CommandNode<CharT, S>* child, CommandNode<CharT, S>* sibling, std::set<std::basic_string_view<CharT>>& inputs);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(AmbiguityConsumer, typename S, S);
    template<typename CharT, typename S>
    using Command = int(*)(CommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(Command, typename S, S);
    template<typename CharT, typename S>
    using RedirectModifier = std::vector<S>(*)(CommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(RedirectModifier, typename S, S);
    template<typename CharT, typename S>
    using SingleRedirectModifier = S(*)(CommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(SingleRedirectModifier, typename S, S);
    template<typename CharT, typename S>
    using ResultConsumer = void(*)(CommandContext<CharT, S>& context, bool success, int result);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(ResultConsumer, typename S, S);
    template<typename CharT, typename S>
    using SuggestionProvider = std::future<Suggestions<CharT>>(*)(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(SuggestionProvider, typename S, S);
}

#define COMMAND(S, ...) [](auto& ctx) -> int __VA_ARGS__
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class CommandNode;
    template<typename CharT, typename S>
    class IArgumentCommandNode;
    template<typename CharT, typename S>
    class LiteralCommandNode;
    template<typename CharT, typename S>
    class RootCommandNode;
    template<typename CharT, typename S>
    class CommandDispatcher;

    template<typename CharT, typename S, typename T, typename node_type>
    class ArgumentBuilder;
    template<typename CharT, typename S>
    class MultiArgumentBuilder;
    template<typename CharT, typename S>
    class LiteralArgumentBuilder;
    template<typename CharT, typename S, typename T>
    class RequiredArgumentBuilder;

    template<typename CharT, typename S>
    class CommandContext;

    enum class CommandNodeType
    {
        RootCommandNode,
        LiteralCommandNode,
        ArgumentCommandNode
    };

    template<typename CharT, typename S>
    class CommandNode
    {
    public:
        CommandNode(Command<CharT, S> command, Predicate<S&> requirement, std::shared_ptr<CommandNode<CharT, S>> redirect, RedirectModifier<CharT, S> modifier, const bool forks)
            : command(std::move(command))
            , requirement(std::move(requirement))
            , redirect(std::move(redirect))
            , modifier(std::move(modifier))
            , forks(std::move(forks))
        {}
        CommandNode() {}
        virtual ~CommandNode() = default;
    public:
        inline Command<CharT, S> GetCommand() const
        {
            return command;
        }

        inline std::map<std::basic_string<CharT>, std::shared_ptr<CommandNode<CharT, S>>, std::less<>> const& GetChildren() const
        {
            return children;
        }

        inline std::shared_ptr<CommandNode<CharT, S>> GetChild(std::basic_string_view<CharT> name) const
        {
            auto found = children.find(name);
            if (found != children.end())
                return found->second;
            return nullptr;
        }

        inline std::shared_ptr<CommandNode<CharT, S>> GetRedirect() const
        {
            return redirect;
        }

        inline RedirectModifier<CharT, S> GetRedirectModifier() const 
        {
            return modifier;
        }

        inline Predicate<S&> GetRequirement()
        {
            return requirement;
        }

        inline bool IsFork() const
        {
            return forks;
        }

        inline bool CanUse(S& source)
        {
            if (requirement)
                return requirement(source);
            else return true;
        }

        void AddChild(std::shared_ptr<CommandNode<CharT, S>> node)
        {
            if (node == nullptr)
                return;

            if (node->GetNodeType() == CommandNodeType::RootCommandNode) {
                throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add a RootCommandNode as a child to any other CommandNode");
            }

            auto child = children.find(node->GetName());
            if (child != children.end()) {
                // We've found something to merge onto
                auto child_node = child->second;

                if (child_node->GetNodeType() != node->GetNodeType())
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");

                auto node_command = node->GetCommand();
                if (node_command != nullptr) {
                    child_node->command = node_command;
                }
                for (auto& [name, grandchild] : node->GetChildren()) {
                    child_node->AddChild(grandchild);
                }
            }
            else {
                children.emplace(node->GetName(), node);
                if (node->GetNodeType() == CommandNodeType::LiteralCommandNode) {
                    literals.emplace_back(std::move(std::static_pointer_cast<LiteralCommandNode<CharT, S>>(std::move(node))));
                }
                else if (node->GetNodeType() == CommandNodeType::ArgumentCommandNode) {
                    arguments.emplace_back(std::move(std::static_pointer_cast<IArgumentCommandNode<CharT, S>>(std::move(node))));
                }
            }
        }

        void FindAmbiguities(AmbiguityConsumer<CharT, S> consumer)
        {
            for (auto [child_name, child] : children) {
                for (auto [sibling_name, sibling] : children) {
                    if (child == sibling)
                        continue;

                    std::set<std::basic_string_view<CharT>> matches;

                    for (auto input : child->GetExamples()) {
                        if (sibling->IsValidInput(input)) {
                            matches.insert(input);
                        }
                    }

                    if (matches.size() > 0) {
                        consumer(this, child, sibling, matches);
                    }
                }

                child.FindAmbiguities(consumer);
            }
        }

        std::tuple<std::shared_ptr<CommandNode<CharT, S>>*, size_t> GetRelevantNodes(StringReader<CharT>& input)
        {
            if (literals.size() > 0) {
                size_t cursor = input.GetCursor();
                while (input.CanRead() && input.Peek() != ' ') {
                    input.Skip();
                }
                std::basic_string_view<CharT> text = input.GetString().substr(cursor, input.GetCursor() - cursor);
                input.SetCursor(cursor);
                auto literal = children.find(text);
                if (literal != children.end() && literal->second->GetNodeType() == CommandNodeType::LiteralCommandNode) {
                    return std::tuple<std::shared_ptr<CommandNode<CharT, S>>*, size_t>(&literal->second, 1);
                }
                else {
                    return std::tuple<std::shared_ptr<CommandNode<CharT, S>>*, size_t>((std::shared_ptr<CommandNode<CharT, S>>*)arguments.data(), arguments.size());
                }
            }
            else {
                return std::tuple<std::shared_ptr<CommandNode<CharT, S>>*, size_t>((std::shared_ptr<CommandNode<CharT, S>>*)arguments.data(), arguments.size());
            }
        }

        bool HasCommand()
        {
            if (GetCommand() != nullptr) return true;
            for (auto [name, child] : children)
                if (child && child->HasCommand())
                    return true;
            return false;
        }
    public:
        virtual std::basic_string<CharT> const& GetName() = 0;
        virtual std::basic_string<CharT> GetUsageText() = 0;
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() = 0;
        virtual void Parse(StringReader<CharT>& reader, CommandContext<CharT, S>& contextBuilder) = 0;
        virtual std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder) = 0;

        virtual CommandNodeType GetNodeType() = 0;
    protected:
        template<typename, typename, typename, typename>
        friend class ArgumentBuilder;
        template<typename, typename>
        friend class MultiArgumentBuilder;
        template<typename, typename>
        friend class CommandDispatcher;
        template<typename, typename, typename>
        friend class RequiredArgumentBuilder;
        template<typename, typename>
        friend class LiteralArgumentBuilder;

        virtual bool IsValidInput(std::basic_string_view<CharT> input) = 0;
        virtual std::basic_string_view<CharT> GetSortedKey() = 0;
    private:
        std::map<std::basic_string<CharT>, std::shared_ptr<CommandNode<CharT, S>>, std::less<>> children;
        std::vector<std::shared_ptr<LiteralCommandNode<CharT, S>>> literals;
        std::vector<std::shared_ptr<IArgumentCommandNode<CharT, S>>> arguments;
        Command<CharT, S> command = nullptr;
        Predicate<S&> requirement = nullptr;
        std::shared_ptr<CommandNode<CharT, S>> redirect = nullptr;
        RedirectModifier<CharT, S> modifier = nullptr;
        bool forks = false;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandNode);
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class RootCommandNode : public CommandNode<CharT, S>
    {
    public:
        RootCommandNode() : CommandNode<CharT, S>(nullptr, [](S&) { return true; }, nullptr, [](auto s)->std::vector<S> { return { s.GetSource() }; }, false) {}

        virtual ~RootCommandNode() = default;
        virtual std::basic_string<CharT> const& GetName() { static const std::basic_string<CharT> blank; return blank; }
        virtual std::basic_string<CharT> GetUsageText() { return {}; }
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() { return {}; }
        virtual void Parse(StringReader<CharT>& reader, CommandContext<CharT, S>& contextBuilder) {}
        virtual std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder)
        {
            return Suggestions<CharT>::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::RootCommandNode; }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) { return false; }
        virtual std::basic_string_view<CharT> GetSortedKey() { return {}; }
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(RootCommandNode);
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class LiteralCommandNode : public CommandNode<CharT, S>
    {
    public:
        LiteralCommandNode(std::basic_string_view<CharT> literal, std::shared_ptr<Command<CharT, S>> command, Predicate<S&> requirement, std::shared_ptr<CommandNode<CharT, S>> redirect, RedirectModifier<CharT, S> modifier, const bool forks)
            : CommandNode<CharT, S>(command, requirement, redirect, modifier, forks)
            , literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](CharT c) { return std::tolower(c); });
        }
        LiteralCommandNode(std::basic_string_view<CharT> literal)
            : literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](CharT c) { return std::tolower(c); });
        }
        virtual ~LiteralCommandNode() = default;
        virtual std::basic_string<CharT> const& GetName() { return literal; }
        virtual std::basic_string<CharT> GetUsageText() { return literal; }
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() { return { literal }; }
        virtual void Parse(StringReader<CharT>& reader, CommandContext<CharT, S>& contextBuilder)
        {
            size_t start = reader.GetCursor();
            size_t end = Parse(reader);
            if (end != size_t(-1)) {
                contextBuilder.WithNode(this, StringRange::Between(start, end));
                return;
            }

            throw exceptions::LiteralIncorrect(reader, literal);
        }
        virtual std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder)
        {
            if (builder.AutoSuggest(literalLowerCase, builder.GetRemainingLowerCase()))
                return builder.BuildFuture();
            else
                return Suggestions<CharT>::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::LiteralCommandNode; }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) {
            StringReader<CharT> reader(input);
            return Parse(reader) != size_t(-1);
        }
        virtual std::basic_string_view<CharT> GetSortedKey() { return literal; }
    private:
        size_t Parse(StringReader<CharT>& reader)
        {
            size_t start = reader.GetCursor();
            if (reader.CanRead(literal.length())) {
                if (reader.GetString().substr(start, literal.length()) == literal) {
                    size_t end = start + literal.length();
                    reader.SetCursor(end);
                    if (!reader.CanRead() || reader.Peek() == CharT(' ')) {
                        return end;
                    }
                    else {
                        reader.SetCursor(start);
                    }
                }
            }
            return size_t(-1);
        }
    private:
        std::basic_string<CharT> literal;
        std::basic_string<CharT> literalLowerCase;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(LiteralCommandNode);
}
#pragma once

// Following code makes that you don't have to specify command source type inside arguments.
// Command source type is automatically distributed from dispatcher.

// Default registration for arguments with or without template, without specialization
#define BRIGADIER_REGISTER_ARGTYPE(type, name)                            \
using name = type

// Registration for arguments with template parameters
#define BRIGADIER_REGISTER_ARGTYPE_TEMPL(type, name)                      \
template<typename... Args>                                                \
using name = type<Args...>

// Registration for arguments with specialized templates
#define BRIGADIER_REGISTER_ARGTYPE_SPEC(type, name, ...)                  \
using name = type<__VA_ARGS__>

// Registration for arguments with specialized templates and template parameters
#define BRIGADIER_REGISTER_ARGTYPE_SPEC_TEMPL(type, name, ...)            \
template<typename... Args>                                                \
using name = type<__VA_ARGS__, Args...>

// Default registration for arguments with or without template, without specialization, with char type
#define BRIGADIER_REGISTER_ARGTYPE_CHAR(type, name)                       \
template<typename CharT>                                                  \
using name = type<CharT>

// Registration for arguments with template parameters, with char type
#define BRIGADIER_REGISTER_ARGTYPE_TEMPL_CHAR(type, name)                 \
template<typename CharT, typename... Args>                                \
using name = type<CharT, Args...>

// Registration for arguments with specialized templates, with char type
#define BRIGADIER_REGISTER_ARGTYPE_SPEC_CHAR(type, name, ...)             \
template<typename CharT>                                                  \
using name = type<CharT, __VA_ARGS__>

// Registration for arguments with specialized templates and template parameters, with char type
#define BRIGADIER_REGISTER_ARGTYPE_SPEC_TEMPL_CHAR(type, name, ...)       \
template<typename CharT, typename... Args>                                \
using name = type<CharT, __VA_ARGS__, Args...>
#pragma once

namespace brigadier
{
    struct TypeInfo
    {
        TypeInfo(size_t hash) : hash(hash) {}
        template<typename CharT, typename ArgType>
        static constexpr size_t Create() { return (((uintptr_t)((void*)ArgType::template GetTypeName<CharT>().data())) + (sizeof(typename ArgType::type) << 24) + (sizeof(ArgType) << 8)); }
        template<typename CharT, template<typename> typename ArgType>
        inline static constexpr size_t Create() { return Create<CharT, ArgType<CharT>>(); }
        inline bool operator==(TypeInfo const& other) { return hash == other.hash; }
        inline bool operator!=(TypeInfo const& other) { return hash != other.hash; }
        size_t hash = 0;
    };

    template<typename CharT, typename S>
    class IParsedArgument
    {
    public:
        IParsedArgument(size_t start, size_t end, TypeInfo typeInfo) : range(start, end), typeInfo(typeInfo) {}
        virtual ~IParsedArgument() = default;

        inline StringRange GetRange()    const { return range; }
        inline TypeInfo                GetTypeInfo() const { return typeInfo; }
    protected:
        StringRange range;
        TypeInfo typeInfo;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(IParsedArgument);

    template<typename CharT, typename S, typename ArgType>
    class ParsedArgument : public IParsedArgument<CharT, S>
    {
    public:
        using T = typename ArgType::type;

        ParsedArgument(size_t start, size_t end, T result) : IParsedArgument<CharT, S>(start, end, TypeInfo(TypeInfo::Create<CharT, ArgType>())), result(std::move(result)) {}
        virtual ~ParsedArgument() = default;

        inline T&       GetResult()       { return result; }
        inline T const& GetResult() const { return result; }
    private:
        T result;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParsedArgument);
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class ParsedCommandNode
    {
    public:
        ParsedCommandNode(CommandNode<CharT, S>* node, StringRange range) : node(node), range(std::move(range)) {}

        inline CommandNode<CharT, S>* GetNode()  const { return node;  }
        inline StringRange     GetRange() const { return range; }
    private:
        CommandNode<CharT, S>* node;
        StringRange range;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParsedCommandNode);
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class SuggestionContext
    {
    public:
        SuggestionContext(CommandNode<CharT, S>* parent, size_t startPos) : parent(parent), startPos(startPos) {}

        CommandNode<CharT, S>* parent;
        size_t startPos;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(SuggestionContext);
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class CommandDispatcher;
    template<typename CharT, typename S>
    class LiteralCommandNode;
    template<typename CharT, typename S>
    class CommandNode;
    template<typename CharT, typename S, typename T>
    class ArgumentCommandNode;
    namespace detail
    {
        template<typename CharT, typename S>
        class CommandContextInternal;
    }

    template<typename CharT, typename S>
    class CommandContext
    {
    public:
        CommandContext(S source, CommandNode<CharT, S>* root, size_t start) : source(std::move(source)), context(std::make_unique<detail::CommandContextInternal<CharT, S>>(root, start)) {}
        CommandContext(S source, CommandNode<CharT, S>* root, StringRange range) : source(std::move(source)), context(std::make_unique<detail::CommandContextInternal<CharT, S>>(root, range)) {}

        inline CommandContext<CharT, S> GetFor(S source) const;
        inline CommandContext<CharT, S>* GetChild() const;
        inline CommandContext<CharT, S>* GetLastChild() const;
        inline CommandContext<CharT, S>* GetParent() const;
        inline CommandContext<CharT, S>* GetLastParent() const;
        inline Command<CharT, S> GetCommand() const;
        inline S& GetSource();
        inline S const& GetSource() const;
        inline RedirectModifier<CharT, S> GetRedirectModifier() const;
        inline StringRange GetRange() const;
        inline std::basic_string_view<CharT> GetInput() const;
        inline CommandNode<CharT, S>* GetRootNode() const;
        inline std::vector<ParsedCommandNode<CharT, S>>& GetNodes() const;
    protected:
        inline detail::CommandContextInternal<CharT, S>* GetInternalContext() const;
    public:
        inline bool HasNodes() const;
        inline bool IsForked() const;

        template<typename ArgType>
        typename ArgType::type GetArgument(std::basic_string_view<CharT> name);
        template<typename ArgType>
        std::optional<typename ArgType::type> GetArgumentOpt(std::basic_string_view<CharT> name);
        template<typename ArgType>
        typename ArgType::type GetArgumentOr(std::basic_string_view<CharT> name, typename ArgType::type default_value);
        
        template<template<typename> typename ArgType>
        inline typename ArgType<CharT>::type GetArgument(std::basic_string_view<CharT> name) {
            return GetArgument<ArgType<CharT>>(name);
        }
        template<template<typename> typename ArgType>
        inline std::optional<typename ArgType<CharT>::type> GetArgumentOpt(std::basic_string_view<CharT> name) {
            return GetArgumentOpt<ArgType<CharT>>(name);
        }
        template<template<typename> typename ArgType>
        inline typename ArgType<CharT>::type GetArgumentOr(std::basic_string_view<CharT> name, typename ArgType<CharT>::type default_value) {
            return GetArgumentOr<ArgType<CharT>>(name, std::move(default_value));
        }

        ~CommandContext();
    protected:
        inline CommandContext<CharT, S>& WithInput(std::basic_string_view<CharT> input);
        inline CommandContext<CharT, S>& WithSource(S source);
        inline CommandContext<CharT, S>& WithArgument(std::basic_string_view<CharT> name, std::shared_ptr<IParsedArgument<CharT, S>> argument);
        inline CommandContext<CharT, S>& WithCommand(Command<CharT, S> command);
        inline CommandContext<CharT, S>& WithNode(CommandNode<CharT, S>* node, StringRange range);
        inline CommandContext<CharT, S>& WithChildContext(CommandContext<CharT, S> childContext);

        inline void Reset()
        {
            detail::CommandContextInternal<CharT, S>& ctx = *context;
            input = {};
            ctx.arguments.clear();
            ctx.command = nullptr;
            ctx.nodes.clear();
            ctx.parent = nullptr;
            ctx.child = {};
            ctx.modifier = nullptr;
            ctx.forks = false;
        }
        inline void Reset(S src, CommandNode<CharT, S>* root)
        {
            Reset();
            source.~S();
            ::new (&source)S(std::move(src));
            detail::CommandContextInternal<CharT, S>& ctx = *context;
            ctx.rootNode = root;
        }
    public:
        inline void Reset(S source, CommandNode<CharT, S>* root, size_t start)
        {
            Reset(source, root);
            detail::CommandContextInternal<CharT, S>& ctx = *context;
            ctx.range = StringRange::At(start);
        }
        inline void Reset(S source, CommandNode<CharT, S>* root, StringRange range)
        {
            Reset(source, root);
            detail::CommandContextInternal<CharT, S>& ctx = *context;
            ctx.range = std::move(range);
        }
    protected:
        SuggestionContext<CharT, S> FindSuggestionContext(size_t cursor);

        void Merge(CommandContext<CharT, S> other);
    private:
        friend class CommandDispatcher<CharT, S>;
        friend class LiteralCommandNode<CharT, S>;
        friend class CommandNode<CharT, S>;
        template<typename, typename, typename>
        friend class ArgumentCommandNode;

        S source;
        std::basic_string_view<CharT> input = {};
        std::shared_ptr<detail::CommandContextInternal<CharT, S>> context = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandContext);

    namespace detail
    {
        template<typename CharT, typename S>
        class CommandContextInternal
        {
        public:
            CommandContextInternal(CommandNode<CharT, S>* root, size_t start) : rootNode(root), range(StringRange::At(start)) {}
            CommandContextInternal(CommandNode<CharT, S>* root, StringRange range) : rootNode(root), range(std::move(range)) {}
        protected:
            friend class CommandContext<CharT, S>;

            std::map<std::basic_string<CharT>, std::shared_ptr<IParsedArgument<CharT, S>>, std::less<>> arguments;
            Command<CharT, S> command = nullptr;
            CommandNode<CharT, S>* rootNode = nullptr;
            std::vector<ParsedCommandNode<CharT, S>> nodes;
            StringRange range;
            CommandContext<CharT, S>* parent = nullptr;
            std::optional<CommandContext<CharT, S>> child = {};
            RedirectModifier<CharT, S> modifier = nullptr;
            bool forks = false;
        };
        BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandContextInternal);
    }

    template<typename CharT, typename S>
    template<typename ArgType>
    typename ArgType::type CommandContext<CharT, S>::GetArgument(std::basic_string_view<CharT> name)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "No such argument '") << std::basic_string<CharT>(name) << BRIGADIER_LITERAL(CharT, "' exists on this command");
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<CharT, ArgType>())) {
            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Argument '") << std::basic_string<CharT>(name) << BRIGADIER_LITERAL(CharT, "' has been acquired using wrong type");
        }

        return ((ParsedArgument<CharT, S, ArgType>*)parsed.get())->GetResult();
    }

    template<typename CharT, typename S>
    template<typename ArgType>
    std::optional<typename ArgType::type> CommandContext<CharT, S>::GetArgumentOpt(std::basic_string_view<CharT> name)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            return std::nullopt;
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<CharT, ArgType>())) {
            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Argument '") << std::basic_string<CharT>(name) << BRIGADIER_LITERAL(CharT, "' has been acquired using wrong type");
        }

        return ((ParsedArgument<CharT, S, ArgType>*)parsed.get())->GetResult();
    }

    template<typename CharT, typename S>
    template<typename ArgType>
    typename ArgType::type CommandContext<CharT, S>::GetArgumentOr(std::basic_string_view<CharT> name, typename ArgType::type default_value)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            return std::move(default_value);
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<CharT, ArgType>())) {
            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Argument '") << std::basic_string<CharT>(name) << BRIGADIER_LITERAL(CharT, "' has been acquired using wrong type");
        }

        return ((ParsedArgument<CharT, S, ArgType>*)parsed.get())->GetResult();
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S> CommandContext<CharT, S>::GetFor(S source) const
    {
        CommandContext<CharT, S> result = *this;
        result.source = std::move(source);
        result.input = input;
        return result;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>* CommandContext<CharT, S>::GetChild() const
    {
        if (context->child.has_value())
            return &context->child.value();
        else return nullptr;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>* CommandContext<CharT, S>::GetParent() const
    {
        return context->parent;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>* CommandContext<CharT, S>::GetLastParent() const
    {
        CommandContext<CharT, S>* result = this;
        while (result->GetParent() != nullptr) {
            result = result->GetParent();
        }
        return result;
    }

    template<typename CharT, typename S>
    inline detail::CommandContextInternal<CharT, S>* CommandContext<CharT, S>::GetInternalContext() const
    {
        return context.get();
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>* CommandContext<CharT, S>::GetLastChild() const
    {
        CommandContext<CharT, S>* result = this;
        while (result->GetChild() != nullptr) {
            result = result->GetChild();
        }
        return result;
    }

    template<typename CharT, typename S>
    inline Command<CharT, S> CommandContext<CharT, S>::GetCommand() const
    {
        return context->command;
    }

    template<typename CharT, typename S>
    inline S& CommandContext<CharT, S>::GetSource()
    {
        return source;
    }

    template<typename CharT, typename S>
    inline S const& CommandContext<CharT, S>::GetSource() const
    {
        return source;
    }

    template<typename CharT, typename S>
    inline RedirectModifier<CharT, S> CommandContext<CharT, S>::GetRedirectModifier() const
    {
        return context->modifier;
    }

    template<typename CharT, typename S>
    inline StringRange CommandContext<CharT, S>::GetRange() const
    {
        return context->range;
    }

    template<typename CharT, typename S>
    inline std::basic_string_view<CharT> CommandContext<CharT, S>::GetInput() const
    {
        return context->input;
    }

    template<typename CharT, typename S>
    inline CommandNode<CharT, S>* CommandContext<CharT, S>::GetRootNode() const
    {
        return context->rootNode;
    }

    template<typename CharT, typename S>
    inline std::vector<ParsedCommandNode<CharT, S>>& CommandContext<CharT, S>::GetNodes() const
    {
        return context->nodes;
    }

    template<typename CharT, typename S>
    inline bool CommandContext<CharT, S>::HasNodes() const
    {
        return context->nodes.size() > 0;
    }

    template<typename CharT, typename S>
    inline bool CommandContext<CharT, S>::IsForked() const
    {
        return context->forks;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>::~CommandContext()
    {
        if (context)
        {
            if (context->child.has_value())
            {
                context->child->context->parent = nullptr;
            }
        }
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithInput(std::basic_string_view<CharT> input)
    {
        this->input = std::move(input);
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithSource(S source)
    {
        context->source = std::move(source);
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithArgument(std::basic_string_view<CharT> name, std::shared_ptr<IParsedArgument<CharT, S>> argument)
    {
        context->arguments.emplace(name, std::move(argument));
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithCommand(Command<CharT, S> command)
    {
        context->command = command;
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithNode(CommandNode<CharT, S>* node, StringRange range)
    {
        if (node)
        {
            context->nodes.emplace_back(node, range);
            context->range = StringRange::Encompassing(context->range, range);
            context->modifier = node->GetRedirectModifier();
            context->forks = node->IsFork();
        }
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithChildContext(CommandContext<CharT, S> childContext)
    {
        context->child = childContext;
        context->child->context->parent = this;
        return *this;
    }

    template<typename CharT, typename S>
    SuggestionContext<CharT, S> CommandContext<CharT, S>::FindSuggestionContext(size_t cursor)
    {
        auto& ctx = context;
        if (ctx->range.GetStart() <= cursor) {
            if (ctx->range.GetEnd() < cursor) {
                if (ctx->child.has_value()) {
                    return ctx->child->FindSuggestionContext(cursor);
                }
                else if (HasNodes()) {
                    auto& last = ctx->nodes.back();
                    return SuggestionContext<CharT, S>(last.GetNode(), last.GetRange().GetEnd() + 1);
                }
                else {
                    return SuggestionContext<CharT, S>(ctx->rootNode, ctx->range.GetStart());
                }
            }
            else {
                auto prev = ctx->rootNode;
                for (auto& node : ctx->nodes) {
                    auto nodeRange = node.GetRange();
                    if (nodeRange.GetStart() <= cursor && cursor <= nodeRange.GetEnd()) {
                        return SuggestionContext<CharT, S>(prev, nodeRange.GetStart());
                    }
                    prev = node.GetNode();
                }
                if (prev == nullptr) {
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Can't find node before cursor");
                }
                return SuggestionContext<CharT, S>(prev, ctx->range.GetStart());
            }
        }
        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Can't find node before cursor");
    }

    template<typename CharT, typename S>
    void CommandContext<CharT, S>::Merge(CommandContext<CharT, S> other)
    {
        detail::CommandContextInternal<CharT, S>* ctx = other.GetInternalContext();
        for (auto& arg : ctx->arguments)
            context->arguments.emplace(std::move(arg));
        context->command = std::move(ctx->command);
        source = std::move(other.source);
        context->nodes.reserve(context->nodes.size() + ctx->nodes.size());
        for (auto& node : ctx->nodes)
        {
            context->range = StringRange::Encompassing(context->range, ctx->range);
            context->modifier = node.GetNode()->GetRedirectModifier();
            context->forks = node.GetNode()->IsFork();

            context->nodes.emplace_back(std::move(node));
        }
        if (ctx->child.has_value())
        {
            ctx->child->context->parent = this;
            if (context->child.has_value())
            {
                context->child->Merge(std::move(*ctx->child));
            }
            else context->child = std::move(ctx->child);
        }
    }
}
#pragma once

#ifdef __has_include
#  if __has_include("nameof.hpp")
#    include "nameof.hpp"
#    define HAS_NAMEOF
#  endif
#  if __has_include("magic_enum.hpp")
#    include "magic_enum.hpp"
#    define HAS_MAGICENUM
#  endif
#endif

namespace brigadier
{
    template<typename T>
    class ArgumentType
    {
    public:
        using type = T;
    public:
        template<typename CharT>
        T Parse(StringReader<CharT>& reader)
        {
            return reader.template ReadValue<T>();
        }

        template<typename CharT, typename S>
        std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder)
        {
            return Suggestions<CharT>::Empty();
        }

        template<typename CharT>
        static constexpr std::basic_string_view<CharT> GetTypeName()
        {
#ifdef HAS_NAMEOF
            return NAMEOF_TYPE(T).substr(NAMEOF_TYPE(T).find(NAMEOF_SHORT_TYPE(T))); // Short type + template list
#else
            return BRIGADIER_LITERAL(CharT, "");
#endif
        }

        template<typename CharT>
        static inline std::vector<std::basic_string_view<CharT>> GetExamples()
        {
            return {};
        }
    };
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(ArgumentType, Type);

    enum class StringArgType {
        SINGLE_WORD,
        QUOTABLE_PHRASE,
        GREEDY_PHRASE,
    };

    template<typename CharT, StringArgType str_type>
    class StringArgumentType : public ArgumentType<std::basic_string<CharT>>
    {
    public:
        StringArgumentType() {};

        StringArgType GetType()
        {
            return str_type;
        }

        template<typename = void>
        std::basic_string<CharT> Parse(StringReader<CharT>& reader)
        {
            if (str_type == StringArgType::GREEDY_PHRASE)
            {
                std::basic_string<CharT> text(reader.GetRemaining());
                reader.SetCursor(reader.GetTotalLength());
                return text;
            }
            else if (str_type == StringArgType::SINGLE_WORD)
            {
                return std::basic_string<CharT>(reader.ReadUnquotedString());
            }
            else
            {
                return reader.ReadString();
            }
        }

        template<typename = void>
        static constexpr std::basic_string_view<CharT> GetTypeName()
        {
            if constexpr (str_type == StringArgType::GREEDY_PHRASE)
            {
                return BRIGADIER_LITERAL(CharT, "words");
            }
            else if constexpr (str_type == StringArgType::SINGLE_WORD)
            {
                return BRIGADIER_LITERAL(CharT, "word");
            }
            else
            {
                return BRIGADIER_LITERAL(CharT, "string");
            }
        }

        template<typename = void>
        static inline std::vector<std::basic_string_view<CharT>> GetExamples()
        {
            if constexpr (str_type == StringArgType::GREEDY_PHRASE)
            {
                return {
                    BRIGADIER_LITERAL(CharT, "word"),
                    BRIGADIER_LITERAL(CharT, "words with spaces"),
                    BRIGADIER_LITERAL(CharT, "\"and symbols\""),
                };
            }
            else if constexpr (str_type == StringArgType::SINGLE_WORD)
            {
                return {
                    BRIGADIER_LITERAL(CharT, "word"),
                    BRIGADIER_LITERAL(CharT, "words_with_underscores"),
                };
            }
            else
            {
                return {
                    BRIGADIER_LITERAL(CharT, "\"quoted phrase\""),
                    BRIGADIER_LITERAL(CharT, "word"),
                    BRIGADIER_LITERAL(CharT, "\"\""),
                };
            }
        }

        template<typename = void>
        static std::basic_string<CharT> EscapeIfRequired(std::basic_string_view<CharT> input)
        {
            for (auto c : input) {
                if (!StringReader<CharT>::IsAllowedInUnquotedString(c)) {
                    return Escape(std::move(input));
                }
            }
            return std::basic_string<CharT>(input);
        }

        template<typename = void>
        static std::basic_string<CharT> Escape(std::basic_string_view<CharT> input)
        {
            std::basic_string<CharT> result;
            result.reserve(input.length());

            result += CharT('\"');
            for (auto c : input) {
                if (c == CharT('\\') || c == CharT('\"')) {
                    result += CharT('\\');
                }
                result += c;
            }
            result += CharT('\"');

            return result;
        }
    };
    BRIGADIER_REGISTER_ARGTYPE_SPEC_CHAR(StringArgumentType, Word, StringArgType::SINGLE_WORD);
    BRIGADIER_REGISTER_ARGTYPE_SPEC_CHAR(StringArgumentType, String, StringArgType::QUOTABLE_PHRASE);
    BRIGADIER_REGISTER_ARGTYPE_SPEC_CHAR(StringArgumentType, GreedyString, StringArgType::GREEDY_PHRASE);
    BRIGADIER_SPECIALIZE_BASIC(Word);
    BRIGADIER_SPECIALIZE_BASIC(String);
    BRIGADIER_SPECIALIZE_BASIC(GreedyString);

    class BoolArgumentType : public ArgumentType<bool>
    {
    public:
        template<typename CharT, typename S>
        std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder)
        {
            builder.AutoSuggestLowerCase(std::initializer_list({ BRIGADIER_LITERAL(CharT, "true"), BRIGADIER_LITERAL(CharT, "false") }));
            return builder.BuildFuture();
        }

        template<typename CharT>
        static constexpr std::basic_string_view<CharT> GetTypeName()
        {
            return BRIGADIER_LITERAL(CharT, "bool");
        }

        template<typename CharT>
        static inline std::vector<std::basic_string_view<CharT>> GetExamples()
        {
            return { 
                BRIGADIER_LITERAL(CharT, "true"),
                BRIGADIER_LITERAL(CharT, "false"),
            };
        }
    };
    BRIGADIER_REGISTER_ARGTYPE(BoolArgumentType, Bool);

    template<typename CharT>
    class CharArgumentType : public ArgumentType<CharT>
    {
    public:
        template<typename = void>
        CharT Parse(StringReader<CharT>& reader)
        {
            if (reader.CanRead())
                return reader.Read();
            else throw exceptions::ReaderExpectedValue(reader);
        }

        template<typename = void>
        static constexpr std::basic_string_view<CharT> GetTypeName()
        {
            return BRIGADIER_LITERAL(CharT, "char");
        }

        template<typename = void>
        static inline std::vector<std::basic_string_view<CharT>> GetExamples()
        {
            return { 
                BRIGADIER_LITERAL(CharT, "c"),
                BRIGADIER_LITERAL(CharT, "@"),
                BRIGADIER_LITERAL(CharT, "."),
            };
        }
    };
    BRIGADIER_REGISTER_ARGTYPE_CHAR(CharArgumentType, Char);
    BRIGADIER_SPECIALIZE_BASIC(Char);

    template<typename T>
    class ArithmeticArgumentType : public ArgumentType<T>
    {
        static_assert(std::is_arithmetic_v<T>, "T must be a number");
    public:
        ArithmeticArgumentType(T minimum = std::numeric_limits<T>::lowest(), T maximum = std::numeric_limits<T>::max()) : minimum(minimum), maximum(maximum) {}

        T GetMinimum() {
            return minimum;
        }
        T GetMaximum() {
            return maximum;
        }

        template<typename CharT>
        T Parse(StringReader<CharT>& reader)
        {
            size_t start = reader.GetCursor();
            T result = reader.template ReadValue<T>();
            if (result < minimum) {
                reader.SetCursor(start);
                throw exceptions::ValueTooLow(reader, result, minimum);
            }
            if (result > maximum) {
                reader.SetCursor(start);
                throw exceptions::ValueTooHigh(reader, result, maximum);
            }
            return result;
        }

        template<typename CharT>
        static constexpr std::basic_string_view<CharT> GetTypeName()
        {
            if constexpr (std::is_floating_point_v<T>)
                return BRIGADIER_LITERAL(CharT, "float");
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_signed_v<T>)
                    return BRIGADIER_LITERAL(CharT, "int");
                else
                    return BRIGADIER_LITERAL(CharT, "uint");
            }
            else return ArgumentType<T>::template GetTypeName<CharT>();
        }

        template<typename CharT>
        static inline std::vector<std::basic_string_view<CharT>> GetExamples()
        {
            if constexpr (std::is_floating_point_v<T>)
                return {
                    BRIGADIER_LITERAL(CharT, "0"),
                    BRIGADIER_LITERAL(CharT, "1.2"),
                    BRIGADIER_LITERAL(CharT, ".5"),
                    BRIGADIER_LITERAL(CharT, "-1"),
                    BRIGADIER_LITERAL(CharT, "-.5"),
                    BRIGADIER_LITERAL(CharT, "-1234.56"),
                };
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_signed_v<T>)
                    return {
                        BRIGADIER_LITERAL(CharT, "0"),
                        BRIGADIER_LITERAL(CharT, "123"),
                        BRIGADIER_LITERAL(CharT, "-123"),
                    };
                else
                    return {
                        BRIGADIER_LITERAL(CharT, "0"),
                        BRIGADIER_LITERAL(CharT, "123"),
                    };
            }
            else return {};
        }
    private:
        T minimum;
        T maximum;
    };
    BRIGADIER_REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Float, float);
    BRIGADIER_REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Double, double);
    BRIGADIER_REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Integer, int);
    BRIGADIER_REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Long, long long);
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(ArithmeticArgumentType, Number);

#ifdef HAS_MAGICENUM
    template<typename T>
    class EnumArgumentType : public ArgumentType<T>
    {
        static_assert(std::is_enum_v<T>, "T must be enum");
    public:
        template<typename CharT>
        T Parse(StringReader<CharT>& reader)
        {
            size_t start = reader.GetCursor();
            auto str = reader.ReadString();
            auto result = magic_enum::enum_cast<T>(str);
            if (!result.has_value())
            {
                reader.SetCursor(start);
                throw exceptions::ReaderInvalidValue(reader, str);
            }
            return result.value();
        }

        template<typename CharT, typename S>
        std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder)
        {
            static constexpr auto names = magic_enum::enum_names<T>();
            builder.AutoSuggestLowerCase(names);
            return builder.BuildFuture();
        }

        template<typename CharT>
        static constexpr std::basic_string_view<CharT> GetTypeName()
        {
            return BRIGADIER_LITERAL(CharT, "enum");
        }

        template<typename CharT>
        static inline std::vector<std::basic_string_view<CharT>> GetExamples()
        {
            static constexpr auto names = magic_enum::enum_names<T>();
            return std::vector<std::basic_string_view<CharT>>(names.begin(), names.end());
        }
    };
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(EnumArgumentType, Enum);
#endif
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S, typename T>
    class RequiredArgumentBuilder;

    template<typename CharT, typename S>
    class IArgumentCommandNode : public CommandNode<CharT, S>
    {
    protected:
        IArgumentCommandNode(std::basic_string_view<CharT> name) : name(name) {}
        virtual ~IArgumentCommandNode() = default;
    public:
        virtual std::basic_string<CharT> const& GetName() {
            return name;
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::ArgumentCommandNode; }
    protected:
        virtual std::basic_string_view<CharT> GetSortedKey() {
            return name;
        }
    protected:
        std::basic_string<CharT> name;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(IArgumentCommandNode);

    template<typename CharT, typename S, typename T>
    class ArgumentCommandNode : public IArgumentCommandNode<CharT, S>
    {
    private:
        static constexpr std::basic_string_view<CharT> USAGE_ARGUMENT_OPEN = BRIGADIER_LITERAL(CharT, "<");
        static constexpr std::basic_string_view<CharT> USAGE_ARGUMENT_CLOSE = BRIGADIER_LITERAL(CharT, ">");
    public:
        template<typename... Args>
        ArgumentCommandNode(std::basic_string_view<CharT> name, Args&&... args)
            : IArgumentCommandNode<CharT, S>(name)
            , type(std::forward<Args>(args)...)
        {}
        virtual ~ArgumentCommandNode() = default;
    public:
        inline SuggestionProvider<CharT, S> const& GetCustomSuggestions() const {
            return customSuggestions;
        }

        inline T const& GetType() {
            return type;
        }
    public:
        virtual std::basic_string<CharT> GetUsageText() {
            std::basic_string<CharT> ret;
            constexpr auto typeName = T::template GetTypeName<CharT>();
            ret.reserve(this->name.size() + USAGE_ARGUMENT_OPEN.size() + USAGE_ARGUMENT_CLOSE.size() + typeName.size() > 0 ? typeName.size() + 2 : 0);
            ret = USAGE_ARGUMENT_OPEN;
            if constexpr (typeName.size() > 0)
            {
                ret += typeName;
                ret += BRIGADIER_LITERAL(CharT, ": ");
            }
            ret += this->name;
            ret += USAGE_ARGUMENT_CLOSE;
            return ret;
        }
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() {
            return type.template GetExamples<CharT>();
        }
        virtual void Parse(StringReader<CharT>& reader, CommandContext<CharT, S>& contextBuilder) {
            size_t start = reader.GetCursor();
            using Type = typename T::type;
            Type result = type.Parse(reader);
            std::shared_ptr<ParsedArgument<CharT, S, T>> parsed = std::make_shared<ParsedArgument<CharT, S, T>>(start, reader.GetCursor(), std::move(result));

            contextBuilder.WithArgument(this->name, parsed);
            contextBuilder.WithNode(this, parsed->GetRange());
        }
        virtual std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder)
        {
            if (customSuggestions == nullptr) {
                return type.template ListSuggestions<CharT, S>(context, builder);
            }
            else {
                return customSuggestions(context, builder);
            }
        }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) {
            try {
                StringReader<CharT> reader = StringReader<CharT>(input);
                type.Parse(reader);
                return !reader.CanRead() || reader.Peek() == ' ';
            }
            catch (CommandSyntaxException<CharT> const&) {
                return false;
            }
        }
    private:
        friend class RequiredArgumentBuilder<CharT, S, T>;
        T type;
        SuggestionProvider<CharT, S> customSuggestions = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ArgumentCommandNode);
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class MultiArgumentBuilder
    {
    public:
        using B = MultiArgumentBuilder<CharT, S>;
    public:
        MultiArgumentBuilder(std::vector<std::shared_ptr<CommandNode<CharT, S>>> nodes, int master = -1) : nodes(std::move(nodes)), master(master) {}
        MultiArgumentBuilder(MultiArgumentBuilder const&) = delete; // no copying. Use reference or GetThis().

        inline B* GetThis() { return this; }

        template<template<typename...> typename Next, template<typename> typename Type, typename... Args>
        auto Then(Args&&... args) {
            return Then<Next, Type<CharT>, Args...>(std::forward<Args>(args)...);
        }
        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto Then(Args&&... args)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
                }
            }

            if constexpr (std::is_same_v<Type, void>) {
                using next_node = typename Next<CharT, S>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto new_node = std::make_shared<next_node>(std::move(node_builder));
                for (auto& node : nodes) {
                    node->AddChild(new_node);
                }
                return Next<CharT, S>(std::move(new_node));
            }
            else {
                using next_node = typename Next<CharT, S, Type>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto new_node = std::make_shared<next_node>(std::move(node_builder));
                for (auto& node : nodes) {
                    node->AddChild(new_node);
                }
                return Next<CharT, S, Type>(std::move(new_node));
            }
        }

        auto Then(std::shared_ptr<LiteralCommandNode<CharT, S>> argument)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
                }
            }
            for (auto& node : nodes) {
                if (argument != nullptr) {
                    node->AddChild(argument);
                }
            }
            return GetBuilder(std::move(argument));
        }

        template<typename T>
        auto Then(std::shared_ptr<ArgumentCommandNode<CharT, S, T>> argument)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
                }
            }
            for (auto& node : nodes) {
                if (argument != nullptr) {
                    node->AddChild(argument);
                }
            }
            return GetBuilder(std::move(argument));
        }

        B& Executes(Command<CharT, S> command, bool only_master = true)
        {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    node->command = command;
                }
            }
            return *GetThis();
        }

        B& Requires(Predicate<S&> requirement, bool only_master = true)
        {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    node->requirement = requirement;
                }
            }
            return *GetThis();
        }

        inline auto Redirect(std::shared_ptr<CommandNode<CharT, S>> target, bool only_master = true)
        {
            return Forward(std::move(target), nullptr, false, only_master);
        }

        inline auto Redirect(std::shared_ptr<CommandNode<CharT, S>> target, SingleRedirectModifier<CharT, S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, false, only_master);
        }

        inline auto Fork(std::shared_ptr<CommandNode<CharT, S>> target, SingleRedirectModifier<CharT, S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, true, only_master);
        }

        inline auto Fork(std::shared_ptr<CommandNode<CharT, S>> target, RedirectModifier<CharT, S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier, true, only_master);
        }

        void Forward(std::shared_ptr<CommandNode<CharT, S>> target, RedirectModifier<CharT, S> modifier, bool fork, bool only_master = true)
        {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    if (node->GetChildren().size() > 0)
                    {
                        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot forward a node with children");
                    }
                }
            }
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    node->redirect = std::move(target);
                    node->modifier = modifier;
                    node->forks = fork;
                }
            }
        }
    protected:
        std::vector<std::shared_ptr<CommandNode<CharT, S>>> nodes;
        int master = -1;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(MultiArgumentBuilder);

    // multi builder
    template<typename CharT, typename S>
    inline MultiArgumentBuilder<CharT, S> GetBuilder(std::vector<std::shared_ptr<CommandNode<CharT, S>>> nodes, int master = -1)
    {
        return MultiArgumentBuilder<CharT, S>(std::move(nodes), master);
    }
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S, typename B, typename NodeType>
    class ArgumentBuilder
    {
    public:
        using node_type = NodeType;
    public:
        ArgumentBuilder(std::shared_ptr<node_type> node)
        {
            if (node) this->node = std::move(node);
            else throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot build empty node");
        }
        ArgumentBuilder(ArgumentBuilder const&) = delete; // no copying. Use reference or GetThis().

        inline B* GetThis() { return static_cast<B*>(this); }

        inline std::shared_ptr<node_type> GetNode() const { return node; }
        inline std::shared_ptr<CommandNode<CharT, S>> GetCommandNode() const { return std::static_pointer_cast<CommandNode<CharT, S>>(node); }
        inline operator std::shared_ptr<node_type>() const { return GetNode(); }
        inline operator std::shared_ptr<CommandNode<CharT, S>>() const { return GetCommandNode(); }

        template<template<typename...> typename Next, template<typename> typename Type, typename... Args>
        auto Then(Args&&... args) {
            return Then<Next, Type<CharT>, Args...>(std::forward<Args>(args)...);
        }
        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto Then(Args&&... args)
        {
            if (node->redirect != nullptr) {
                throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
            }

            if constexpr (std::is_same_v<Type, void>) {
                using next_node = typename Next<CharT, S>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = node->children.find(name);
                if (arg == node->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    node->AddChild(new_node);
                    return Next<CharT, S>(std::move(new_node));
                }
                else {
                    auto& arg_ptr = arg->second;
                    if (arg_ptr) {
                        if (node_builder.GetNodeType() != arg_ptr->GetNodeType()) {
                            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");
                        }
                    }
                    return Next<CharT, S>(std::static_pointer_cast<next_node>(arg_ptr));
                }
            }
            else
            {
                using next_node = typename Next<CharT, S, Type>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = node->children.find(name);
                if (arg == node->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    node->AddChild(new_node);
                    return Next<CharT, S, Type>(std::move(new_node));
                }
                else {
                    auto& arg_ptr = arg->second;
                    if (arg_ptr) {
                        if (node_builder.GetNodeType() != arg_ptr->GetNodeType()) {
                            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");
                        }
                    }
                    return Next<CharT, S, Type>(std::static_pointer_cast<next_node>(arg_ptr));
                }
            }
        }

        template<template<typename...> typename Next, template<typename> typename Type, typename... Args>
        auto ThenOptional(Args&&... args) {
            return ThenOptional<Next, Type<CharT>, Args...>(std::forward<Args>(args)...);
        }
        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto ThenOptional(Args&&... args)
        {
            auto opt = Then<Next, Type, Args...>(std::forward<Args>(args)...);
            return MultiArgumentBuilder<CharT, S>({ opt.GetCommandNode(), GetCommandNode() }, 0);
        }

        //auto Then(std::shared_ptr<LiteralCommandNode<CharT, S>> argument)
        //{
        //    if (node->redirect != nullptr) {
        //        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
        //    }
        //    if (argument != nullptr) {
        //        node->AddChild(std::move(argument));
        //    }
        //    return GetBuilder<CharT, S>(std::move(node->GetChild(argument)));
        //}

        //template<typename T>
        //auto Then(std::shared_ptr<ArgumentCommandNode<CharT, S, T>> argument)
        //{
        //    if (node->redirect != nullptr) {
        //        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
        //    }
        //    if (argument != nullptr) {
        //        node->AddChild(std::move(argument));
        //    }
        //    return GetBuilder<CharT, S>(std::move(node->GetChild(argument)));
        //}

        B& Executes(Command<CharT, S> command)
        {
            node->command = command;
            return *GetThis();
        }

        B& Requires(Predicate<S&> requirement)
        {
            node->requirement = requirement;
            return *GetThis();
        }

        inline auto Redirect(std::shared_ptr<CommandNode<CharT, S>> target)
        {
            return Forward(std::move(target), nullptr, false);
        }

        inline auto Redirect(std::shared_ptr<CommandNode<CharT, S>> target, SingleRedirectModifier<CharT, S> modifier)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, false);
        }

        inline auto Fork(std::shared_ptr<CommandNode<CharT, S>> target, SingleRedirectModifier<CharT, S> modifier)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, true);
        }

        inline auto Fork(std::shared_ptr<CommandNode<CharT, S>> target, RedirectModifier<CharT, S> modifier)
        {
            return Forward(std::move(target), modifier, true);
        }

        void Forward(std::shared_ptr<CommandNode<CharT, S>> target, RedirectModifier<CharT, S> modifier, bool fork)
        {
            if (node->GetChildren().size() > 0)
            {
                throw RuntimeError<CharT>() << "Cannot forward a node with children";
            }
            node->redirect = std::move(target);
            node->modifier = modifier;
            node->forks = fork;
        }
    protected:
        template<typename, typename, typename>
        friend class RequiredArgumentBuilder;

        std::shared_ptr<node_type> node;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ArgumentBuilder);
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class LiteralArgumentBuilder : public ArgumentBuilder<CharT, S, LiteralArgumentBuilder<CharT, S>, LiteralCommandNode<CharT, S>>
    {
    public:
        LiteralArgumentBuilder(std::shared_ptr<LiteralCommandNode<CharT, S>> node) : ArgumentBuilder<CharT, S, LiteralArgumentBuilder<CharT, S>, LiteralCommandNode<CharT, S>>(std::move(node)) {}
    };
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(LiteralArgumentBuilder, Literal);
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(LiteralArgumentBuilder);

    // single builder
    template<typename CharT, typename S>
    inline LiteralArgumentBuilder<CharT, S> GetBuilder(std::shared_ptr<LiteralCommandNode<CharT, S>> node)
    {
        return LiteralArgumentBuilder<CharT, S>(std::move(node));
    }

    // new single builder
    template<typename CharT, typename S, typename... Args>
    inline LiteralArgumentBuilder<CharT, S> MakeLiteral(Args&&... args)
    {
        return LiteralArgumentBuilder<CharT, S>(std::make_shared<LiteralCommandNode<CharT, S>>(std::forward<Args>(args)...));
    }
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S, typename T>
    class RequiredArgumentBuilder : public ArgumentBuilder<CharT, S, RequiredArgumentBuilder<CharT, S, T>, ArgumentCommandNode<CharT, S, T>>
    {
    public:
        RequiredArgumentBuilder(std::shared_ptr<ArgumentCommandNode<CharT, S, T>> node) : ArgumentBuilder<CharT, S, RequiredArgumentBuilder<CharT, S, T>, ArgumentCommandNode<CharT, S, T>>(std::move(node)) {}

        RequiredArgumentBuilder<CharT, S, T>& Suggests(SuggestionProvider<CharT, S> provider)
        {
            this->node->suggestionsProvider = provider;
            return *this;
        }
    };
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(RequiredArgumentBuilder, Argument);
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(RequiredArgumentBuilder);

    // single builder
    template<typename CharT, typename S, template<typename...> typename Spec, typename... Types>
    inline RequiredArgumentBuilder<CharT, S, Spec<CharT, S, Types...>> GetBuilder(std::shared_ptr<ArgumentCommandNode<CharT, S, Spec<CharT, S, Types...>>> node)
    {
        return RequiredArgumentBuilder<CharT, S, Spec<CharT, S, Types...>>(std::move(node));
    }

    // new single builder
    template<typename CharT, typename S, template<typename...> typename Spec, typename Type, typename... Args>
    inline RequiredArgumentBuilder<CharT, S, Spec<CharT, S, Type>> MakeArgument(Args&&... args)
    {
        return RequiredArgumentBuilder<CharT, S, Spec<CharT, S, Type>>(std::make_shared<ArgumentCommandNode<CharT, S, Spec<CharT, S, Type>>>(std::forward<Args>(args)...));
    }
}
#pragma once

namespace brigadier
{
    template<typename CharT, typename S>
    class CommandDispatcher;

    template<typename CharT, typename S>
    class ParseResults
    {
    public:
        ParseResults(CommandContext<CharT, S> context, StringReader<CharT> reader, std::map<CommandNode<CharT, S>*, CommandSyntaxException<CharT>> exceptions)
            : context(std::move(context))
            , reader(std::move(reader))
            , exceptions(std::move(exceptions))
        {}
        ParseResults(CommandContext<CharT, S> context, StringReader<CharT> reader)
            : context(std::move(context))
            , reader(std::move(reader))
        {}

        ParseResults(CommandContext<CharT, S> context) : ParseResults(std::move(context), StringReader<CharT>()) {}
    public:
        inline CommandContext<CharT, S> const& GetContext() const { return context; }
        inline StringReader<CharT> const& GetReader()  const { return reader; }
        inline std::map<CommandNode<CharT, S>*, CommandSyntaxException<CharT>> const& GetExceptions() const { return exceptions; }

        inline bool IsBetterThan(ParseResults<CharT, S> const& other) const
        {
            if (!GetReader().CanRead() && other.GetReader().CanRead()) {
                return true;
            }
            if (GetReader().CanRead() && !other.GetReader().CanRead()) {
                return false;
            }
            if (GetExceptions().empty() && !other.GetExceptions().empty()) {
                return true;
            }
            if (!GetExceptions().empty() && other.GetExceptions().empty()) {
                return false;
            }
            return false;
        }

        inline void Reset(StringReader<CharT> new_reader)
        {
            exceptions.clear();
            reader = std::move(new_reader);
        }
        inline void Reset(S source, CommandNode<CharT, S>* root, size_t start, StringReader<CharT> reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, start);
        }
        inline void Reset(S source, CommandNode<CharT, S>* root, StringRange range, StringReader<CharT> reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, std::move(range));
        }
    private:
        template<typename, typename>
        friend class CommandDispatcher;

        CommandContext<CharT, S> context;
        std::map<CommandNode<CharT, S>*, CommandSyntaxException<CharT>> exceptions;
        StringReader<CharT> reader;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParseResults);
}
#pragma once

namespace brigadier
{
    /**
    The core command dispatcher, for registering, parsing, and executing commands.
    
    \param <S> a custom "source" type, such as a user or originator of a command
    */
    template<typename CharT, typename S>
    class CommandDispatcher
    {
    public:
        /**
        The string required to separate individual arguments in an input string
        */
        static constexpr std::basic_string_view<CharT> ARGUMENT_SEPARATOR = BRIGADIER_LITERAL(CharT, " ");

        /**
        The char required to separate individual arguments in an input string
        */
        static constexpr CharT ARGUMENT_SEPARATOR_CHAR = CharT(' ');
    private:
        static constexpr std::basic_string_view<CharT> USAGE_OPTIONAL_OPEN = BRIGADIER_LITERAL(CharT, "[");
        static constexpr std::basic_string_view<CharT> USAGE_OPTIONAL_CLOSE = BRIGADIER_LITERAL(CharT, "]");
        static constexpr std::basic_string_view<CharT> USAGE_REQUIRED_OPEN = BRIGADIER_LITERAL(CharT, "(");
        static constexpr std::basic_string_view<CharT> USAGE_REQUIRED_CLOSE = BRIGADIER_LITERAL(CharT, ")");
        static constexpr std::basic_string_view<CharT> USAGE_OR = BRIGADIER_LITERAL(CharT, "|");
    public:
        /**
        Create a new CommandDispatcher with the specified root node.
        
        This is often useful to copy existing or pre-defined command trees.
        
        \param root the existing RootCommandNode to use as the basis for this tree
        */
        CommandDispatcher(RootCommandNode<CharT, S>* root) : root(std::make_shared<RootCommandNode<CharT, S>>(*root)) {}

        /**
        Creates a new CommandDispatcher with an empty command tree.
        */
        CommandDispatcher() : root(std::make_shared<RootCommandNode<CharT, S>>()) {}

        /**
        Utility method for registering new commands.

        \param args these arguments are forwarded to builder to node constructor. The first param is always node name.
        \return the builder with node added to this tree
        */
        template<template<typename...> typename Next, template<typename> typename Type, typename... Args>
        auto Register(Args&&... args) {
            return Register<Next, Type<CharT>, Args...>(std::forward<Args>(args)...);
        }
        template<template<typename...> typename Next = Literal, typename Type = void, typename... Args>
        auto Register(Args&&... args)
        {
            if constexpr (std::is_same_v<Type, void>) {
                using next_node = typename Next<CharT, S>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = root->children.find(name);
                if (arg == root->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    root->AddChild(new_node);
                    return Next<CharT, S>(std::move(new_node));
                }
                else {
                    auto& arg_ptr = arg->second;
                    if (arg_ptr) {
                        if (node_builder.GetNodeType() != arg_ptr->GetNodeType()) {
                            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");
                        }
                    }
                    return Next<CharT, S>(std::static_pointer_cast<next_node>(arg_ptr));
                }
            }
            else
            {
                using next_node = typename Next<CharT, S, Type>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = root->children.find(name);
                if (arg == root->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    root->AddChild(new_node);
                    return Next<CharT, S, Type>(std::move(new_node));
                }
                else {
                    auto& arg_ptr = arg->second;
                    if (arg_ptr) {
                        if (node_builder.GetNodeType() != arg_ptr->GetNodeType()) {
                            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");
                        }
                    }
                    return Next<CharT, S, Type>(std::static_pointer_cast<next_node>(arg_ptr));
                }
            }
        }

        /**
        Sets a callback to be informed of the result of every command.

        \param consumer the new result consumer to be called
        */
        void SetConsumer(ResultConsumer<CharT, S> consumer)
        {
            this->consumer = consumer;
        }

        /**
        Gets the root of this command tree.

        This is often useful as a target of a ArgumentBuilder::Redirect(CommandNode),
        GetAllUsage(CommandNode, Object, boolean) or GetSmartUsage(CommandNode, Object).
        You may also use it to clone the command tree via CommandDispatcher(RootCommandNode).

        \return root of the command tree
        */
        std::shared_ptr<RootCommandNode<CharT, S>> GetRoot() const
        {
            return root;
        }

        /**
        Parses and executes a given command.

        This is a shortcut to first Parse(StringReader, Object) and then Execute(ParseResults).

        It is recommended to parse and execute as separate steps, as parsing is often the most expensive step, and easiest to cache.

        If this command returns a value, then it successfully executed something. If it could not parse the command, or the execution was a failure,
        then an exception will be thrown. Most exceptions will be of type CommandSyntaxException, but it is possible that a RuntimeError
        may bubble up from the result of a command. The meaning behind the returned result is arbitrary, and will depend
        entirely on what command was performed.

        If the command passes through a node that is CommandNode::IsFork() then it will be 'forked'.
        A forked command will not bubble up any CommandSyntaxException's, and the 'result' returned will turn into
        'amount of successful commands executes'.

        After each and any command is ran, a registered callback given to SetConsumer(ResultConsumer)
        will be notified of the result and success of the command. You can use that method to gather more meaningful
        results than this method will return, especially when a command forks.

        \param input a command string to parse and execute
        \param source a custom "source" object, usually representing the originator of this command
        \return a numeric result from a "command" that was performed
        \throws CommandSyntaxException if the command failed to parse or execute
        \throws RuntimeError if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(StringReader, Object)
        */
        int Execute(std::basic_string_view<CharT> input, S source)
        {
            return Execute(StringReader<CharT>(input), std::move(source));
        }

        /**
        Parses and executes a given command.

        This is a shortcut to first Parse(StringReader, Object) and then Execute(ParseResults).

        It is recommended to parse and execute as separate steps, as parsing is often the most expensive step, and easiest to cache.

        If this command returns a value, then it successfully executed something. If it could not parse the command, or the execution was a failure,
        then an exception will be thrown. Most exceptions will be of type CommandSyntaxException, but it is possible that a RuntimeError
        may bubble up from the result of a command. The meaning behind the returned result is arbitrary, and will depend
        entirely on what command was performed.

        If the command passes through a node that is CommandNode::IsFork() then it will be 'forked'.
        A forked command will not bubble up any CommandSyntaxException's, and the 'result' returned will turn into
        'amount of successful commands executes'.

        After each and any command is ran, a registered callback given to SetConsumer(ResultConsumer)
        will be notified of the result and success of the command. You can use that method to gather more meaningful
        results than this method will return, especially when a command forks.
    
        \param input a command string to parse & execute
        \param source a custom "source" object, usually representing the originator of this command
        \return a numeric result from a "command" that was performed
        \throws CommandSyntaxException if the command failed to parse or execute
        \throws RuntimeError if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(String, Object)
        */
        int Execute(StringReader<CharT> input, S source)
        {
            return Execute(Parse(std::move(input), std::move(source)));
        }

        /**
        Executes a given pre-parsed command.

        If this command returns a value, then it successfully executed something. If the execution was a failure,
        then an exception will be thrown.
        Most exceptions will be of type CommandSyntaxException, but it is possible that a RuntimeError
        may bubble up from the result of a command. The meaning behind the returned result is arbitrary, and will depend
        entirely on what command was performed.

        If the command passes through a node that is CommandNode::IsFork() then it will be 'forked'.
        A forked command will not bubble up any CommandSyntaxException<CharT>'s, and the 'result' returned will turn into
        'amount of successful commands executes'.

        After each and any command is ran, a registered callback given to SetConsumer(ResultConsumer)
        will be notified of the result and success of the command. You can use that method to gather more meaningful
        results than this method will return, especially when a command forks.

        \param parse the result of a successful Parse(StringReader, Object)
        \return a numeric result from a "command" that was performed.
        \throws CommandSyntaxException if the command failed to parse or execute
        \throws RuntimeError if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(String, Object)
        \see Execute(StringReader, Object)
        */
        int Execute(ParseResults<CharT, S> const& parse)
        {
            if (parse.GetReader().CanRead()) {
                if (parse.GetExceptions().size() == 1) {
                    throw parse.GetExceptions().begin()->second;
                }
                else if (parse.GetContext().GetRange().IsEmpty()) {
                    throw exceptions::DispatcherUnknownCommand(parse.GetReader());
                }
                else {
                    throw exceptions::DispatcherUnknownArgument(parse.GetReader());
                }
            }

            int result = 0;
            int successfulForks = 0;
            bool forked = false;
            bool foundCommand = false;
            auto command = parse.GetReader().GetString();
            auto original = parse.GetContext();
            original.WithInput(command);
            std::vector<CommandContext<CharT, S>> contexts = { original };
            std::vector<CommandContext<CharT, S>> next;

            while (!contexts.empty()) {
                for (auto& context : contexts) {
                    CommandContext<CharT, S>* child = context.GetChild();
                    if (child != nullptr) {
                        forked |= context.IsForked();
                        if (child->HasNodes()) {
                            foundCommand = true;
                            RedirectModifier<CharT, S> modifier = context.GetRedirectModifier();
                            if (modifier == nullptr) {
                                next.push_back(child->GetFor(context.GetSource()));
                            } else {
                                try {
                                    auto results = modifier(context);
                                    if (!results.empty()) {
                                        for (auto& source : results) {
                                            next.push_back(child->GetFor(std::move(source)));
                                        }
                                    }
                                }
                                catch (CommandSyntaxException<CharT> const&) {
                                    consumer(context, false, 0);
                                    if (!forked) {
                                        throw;
                                    }
                                }
                            }
                        }
                    } else if (context.GetCommand() != nullptr) {
                        foundCommand = true;
                        try {
                            int value = context.GetCommand()(context);
                            result += value;
                            consumer(context, true, value);
                            successfulForks++;
                        }
                        catch (CommandSyntaxException<CharT> const&) {
                            consumer(context, false, 0);
                            if (!forked) {
                                throw;
                            }
                        }
                    }
                }

                contexts = std::move(next);
            }

            if (!foundCommand) {
                consumer(original, false, 0);
                throw exceptions::DispatcherUnknownCommand(parse.GetReader());
            }

            return forked ? successfulForks : result;
        }

        /**
        Parses a given command.

        The result of this method can be cached, and it is advised to do so where appropriate. Parsing is often the
        most expensive step, and this allows you to essentially "precompile" a command if it will be ran often.

        If the command passes through a node that is CommandNode::IsFork() then the resulting context will be marked as 'forked'.
        Forked contexts may contain child contexts, which may be modified by the RedirectModifier attached to the fork.

        Parsing a command can never fail, you will always be provided with a new ParseResults.
        However, that does not mean that it will always parse into a valid command. You should inspect the returned results
        to check for validity. If its ParseResults::GetReader() StringReader<CharT>::CanRead() then it did not finish
        parsing successfully. You can use that position as an indicator to the user where the command stopped being valid.
        You may inspect ParseResults::GetExceptions() if you know the parse failed, as it will explain why it could
        not find any valid commands. It may contain multiple exceptions, one for each "potential node" that it could have visited,
        explaining why it did not go down that node.

        When you eventually call Execute(ParseResults) with the result of this method, the above error checking
        will occur. You only need to inspect it yourself if you wish to handle that yourself.

        \param command a command string to parse
        \param source a custom "source" object, usually representing the originator of this command
        \return the result of parsing this command
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(String, Object)
        */
        ParseResults<CharT, S> Parse(std::basic_string_view<CharT> command, S source)
        {
            StringReader<CharT> reader = StringReader<CharT>(command);
            return Parse(reader, std::move(source));
        }

        /**
        Parses a given command.

        The result of this method can be cached, and it is advised to do so where appropriate. Parsing is often the
        most expensive step, and this allows you to essentially "precompile" a command if it will be ran often.

        If the command passes through a node that is CommandNode::IsFork() then the resulting context will be marked as 'forked'.
        Forked contexts may contain child contexts, which may be modified by the RedirectModifier attached to the fork.

        Parsing a command can never fail, you will always be provided with a new ParseResults.
        However, that does not mean that it will always parse into a valid command. You should inspect the returned results
        to check for validity. If its ParseResults::GetReader() StringReader::CanRead() then it did not finish
        parsing successfully. You can use that position as an indicator to the user where the command stopped being valid.
        You may inspect ParseResults::GetExceptions() if you know the parse failed, as it will explain why it could
        not find any valid commands. It may contain multiple exceptions, one for each "potential node" that it could have visited,
        explaining why it did not go down that node.

        When you eventually call Execute(ParseResults) with the result of this method, the above error checking
        will occur. You only need to inspect it yourself if you wish to handle that yourself.

        \param command a command string to parse
        \param source a custom "source" object, usually representing the originator of this command
        \return the result of parsing this command
        \see Parse(String, Object)
        \see Execute(ParseResults)
        \see Execute(String, Object)
        */
        ParseResults<CharT, S> Parse(StringReader<CharT> command, S source)
        {
            ParseResults<CharT, S> result(CommandContext<CharT, S>(std::move(source), root.get(), command.GetCursor()), std::move(command));
            ParseNodes(root.get(), result);
            return result;
        }

    private:
        void ParseNodes(CommandNode<CharT, S>* node, ParseResults<CharT, S>& result)
        {
            if (!node)
                return;

            S& source = result.context.GetSource();

            std::optional<ParseResults<CharT, S>> ctxs[2] = {};

            bool best = 0;

            size_t cursor = result.reader.GetCursor();

            auto [relevant_nodes, relevant_node_count] = node->GetRelevantNodes(result.reader);

            for (size_t i = 0; i < relevant_node_count; ++i) {
                auto& child = relevant_nodes[i];
                auto& current_result_ctx = ctxs[!best];
                auto& best_potential = ctxs[best];

                if (!child->CanUse(source)) {
                    continue;
                }

                // initialize current context
                if (current_result_ctx.has_value()) {
                    // context already exists so we have to reset it (avoid memory reallocation)
                    current_result_ctx->Reset(source, result.context.GetRootNode(), result.GetContext().GetRange(), result.GetReader());
                }
                else {
                    // create context
                    current_result_ctx.emplace(CommandContext<CharT, S>(source, result.GetContext().GetRootNode(), result.GetContext().GetRange()), result.GetReader());
                }

                auto& current_result = current_result_ctx.value();

                StringReader<CharT>& reader = current_result.reader;
                CommandContext<CharT, S>& context = current_result.context;

                try {
                    try {
                        child->Parse(reader, context);
                    }
                    catch (RuntimeError<CharT> const& ex) {
                        throw exceptions::DispatcherParseException(reader, ex.What());
                    }
                    if (reader.CanRead() && reader.Peek() != ARGUMENT_SEPARATOR_CHAR) {
                        throw exceptions::DispatcherExpectedArgumentSeparator(reader);
                    }
                }
                catch (CommandSyntaxException<CharT> const& ex) {
                    result.exceptions.emplace(child.get(), std::move(ex));
                    reader.SetCursor(cursor);
                    continue;
                }

                context.WithCommand(child->GetCommand());

                if (reader.CanRead(child->GetRedirect() == nullptr ? 2 : 1)) {
                    reader.Skip();
                    if (child->GetRedirect() != nullptr) {
                        ParseResults<CharT, S> child_result(CommandContext<CharT, S>(source, child->GetRedirect().get(), reader.GetCursor()), reader);
                        ParseNodes(child->GetRedirect().get(), child_result);
                        result.context.Merge(std::move(context));
                        result.context.WithChildContext(std::move(child_result.context));
                        result.exceptions = std::move(child_result.exceptions);
                        result.reader = std::move(child_result.reader);
                        return;
                    }
                    else {
                        ParseNodes(child.get(), current_result);
                    }
                }

                if (best_potential.has_value()) {
                    if (current_result.IsBetterThan(*best_potential)) {
                        best = !best;
                    }
                }
                else {
                    best = !best;
                }
            }

            auto& best_potential = ctxs[best];
            if (best_potential.has_value()) {
                result.exceptions.clear();
                result.reader = std::move(best_potential->reader);
                result.context.Merge(std::move(best_potential->context));
            }
        }

    public:
        /**
        Gets all possible executable commands following the given node.

        You may use GetRoot() as a target to get all usage data for the entire command tree.

        The returned syntax will be in "simple" form: `<param>` and `literal`. "Optional" nodes will be
        listed as multiple entries: the parent node, and the child nodes.
        For example, a required literal "foo" followed by an optional param "int" will be two nodes:
          foo
          foo <int>

        The path to the specified node will NOT be prepended to the output, as there can theoretically be many
        ways to reach a given node. It will only give you paths relative to the specified node, not absolute from root.

        \param node target node to get child usage strings for
        \param source a custom "source" object, usually representing the originator of this command
        \param restricted if true, commands that the source cannot access will not be mentioned
        \return array of full usage strings under the target node
        */
        std::vector<std::basic_string<CharT>> GetAllUsage(CommandNode<CharT, S>* node, S source, bool restricted)
        {
            std::vector<std::basic_string<CharT>> result;
            GetAllUsage(node, std::move(source), result, {}, restricted);
            return result;
        }

    private:
        void GetAllUsage(CommandNode<CharT, S>* node, S source, std::vector<std::basic_string<CharT>>& result, std::basic_string<CharT> prefix, bool restricted)
        {
            if (!node)
                return;

            if (restricted && !node->CanUse(source))
                return;

            if (node->GetCommand())
                result.push_back(prefix);

            if (node->GetRedirect()) {
                if (prefix.empty()) {
                    prefix = node->GetUsageText();
                }
                prefix = node->GetUsageText();
                prefix += ARGUMENT_SEPARATOR;
                if (node->GetRedirect() == root) {
                    prefix += BRIGADIER_LITERAL(CharT, "...");
                }
                else {
                    prefix += BRIGADIER_LITERAL(CharT, "-> ");
                    prefix += node->GetRedirect()->GetUsageText();
                }
                result.emplace_back(std::move(prefix));
            }
            else if (!node->GetChildren().empty()) {
                for (auto const& [name, child] : node->GetChildren()) {
                    std::basic_string<CharT> next_prefix = prefix;
                    if (!next_prefix.empty()) {
                        next_prefix += ARGUMENT_SEPARATOR;
                    }
                    next_prefix += child->GetUsageText();
                    GetAllUsage(child.get(), std::move(source), result, std::move(next_prefix), restricted);
                }
            }
        }

    public:
        /**
        Gets the possible executable commands from a specified node.

        You may use GetRoot() as a target to get usage data for the entire command tree.

        The returned syntax will be in "smart" form: `<param>`, `literal`, `[optional]` and `(either|or)`.
        These forms may be mixed and matched to provide as much information about the child nodes as it can, without being too verbose.
        For example, a required literal "foo" followed by an optional param "int" can be compressed into one string:
            `foo [<int>]`

        The path to the specified node will NOT be prepended to the output, as there can theoretically be many
        ways to reach a given node. It will only give you paths relative to the specified node, not absolute from root.

        The returned usage will be restricted to only commands that the provided source can use.

        \param node target node to get child usage strings for
        \param source a custom "source" object, usually representing the originator of this command
        \return array of full usage strings under the target node
        */
        std::map<CommandNode<CharT, S>*, std::basic_string<CharT>> GetSmartUsage(CommandNode<CharT, S>* node, S source)
        {
            std::map<CommandNode<CharT, S>*, std::basic_string<CharT>> result;

            for (auto const& [name, child] : node->GetChildren()) {
                std::basic_string<CharT> usage = GetSmartUsage(child.get(), std::move(source), node->GetCommand() != nullptr, false);
                if (!usage.empty()) {
                    result[child.get()] = std::move(usage);
                }
            }
            return result;
        }

    private:
        std::basic_string<CharT> GetSmartUsage(CommandNode<CharT, S>* node, S source, bool optional, bool deep)
        {
            if (!node)
                return {};

            if (!node->CanUse(source))
                return {};

            std::basic_string<CharT> self;
            if (optional) {
                self = USAGE_OPTIONAL_OPEN;
                self += node->GetUsageText();
                self += USAGE_OPTIONAL_CLOSE;
            }
            else {
                self = node->GetUsageText();
            }
            const bool childOptional = node->GetCommand() != nullptr;
            std::basic_string_view<CharT> open = childOptional ? USAGE_OPTIONAL_OPEN : USAGE_REQUIRED_OPEN;
            std::basic_string_view<CharT> close = childOptional ? USAGE_OPTIONAL_CLOSE : USAGE_REQUIRED_CLOSE;

            if (!deep) {
                if (node->GetRedirect()) {
                    self += ARGUMENT_SEPARATOR;
                    if (node->GetRedirect() == root) {
                        self += BRIGADIER_LITERAL(CharT, "...");
                    }
                    else {
                        self += BRIGADIER_LITERAL(CharT, "-> ");
                        self += node->GetRedirect()->GetUsageText();
                    }
                    return self;
                }
                else {
                    std::vector<CommandNode<CharT, S>*> children;
                    for (auto const& [name, child] : node->GetChildren()) {
                        if (child->CanUse(source)) {
                            children.push_back(child.get());
                        }
                    }
                    if (children.size() == 1) {
                        std::basic_string<CharT> usage = GetSmartUsage(children[0], source, childOptional, childOptional);
                        if (!usage.empty()) {
                            self += ARGUMENT_SEPARATOR;
                            self += std::move(usage);
                            return self;
                        }
                    }
                    else if (children.size() > 1) {
                        std::set<std::basic_string<CharT>> childUsage;
                        for (auto child : children) {
                            std::basic_string<CharT> usage = GetSmartUsage(child, source, childOptional, true);
                            if (!usage.empty()) {
                                childUsage.insert(usage);
                            }
                        }
                        if (childUsage.size() == 1) {
                            std::basic_string<CharT> usage = *childUsage.begin();
                            self += ARGUMENT_SEPARATOR;
                            if (childOptional) {
                                self += USAGE_OPTIONAL_OPEN;
                                self += usage;
                                self += USAGE_OPTIONAL_CLOSE;
                            }
                            else self += usage;
                            return self;
                        }
                        else if (childUsage.size() > 1) {
                            std::basic_string<CharT> builder(open);
                            int count = 0;
                            for (auto child : children) {
                                if (count > 0) {
                                    builder += USAGE_OR;
                                }
                                builder += child->GetUsageText();
                                count++;
                            }
                            if (count > 0) {
                                self += ARGUMENT_SEPARATOR;
                                self += std::move(builder);
                                self += close;
                                return self;
                            }
                        }
                    }
                }
            }

            return self;
        }
    public:
        /**
        Gets suggestions for a parsed input string on what comes next.

        As it is ultimately up to custom argument types to provide suggestions, it may be an asynchronous operation,
        for example getting in-game data or player names etc. As such, this method returns a future and no guarantees
        are made to when or how the future completes.

        The suggestions provided will be in the context of the end of the parsed input string, but may suggest
        new or replacement strings for earlier in the input string. For example, if the end of the string was
        `foobar` but an argument preferred it to be `minecraft:foobar`, it will suggest a replacement for that
        whole segment of the input.

        \param parse the result of a Parse(StringReader, Object)
        \param cancel a pointer to a bool that can cancel future when set to true. Result will be empty in such a case.
        \return a future that will eventually resolve into a Suggestions object
        */
        std::future<Suggestions<CharT>> GetCompletionSuggestions(ParseResults<CharT, S> const& parse, bool* cancel = nullptr)
        {
            return GetCompletionSuggestions(parse, parse.GetReader().GetTotalLength(), cancel);
        }

        /**
        Gets suggestions for a parsed input string on what comes next.

        As it is ultimately up to custom argument types to provide suggestions, it may be an asynchronous operation,
        for example getting in-game data or player names etc. As such, this method returns a future and no guarantees
        are made to when or how the future completes.

        The suggestions provided will be in the context of the end of the parsed input string, but may suggest
        new or replacement strings for earlier in the input string. For example, if the end of the string was
        `foobar` but an argument preferred it to be `minecraft:foobar`, it will suggest a replacement for that
        whole segment of the input.

        \param parse the result of a Parse(StringReader, Object)
        \param cursor the place where the suggestions should be considered
        \param cancel a pointer to a bool that can cancel future when set to true. Result will be empty in such a case.
        \return a future that will eventually resolve into a Suggestions object
        */
        std::future<Suggestions<CharT>> GetCompletionSuggestions(ParseResults<CharT, S> const& parse, size_t cursor, bool* cancel = nullptr)
        {
            return std::async(std::launch::async, [](ParseResults<CharT, S> const* parse, size_t cursor, bool* cancel) {
                auto context = parse->GetContext();

                SuggestionContext<CharT, S> nodeBeforeCursor = context.FindSuggestionContext(cursor);
                CommandNode<CharT, S>* parent = nodeBeforeCursor.parent;
                size_t start = (std::min)(nodeBeforeCursor.startPos, cursor);

                std::basic_string_view<CharT> fullInput = parse->GetReader().GetString();
                std::basic_string_view<CharT> truncatedInput = fullInput.substr(0, cursor);
                std::basic_string<CharT> truncatedInputLowerCase(truncatedInput);
                std::transform(truncatedInputLowerCase.begin(), truncatedInputLowerCase.end(), truncatedInputLowerCase.begin(), [](CharT c) { return std::tolower(c); });

                context.WithInput(truncatedInput);

                std::vector<std::future<Suggestions<CharT>>> futures;
                std::vector<SuggestionsBuilder<CharT>> builders;
                size_t max_size = parent->GetChildren().size();
                futures.reserve(max_size);
                builders.reserve(max_size);
                for (auto const& [name, node] : parent->GetChildren()) {
                    try {
                        builders.emplace_back(truncatedInput, truncatedInputLowerCase, start, cancel);
                        futures.push_back(node->ListSuggestions(context, builders.back()));
                    }
                    catch (CommandSyntaxException<CharT> const&) {}
                }

                std::vector<Suggestions<CharT>> suggestions;
                for (auto& future : futures)
                {
                    suggestions.emplace_back(future.get());
                }
                return Suggestions<CharT>::Merge(fullInput, suggestions);
            }, &parse, cursor, cancel);
        }

        /**
        Finds a valid path to a given node on the command tree.

        There may theoretically be multiple paths to a node on the tree, especially with the use of forking or redirecting.
        As such, this method makes no guarantees about which path it finds. It will not look at forks or redirects,
        and find the first instance of the target node on the tree.

        The only guarantee made is that for the same command tree and the same version of this library, the result of
        this method will ALWAYS be a valid input for FindNode(Collection), which should return the same node
        as provided to this method.

        \param target the target node you are finding a path for
        \return a path to the resulting node, or an empty list if it was not found
        */
        std::vector<std::basic_string<CharT>> GetPath(CommandNode<CharT, S>* target)
        {
            std::vector<std::vector<CommandNode<CharT, S>*>> nodes;
            AddPaths(root.get(), nodes, {});

            for (std::vector<CommandNode<CharT, S>*>& list : nodes) {
                if (list.back() == target) {
                    std::vector<std::basic_string<CharT>> result;
                    result.reserve(list.size());
                    for (auto node : list) {
                        if (node != root.get()) {
                            result.push_back(node->GetName());
                        }
                    }
                    return result;
                }
            }

            return {};
        }

        /**
        Finds a node by its path

        Paths may be generated with GetPath(CommandNode), and are guaranteed (for the same tree, and the
        same version of this library) to always produce the same valid node by this method.

        If a node could not be found at the specified path, then nullptr will be returned.

        \param path a generated path to a node
        \return the node at the given path, or null if not found
        */
        CommandNode<CharT, S>* FindNode(std::vector<std::basic_string<CharT>> const& path) {
            CommandNode<CharT, S>* node = root.get();
            for (auto& name : path) {
                node = node->GetChild(name).get();
                if (node == nullptr) {
                    return nullptr;
                }
            }
            return node;
        }

    public:
        /**
        Scans the command tree for potential ambiguous commands.

        This is a shortcut for CommandNode::FindAmbiguities(AmbiguityConsumer) on GetRoot().

        Ambiguities are detected by testing every CommandNode::GetExamples() on one node verses every sibling
        node. This is not fool proof, and relies a lot on the providers of the used argument types to give good examples.

        \param consumer a callback to be notified of potential ambiguities
        */
        void FindAmbiguities(AmbiguityConsumer<CharT, S> consumer) {
            if (!consumer) return;
            root->FindAmbiguities(consumer);
        }

    private:
        void AddPaths(CommandNode<CharT, S>* node, std::vector<std::vector<CommandNode<CharT, S>*>>& result, std::vector<CommandNode<CharT, S>*> parents) {
            parents.push_back(node);
            result.push_back(parents);

            for (auto const& [name, child] : node->GetChildren()) {
                AddPaths(child.get(), result, parents);
            }
        }

    private:
        std::shared_ptr<RootCommandNode<CharT, S>> root;
        ResultConsumer<CharT, S> consumer = [](CommandContext<CharT, S>& context, bool success, int result) {};
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandDispatcher);
}
#pragma once
