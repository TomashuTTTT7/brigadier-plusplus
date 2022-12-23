#pragma once

#include "ArgumentBuilder.hpp"

namespace brigadier
{
    template<typename CharT, typename S, typename T>
    class BasicRequiredArgumentBuilder : public BasicArgumentBuilder<CharT, S, BasicRequiredArgumentBuilder<CharT, S, T>, BasicArgumentCommandNode<CharT, S, T>>
    {
    public:
        BasicRequiredArgumentBuilder(std::shared_ptr<BasicArgumentCommandNode<CharT, S, T>> node) : BasicArgumentBuilder<CharT, S, BasicRequiredArgumentBuilder<CharT, S, T>, BasicArgumentCommandNode<CharT, S, T>>(std::move(node)) {}

        BasicRequiredArgumentBuilder<CharT, S, T>& Suggests(BasicSuggestionProvider<CharT, S> provider)
        {
            this->node->suggestionsProvider = provider;
            return *this;
        }
    };
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(BasicRequiredArgumentBuilder, Argument);
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(RequiredArgumentBuilder);

    // single builder
    template<typename CharT, typename S, template<typename...> typename Spec, typename... Types>
    inline BasicRequiredArgumentBuilder<CharT, S, Spec<CharT, S, Types...>> GetBuilder(std::shared_ptr<BasicArgumentCommandNode<CharT, S, Spec<CharT, S, Types...>>> node)
    {
        return BasicRequiredArgumentBuilder<CharT, S, Spec<CharT, S, Types...>>(std::move(node));
    }

    // new single builder
    template<typename CharT, typename S, template<typename...> typename Spec, typename Type, typename... Args>
    inline BasicRequiredArgumentBuilder<CharT, S, Spec<CharT, S, Type>> MakeArgument(Args&&... args)
    {
        return BasicRequiredArgumentBuilder<CharT, S, Spec<CharT, S, Type>>(std::make_shared<BasicArgumentCommandNode<CharT, S, Spec<CharT, S, Type>>>(std::forward<Args>(args)...));
    }
}
