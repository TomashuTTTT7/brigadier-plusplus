#pragma once

#include "CommandNode.hpp"

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
