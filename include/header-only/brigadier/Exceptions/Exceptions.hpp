#pragma once

#include "../StringReader.hxx"

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
