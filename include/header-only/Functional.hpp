#pragma once

#include <memory>
#include "Suggestion/SuggestionsBuilder.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class CommandNode;
    template<typename CharT, typename S>
    class CommandContext;

    template<typename... Ts>
    using Predicate = bool(*)(Ts&&... args);
    template<typename CharT, typename S>
    using AmbiguityConsumer = void(*)(CommandNode<CharT, S>* parent, CommandNode<CharT, S>* child, CommandNode<CharT, S>* sibling, std::set<std::basic_string_view<CharT>>& inputs);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(AmbiguityConsumer, typename S, S);
    template<typename CharT, typename S>
    using Command = int(*)(CommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(Command, typename S, S);
    template<typename CharT, typename S>
    using RedirectModifier = std::vector<S>(*)(CommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(RedirectModifier, typename S, S);
    template<typename CharT, typename S>
    using SingleRedirectModifier = S(*)(CommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(SingleRedirectModifier, typename S, S);
    template<typename CharT, typename S>
    using ResultConsumer = void(*)(CommandContext<CharT, S>& context, bool success, int result);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(ResultConsumer, typename S, S);
    template<typename CharT, typename S>
    using SuggestionProvider = std::future<Suggestions<CharT>>(*)(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(SuggestionProvider, typename S, S);
}

#define COMMAND [](auto& ctx) -> int
