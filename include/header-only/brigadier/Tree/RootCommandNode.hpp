#pragma once

#include "CommandNode.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicRootCommandNode : public BasicCommandNode<CharT, S>
    {
    public:
        BasicRootCommandNode() : BasicCommandNode<CharT, S>(nullptr, [](S&) { return true; }, nullptr, [](auto s)->std::vector<S> { return { s.GetSource() }; }, false) {}

        virtual ~BasicRootCommandNode() = default;
        virtual std::basic_string<CharT> const& GetName() { static const std::basic_string<CharT> blank; return blank; }
        virtual std::basic_string<CharT> GetUsageText() { return {}; }
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() { return {}; }
        virtual void Parse(BasicStringReader<CharT>& reader, BasicCommandContext<CharT, S>& contextBuilder) {}
        virtual std::future<BasicSuggestions<CharT>> ListSuggestions(BasicCommandContext<CharT, S>& context, BasicSuggestionsBuilder<CharT>& builder)
        {
            return BasicSuggestions<CharT>::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::RootCommandNode; }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) { return false; }
        virtual std::basic_string_view<CharT> GetSortedKey() { return {}; }
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(RootCommandNode);
}
