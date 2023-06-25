#pragma once

#include "Context/CommandContext.hpp"
#include <map>

namespace brigadier
{
    template<typename CharT, typename S>
    class RootCommandNode;

    template<typename CharT, typename S>
    class ParseResults
    {
    public:
        ParseResults(CommandContext<CharT, S> context, StringReader<CharT> reader, std::map<CommandNode<CharT, S> const*, CommandSyntaxException<CharT>> exceptions)
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
        inline CommandContext<CharT, S>                                              const& GetContext()    const { return context; }
        inline StringReader<CharT>                                                   const& GetReader()     const { return reader; }
        inline std::map<CommandNode<CharT, S> const*, CommandSyntaxException<CharT>> const& GetExceptions() const { return exceptions; }

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
        inline void Reset(S source, CommandNode<CharT, S> const* root, size_t start, StringReader<CharT> reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, start);
        }
        inline void Reset(S source, CommandNode<CharT, S> const* root, StringRange range, StringReader<CharT> reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, std::move(range));
        }
    private:
        template<typename, typename>
        friend class RootCommandNode;

        CommandContext<CharT, S> context;
        std::map<CommandNode<CharT, S> const*, CommandSyntaxException<CharT>> exceptions;
        StringReader<CharT> reader;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParseResults);
}
