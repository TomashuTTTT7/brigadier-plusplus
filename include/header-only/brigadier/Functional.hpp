#pragma once

#include <memory>
#include "Suggestion/SuggestionsBuilder.hpp"

namespace brigadier
{
    template<typename S>
    class CommandNode;
    template<typename S>
    class CommandContext;

    template<typename... Ts>
    using Predicate = bool(*)(Ts&&... args);
    template<typename S>
    using AmbiguityConsumer = void(*)(CommandNode<S>* parent, CommandNode<S>* child, CommandNode<S>* sibling, std::set<std::string_view>& inputs);
    template<typename S>
    using Command = int(*)(CommandContext<S>& context);
    template<typename S>
    using RedirectModifier = std::vector<S>(*)(CommandContext<S>& context);
    template<typename S>
    using SingleRedirectModifier = S(*)(CommandContext<S>& context);
    template<typename S>
    using ResultConsumer = void(*)(CommandContext<S>& context, bool success, int result);
    template<typename S>
    using SuggestionProvider = std::future<Suggestions>(*)(CommandContext<S>& context, SuggestionsBuilder& builder);
}

#define COMMAND(S, ...) [](brigadier::CommandContext<S>& ctx) -> int __VA_ARGS__
