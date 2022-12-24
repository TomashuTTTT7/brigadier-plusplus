#pragma once

#include <vector>
#include <memory>
#include <future>

#include "ArgumentRegister.hpp"
#include "../StringReader.hpp"
#include "../Suggestion/SuggestionsBuilder.hpp"
#include "../Context/CommandContext.hpp"

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