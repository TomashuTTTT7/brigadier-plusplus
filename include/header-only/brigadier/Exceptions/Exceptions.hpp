#pragma once

#include "../StringReader.hpp"

namespace brigadier
{
    class BuiltInExceptionProvider;

    struct ExceptionContext
    {
        ExceptionContext(std::nullptr_t) {}
        ExceptionContext() {}
        ExceptionContext(StringReader const& reader) : input(reader.GetString()), cursor(reader.GetCursor()) {}
        std::string_view input;
        int cursor = -1;
    };

    class CommandSyntaxException
    {
    public:
        static inline const int context_amount = 10;
        using BuiltInExceptions = BuiltInExceptionProvider;
        template<typename... Args>
        CommandSyntaxException(ExceptionContext ctx, Args&&... args) : ctx(ctx), msg(std::move(CreateMessageApplyContext(ctx, args...))) {}
        std::string const& What() { return msg; }
    private:
        template<typename T>
        static inline void Add(std::ostringstream& stream, T&& value)
        {
            stream << value;
        }
        template<typename T, typename... Args>
        static inline void Add(std::ostringstream& stream, T&& value, Args&&... args)
        {
            stream << value;
            Add(stream, args...);
        }
        template<typename... Args>
        static inline std::string CreateMessage(Args&&... args)
        {
            std::ostringstream s;
            Add(s, args...);
            return s.str();
        }
        template<typename... Args>
        static inline std::string CreateMessageApplyContext(ExceptionContext ctx, Args&&... args)
        {
            if (ctx.cursor < 0) return CreateMessage(args...);
            else return CreateMessage(args..., " at position ", ctx.cursor, ": ", ctx.cursor > context_amount ? "..." : "", ctx.input.substr((std::max)(0, ctx.cursor - context_amount), context_amount), "<--[HERE]");
        }
    public:
        int GetCursor() const
        {
            return ctx.cursor;
        }
        std::string_view GetInput() const
        {
            return ctx.input;
        }
    private:
        ExceptionContext ctx;
        std::string msg;
    };

    class BuiltInExceptionProvider
    {
    public:
        template<typename T0, typename T1> static inline CommandSyntaxException ValueTooLow                        (ExceptionContext ctx, T0 found, T1 min)    { return CommandSyntaxException(ctx, "Value must not be less than ", min, ", found ", found); }
        template<typename T0, typename T1> static inline CommandSyntaxException ValueTooHigh                       (ExceptionContext ctx, T0 found, T1 max)    { return CommandSyntaxException(ctx, "Value must not be more than ", max, ", found ", found); }
        template<typename T0>              static inline CommandSyntaxException LiteralIncorrect                   (ExceptionContext ctx, T0 const& expected)  { return CommandSyntaxException(ctx, "Expected literal ", expected); }
                                           static inline CommandSyntaxException ReaderExpectedStartOfQuote         (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Expected quote to start a string"); }
                                           static inline CommandSyntaxException ReaderExpectedEndOfQuote           (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Unclosed quoted string"); }
        template<typename T0>              static inline CommandSyntaxException ReaderInvalidEscape                (ExceptionContext ctx, T0 const& character) { return CommandSyntaxException(ctx, "Invalid escape sequence '", character, "' in quoted string"); }
        template<typename T0>              static inline CommandSyntaxException ReaderInvalidValue                 (ExceptionContext ctx, T0 const& value)     { return CommandSyntaxException(ctx, "Invalid value '", value, "'"); }
                                           static inline CommandSyntaxException ReaderExpectedValue                (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Expected value"); }
        template<typename T0>              static inline CommandSyntaxException ReaderExpectedSymbol               (ExceptionContext ctx, T0 const& symbol)    { return CommandSyntaxException(ctx, "Expected '", symbol, "'"); }
        template<typename T0>              static inline CommandSyntaxException ReaderExpectedOneOf                (ExceptionContext ctx, T0 const& symbols)   { return CommandSyntaxException(ctx, "Expected one of `", symbols, "`"); }
                                           static inline CommandSyntaxException DispatcherUnknownCommand           (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Unknown command"); }
                                           static inline CommandSyntaxException DispatcherUnknownArgument          (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Incorrect argument for command"); }
                                           static inline CommandSyntaxException DispatcherExpectedArgumentSeparator(ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Expected whitespace to end one argument, but found trailing data"); }
        template<typename T0>              static inline CommandSyntaxException DispatcherParseException           (ExceptionContext ctx, T0 const& message)   { return CommandSyntaxException(ctx, "Could not parse command: ", message); }
    };
}
