#pragma once

#include "CommandNode.hpp"

namespace brigadier
{
    template<typename S>
    class RootCommandNode : public CommandNode<S>
    {
    public:
        RootCommandNode() : CommandNode<S>(nullptr, [](S&) { return true; }, nullptr, [](auto s)->std::vector<S> { return { s.GetSource() }; }, false) {}

        virtual ~RootCommandNode() = default;
        virtual std::string const& GetName() { static const std::string blank; return blank; }
        virtual std::string GetUsageText() { return {}; }
        virtual std::vector<std::string_view> GetExamples() { return {}; }
        virtual void Parse(StringReader& reader, CommandContext<S>& contextBuilder) {}
        virtual std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            return Suggestions::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::RootCommandNode; }
    protected:
        virtual bool IsValidInput(std::string_view input) { return false; }
        virtual std::string_view GetSortedKey() { return {}; }
    };
}
