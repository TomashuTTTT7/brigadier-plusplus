// string type
// macro for creating specializations for basic

#pragma once

#define BRIGADIER_PACK(...) __VA_ARGS__

namespace brigadier::utils
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

#define BRIGADIER_SPECIALIZE_BASIC(type)  \

//using    type = basic_##type<char>;       \
//using W##type = basic_##type<wchar_t>;

#define BRIGADIER_SPECIALIZE_BASIC_TEMPL(type)                        \

//template<typename... Ts> using      type = Basic##type<char, Ts...>;  \
//template<typename... Ts> using   W##type = Basic##type<wchar_t, Ts...>;