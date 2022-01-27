#pragma once

#include "Context/CommandContext.hpp"

#include <map>

namespace brigadier
{
    template<typename S>
    class CommandDispatcher;

    template<typename S>
    class ParseResults
    {
    public:
        ParseResults(CommandContext<S> context, StringReader reader, std::map<CommandNode<S>*, CommandSyntaxException> exceptions)
            : context(std::move(context))
            , reader(std::move(reader))
            , exceptions(std::move(exceptions))
        {}
        ParseResults(CommandContext<S> context, StringReader reader)
            : context(std::move(context))
            , reader(std::move(reader))
        {}

        ParseResults(CommandContext<S> context) : ParseResults(std::move(context), StringReader()) {}
    public:
        inline CommandContext<S> const& GetContext() const { return context; }
        inline StringReader      const& GetReader()  const { return reader;  }
        inline std::map<CommandNode<S>*, CommandSyntaxException> const& GetExceptions() const { return exceptions; }

        inline bool IsBetterThan(ParseResults<S> const& other) const
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

        inline void Reset(StringReader new_reader)
        {
            exceptions.clear();
            reader = std::move(new_reader);
        }
        inline void Reset(S source, CommandNode<S>* root, int start, StringReader reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, start);
        }
        inline void Reset(S source, CommandNode<S>* root, StringRange range, StringReader reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, std::move(range));
        }

    private:
        template<typename _S>
        friend class CommandDispatcher;

        CommandContext<S> context;
        std::map<CommandNode<S>*, CommandSyntaxException> exceptions;
        StringReader reader;
    };
}
