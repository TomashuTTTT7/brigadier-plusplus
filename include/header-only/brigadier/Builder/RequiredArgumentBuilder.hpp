#pragma once

#include "ArgumentBuilder.hpp"

namespace brigadier
{
    template<typename CharT, typename S, typename T>
    class RequiredArgumentBuilder : public ArgumentBuilder<CharT, S, RequiredArgumentBuilder<CharT, S, T>, ArgumentCommandNode<CharT, S, T>>
    {
    public:
        RequiredArgumentBuilder(std::shared_ptr<ArgumentCommandNode<CharT, S, T>> node) : ArgumentBuilder<CharT, S, RequiredArgumentBuilder<CharT, S, T>, ArgumentCommandNode<CharT, S, T>>(std::move(node)) {}

        RequiredArgumentBuilder<CharT, S, T>& Suggests(SuggestionProvider<CharT, S> provider)
        {
            this->node->suggestionsProvider = provider;
            return *this;
        }
    };
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(RequiredArgumentBuilder, Argument);
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(RequiredArgumentBuilder);

    // single builder
    template<typename CharT, typename S, template<typename...> typename Spec, typename... Types>
    inline RequiredArgumentBuilder<CharT, S, Spec<CharT, S, Types...>> GetBuilder(std::shared_ptr<ArgumentCommandNode<CharT, S, Spec<CharT, S, Types...>>> node)
    {
        return RequiredArgumentBuilder<CharT, S, Spec<CharT, S, Types...>>(std::move(node));
    }

    // new single builder
    template<typename CharT, typename S, template<typename...> typename Spec, typename Type, typename... Args>
    inline RequiredArgumentBuilder<CharT, S, Spec<CharT, S, Type>> MakeArgument(Args&&... args)
    {
        return RequiredArgumentBuilder<CharT, S, Spec<CharT, S, Type>>(std::make_shared<ArgumentCommandNode<CharT, S, Spec<CharT, S, Type>>>(std::forward<Args>(args)...));
    }
}
