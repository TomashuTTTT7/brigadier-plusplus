#pragma once

#include "ArgumentBuilder.hpp"

namespace brigadier
{
    template<typename S>
    class LiteralArgumentBuilder : public ArgumentBuilder<S, LiteralArgumentBuilder<S>, LiteralCommandNode<S>>
    {
    public:
        LiteralArgumentBuilder(std::shared_ptr<LiteralCommandNode<S>> node) : ArgumentBuilder<S, LiteralArgumentBuilder<S>, LiteralCommandNode<S>>(std::move(node)) {}
    };
    REGISTER_ARGTYPE_TEMPL(LiteralArgumentBuilder, Literal);

    // single builder
    template<typename S>
    inline LiteralArgumentBuilder<S> GetBuilder(std::shared_ptr<LiteralCommandNode<S>> node)
    {
        return LiteralArgumentBuilder<S>(std::move(node));
    }

    // new single builder
    template<typename S, typename... Args>
    inline LiteralArgumentBuilder<S> MakeLiteral(Args&&... args)
    {
        return LiteralArgumentBuilder<S>(std::make_shared<LiteralCommandNode<S>>(std::forward<Args>(args)...));
    }
}
