#pragma once

#include "ArgumentBuilder.hpp"

namespace brigadier
{
    template<typename S, typename T>
    class RequiredArgumentBuilder : public ArgumentBuilder<S, RequiredArgumentBuilder<S, T>, ArgumentCommandNode<S, T>>
    {
    public:
        RequiredArgumentBuilder(std::shared_ptr<ArgumentCommandNode<S, T>> node) : ArgumentBuilder<S, RequiredArgumentBuilder<S, T>, ArgumentCommandNode<S, T>>(std::move(node)) {}

        RequiredArgumentBuilder<S, T>& Suggests(SuggestionProvider<S> provider)
        {
            this->node->suggestionsProvider = provider;
            return *this;
        }
    };
    REGISTER_ARGTYPE_TEMPL(RequiredArgumentBuilder, Argument);

    // single builder
    template<typename S, template<typename...> typename Spec, typename... Types>
    inline RequiredArgumentBuilder<S, Spec<S, Types...>> GetBuilder(std::shared_ptr<ArgumentCommandNode<S, Spec<S, Types...>>> node)
    {
        return RequiredArgumentBuilder<S, Spec<S, Types...>>(std::move(node));
    }

    // new single builder
    template<typename S, template<typename...> typename Spec, typename Type, typename... Args>
    inline RequiredArgumentBuilder<S, Spec<S, Type>> MakeArgument(Args&&... args)
    {
        return RequiredArgumentBuilder<S, Spec<S, Type>>(std::make_shared<ArgumentCommandNode<S, Spec<S, Type>>>(std::forward<Args>(args)...));
    }
}
