#pragma once

#include "../StringReader.hpp"

namespace brigadier
{
    template<typename CharT, typename E>
    class BasicException {
    public:
        BasicException() {}
        BasicException(BasicException&&) = default;
        BasicException(BasicException const& that) {
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
    class BasicRuntimeError : public BasicException<CharT, BasicRuntimeError<CharT>> {};
    BRIGADIER_SPECIALIZE_BASIC(RuntimeError);

    static constexpr size_t default_context_amount = 10;

    template<typename CharT>
    class BasicCommandSyntaxException : public BasicException<CharT, BasicCommandSyntaxException<CharT>> {
    public:
        BasicCommandSyntaxException(BasicStringReader<CharT> context) : context(context) {}
        BasicCommandSyntaxException(std::nullptr_t) {}
        BasicCommandSyntaxException() {}
        BasicCommandSyntaxException(BasicCommandSyntaxException&&) = default;
        BasicCommandSyntaxException(BasicCommandSyntaxException const& that) {
            this->context = that.context;
        }
        //virtual ~BasicCommandSyntaxException() = default;
    public:
        inline BasicStringReader<CharT> const& GetContext() const { return context; }
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
            return BasicException<CharT, BasicCommandSyntaxException<CharT>>::What();
        }
    protected:
        BasicStringReader<CharT> context;
    };
    BRIGADIER_SPECIALIZE_BASIC(CommandSyntaxException);

    namespace exceptions
    {
        template<typename CharT, typename T0, typename T1> static inline BasicCommandSyntaxException<CharT> ValueTooLow                        (BasicStringReader<CharT> ctx, T0 found, T1 min)    { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Value must not be less than ") << min << BRIGADIER_LITERAL(CharT, ", found ") << found; }
        template<typename CharT, typename T0, typename T1> static inline BasicCommandSyntaxException<CharT> ValueTooHigh                       (BasicStringReader<CharT> ctx, T0 found, T1 max)    { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Value must not be more than ") << max << BRIGADIER_LITERAL(CharT, ", found ") << found; }
        template<typename CharT, typename T0>              static inline BasicCommandSyntaxException<CharT> LiteralIncorrect                   (BasicStringReader<CharT> ctx, T0 const& expected)  { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected literal ") << expected; }
        template<typename CharT>                           static inline BasicCommandSyntaxException<CharT> ReaderExpectedStartOfQuote         (BasicStringReader<CharT> ctx)                      { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected quote to start a string"); }
        template<typename CharT>                           static inline BasicCommandSyntaxException<CharT> ReaderExpectedEndOfQuote           (BasicStringReader<CharT> ctx)                      { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Unclosed quoted string"); }
        template<typename CharT, typename T0>              static inline BasicCommandSyntaxException<CharT> ReaderInvalidEscape                (BasicStringReader<CharT> ctx, T0 const& character) { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Invalid escape sequence '") << character << BRIGADIER_LITERAL(CharT, "' in quoted string"); }
        template<typename CharT, typename T0>              static inline BasicCommandSyntaxException<CharT> ReaderInvalidValue                 (BasicStringReader<CharT> ctx, T0 const& value)     { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Invalid value '") << value << CharT('\''); }
        template<typename CharT>                           static inline BasicCommandSyntaxException<CharT> ReaderExpectedValue                (BasicStringReader<CharT> ctx)                      { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected value"); }
        template<typename CharT, typename T0>              static inline BasicCommandSyntaxException<CharT> ReaderExpectedSymbol               (BasicStringReader<CharT> ctx, T0 const& symbol)    { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected '") << symbol << CharT('\''); }
        template<typename CharT, typename T0>              static inline BasicCommandSyntaxException<CharT> ReaderExpectedOneOf                (BasicStringReader<CharT> ctx, T0 const& symbols)   { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected one of `") << symbols << CharT('`'); }
        template<typename CharT>                           static inline BasicCommandSyntaxException<CharT> DispatcherUnknownCommand           (BasicStringReader<CharT> ctx)                      { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Unknown command"); }
        template<typename CharT>                           static inline BasicCommandSyntaxException<CharT> DispatcherUnknownArgument          (BasicStringReader<CharT> ctx)                      { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Incorrect argument for command"); }
        template<typename CharT>                           static inline BasicCommandSyntaxException<CharT> DispatcherExpectedArgumentSeparator(BasicStringReader<CharT> ctx)                      { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Expected whitespace to end one argument, but found trailing data"); }
        template<typename CharT, typename T0>              static inline BasicCommandSyntaxException<CharT> DispatcherParseException           (BasicStringReader<CharT> ctx, T0 const& message)   { return BasicCommandSyntaxException<CharT>(std::move(ctx)) << BRIGADIER_LITERAL(CharT, "Could not parse command: ") << message; }
    }
}
