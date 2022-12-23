#pragma once

#include <memory>
#include "Suggestion/SuggestionsBuilder.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicCommandNode;
    template<typename CharT, typename S>
    class BasicCommandContext;

    template<typename... Ts>
    using Predicate = bool(*)(Ts&&... args);
    template<typename CharT, typename S>
    using BasicAmbiguityConsumer = void(*)(BasicCommandNode<CharT, S>* parent, BasicCommandNode<CharT, S>* child, BasicCommandNode<CharT, S>* sibling, std::set<std::basic_string_view<CharT>>& inputs);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(AmbiguityConsumer, typename S, S);
    template<typename CharT, typename S>
    using BasicCommand = int(*)(BasicCommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(Command, typename S, S);
    template<typename CharT, typename S>
    using BasicRedirectModifier = std::vector<S>(*)(BasicCommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(RedirectModifier, typename S, S);
    template<typename CharT, typename S>
    using BasicSingleRedirectModifier = S(*)(BasicCommandContext<CharT, S>& context);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(SingleRedirectModifier, typename S, S);
    template<typename CharT, typename S>
    using BasicResultConsumer = void(*)(BasicCommandContext<CharT, S>& context, bool success, int result);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(ResultConsumer, typename S, S);
    template<typename CharT, typename S>
    using BasicSuggestionProvider = std::future<BasicSuggestions<CharT>>(*)(BasicCommandContext<CharT, S>& context, BasicSuggestionsBuilder<CharT>& builder);
    BRIGADIER_SPECIALIZE_BASIC_ALIAS(SuggestionProvider, typename S, S);
}

#define COMMAND(S, ...) [](auto& ctx) -> int __VA_ARGS__
