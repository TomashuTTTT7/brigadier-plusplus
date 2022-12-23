#pragma once

#include "Context/CommandContext.hpp"
#include <map>

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicCommandDispatcher;

    template<typename CharT, typename S>
    class BasicParseResults
    {
    public:
        BasicParseResults(BasicCommandContext<CharT, S> context, BasicStringReader<CharT> reader, std::map<BasicCommandNode<CharT, S>*, BasicCommandSyntaxException<CharT>> exceptions)
            : context(std::move(context))
            , reader(std::move(reader))
            , exceptions(std::move(exceptions))
        {}
        BasicParseResults(BasicCommandContext<CharT, S> context, BasicStringReader<CharT> reader)
            : context(std::move(context))
            , reader(std::move(reader))
        {}

        BasicParseResults(BasicCommandContext<CharT, S> context) : BasicParseResults(std::move(context), BasicStringReader<CharT>()) {}
    public:
        inline BasicCommandContext<CharT, S> const& GetContext() const { return context; }
        inline BasicStringReader<CharT> const& GetReader()  const { return reader; }
        inline std::map<BasicCommandNode<CharT, S>*, BasicCommandSyntaxException<CharT>> const& GetExceptions() const { return exceptions; }

        inline bool IsBetterThan(BasicParseResults<CharT, S> const& other) const
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

        inline void Reset(BasicStringReader<CharT> new_reader)
        {
            exceptions.clear();
            reader = std::move(new_reader);
        }
        inline void Reset(S source, BasicCommandNode<CharT, S>* root, size_t start, BasicStringReader<CharT> reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, start);
        }
        inline void Reset(S source, BasicCommandNode<CharT, S>* root, StringRange range, BasicStringReader<CharT> reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, std::move(range));
        }
    private:
        template<typename, typename>
        friend class BasicCommandDispatcher;

        BasicCommandContext<CharT, S> context;
        std::map<BasicCommandNode<CharT, S>*, BasicCommandSyntaxException<CharT>> exceptions;
        BasicStringReader<CharT> reader;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParseResults);
}
