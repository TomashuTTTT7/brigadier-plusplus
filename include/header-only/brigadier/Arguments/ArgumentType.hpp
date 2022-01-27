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
        template<typename... Args>
        static inline CommandSyntaxException CommandParseException(ExceptionContext const& ctx, Args&&... args)
        {
            return CommandSyntaxException(ctx, "Error during parsing value of type '", GetTypeName(), "': ", std::forward<Args>(args)...);
        }

        T Parse(StringReader& reader)
        {
            return reader.ReadValue<T>();
        }

        template<typename S>
        std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            return Suggestions::Empty();
        }

        static constexpr std::string_view GetTypeName()
        {
#ifdef HAS_NAMEOF
            return NAMEOF_TYPE(T).substr(NAMEOF_TYPE(T).find(NAMEOF_SHORT_TYPE(T))); // Short type + template list
#else
            return "";
#endif
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            return {};
        }
    };
    REGISTER_ARGTYPE_TEMPL(ArgumentType, Type);

    enum class StringArgType {
        SINGLE_WORD,
        QUOTABLE_PHRASE,
        GREEDY_PHRASE
    };

    template<StringArgType strType>
    class StringArgumentType : public ArgumentType<std::string>
    {
    public:
        StringArgumentType() {};

        StringArgType GetType()
        {
            return strType;
        }

        std::string Parse(StringReader& reader) {
            if (strType == StringArgType::GREEDY_PHRASE)
            {
                std::string text(reader.GetRemaining());
                reader.SetCursor(reader.GetTotalLength());
                return text;
            }
            else if (strType == StringArgType::SINGLE_WORD)
            {
                return std::string(reader.ReadUnquotedString());
            }
            else
            {
                return reader.ReadString();
            }
        }

        static constexpr std::string_view GetTypeName()
        {
            if constexpr (strType == StringArgType::GREEDY_PHRASE)
            {
                return "words";
            }
            else if constexpr (strType == StringArgType::SINGLE_WORD)
            {
                return "word";
            }
            else
            {
                return "string";
            }
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            if constexpr (strType == StringArgType::GREEDY_PHRASE)
            {
                return { "word", "words with spaces", "\"and symbols\"" };
            }
            else if constexpr (strType == StringArgType::SINGLE_WORD)
            {
                return { "word", "words_with_underscores" };
            }
            else
            {
                return { "\"quoted phrase\"", "word", "\"\"" };
            }
        }

        static std::string EscapeIfRequired(std::string_view input) {
            for (auto c : input) {
                if (!StringReader::IsAllowedInUnquotedString(c)) {
                    return Escape(std::move(input));
                }
            }
            return std::string(input);
        }

        static std::string Escape(std::string_view input) {
            std::string result = "\"";

            for (auto c : input) {
                if (c == '\\' || c == '\"') {
                    result += '\\';
                }
                result += c;
            }

            result += '\"';
            return result;
        }
    };
    REGISTER_ARGTYPE_SPEC(StringArgumentType, Word, StringArgType::SINGLE_WORD);
    REGISTER_ARGTYPE_SPEC(StringArgumentType, String, StringArgType::QUOTABLE_PHRASE);
    REGISTER_ARGTYPE_SPEC(StringArgumentType, GreedyString, StringArgType::GREEDY_PHRASE);

    class BoolArgumentType : public ArgumentType<bool>
    {
    public:
        template<typename S>
        std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            builder.AutoSuggestLowerCase(std::initializer_list({ "true", "false" }));
            return builder.BuildFuture();
        }
        static constexpr std::string_view GetTypeName()
        {
            return "bool";
        }
        static inline std::vector<std::string_view> GetExamples()
        {
            return { "true", "false" };
        }
    };
    REGISTER_ARGTYPE(BoolArgumentType, Bool);

    class CharArgumentType : public ArgumentType<char>
    {
    public:
        char Parse(StringReader& reader)
        {
            if (reader.CanRead())
                return reader.Read();
            else throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedValue(reader);
        }

        static constexpr std::string_view GetTypeName()
        {
            return "char";
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            return { "c", "@", "." };
        }
    };
    REGISTER_ARGTYPE(CharArgumentType, Char);

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

        T Parse(StringReader& reader)
        {
            int start = reader.GetCursor();
            T result = reader.ReadValue<T>();
            if (result < minimum) {
                reader.SetCursor(start);
                throw CommandSyntaxException::BuiltInExceptions::ValueTooLow(reader, result, minimum);
            }
            if (result > maximum) {
                reader.SetCursor(start);
                throw CommandSyntaxException::BuiltInExceptions::ValueTooHigh(reader, result, maximum);
            }
            return result;
        }

        static constexpr std::string_view GetTypeName()
        {
            /**/ if constexpr (std::is_floating_point_v<T>)
                return "float";
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_signed_v<T>)
                    return "int";
                else
                    return "uint";
            }
            else return ArgumentType<T>::GetTypeName();
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            /**/ if constexpr (std::is_floating_point_v<T>)
                return { "0", "1.2", ".5", "-1", "-.5", "-1234.56" };
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_signed_v<T>)
                    return { "0", "123", "-123" };
                else
                    return { "0", "123" };
            }
            else return {};
        }
    private:
        T minimum;
        T maximum;
    };
    REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Float, float);
    REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Double, double);
    REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Integer, int);
    REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Long, long long);
    REGISTER_ARGTYPE_TEMPL(ArithmeticArgumentType, Number);

#ifdef HAS_MAGICENUM
    template<typename T>
    class EnumArgumentType : public ArgumentType<T>
    {
        static_assert(std::is_enum_v<T>, "T must be enum");
    public:
        T Parse(StringReader& reader)
        {
            int start = reader.GetCursor();
            auto str = reader.ReadString();
            auto result = magic_enum::enum_cast<T>(str);
            if (!result.has_value())
            {
                throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidValue(reader, str);
            }
            return result.value();
        }

        template<typename S>
        std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            static constexpr auto names = magic_enum::enum_names<T>();
            builder.AutoSuggestLowerCase(names);
            return builder.BuildFuture();
        }

        static constexpr std::string_view GetTypeName()
        {
            return "enum";
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            static constexpr auto names = magic_enum::enum_names<T>();
            return std::vector<std::string_view>(names.begin(), names.end());
        }
    };
    REGISTER_ARGTYPE_TEMPL(EnumArgumentType, Enum);
#endif
}
