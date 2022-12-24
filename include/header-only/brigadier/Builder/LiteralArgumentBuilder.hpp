#pragma once

#include "ArgumentBuilder.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class LiteralArgumentBuilder : public ArgumentBuilder<CharT, S, LiteralArgumentBuilder<CharT, S>, LiteralCommandNode<CharT, S>>
    {
    public:
        LiteralArgumentBuilder(std::shared_ptr<LiteralCommandNode<CharT, S>> node) : ArgumentBuilder<CharT, S, LiteralArgumentBuilder<CharT, S>, LiteralCommandNode<CharT, S>>(std::move(node)) {}
    };
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(LiteralArgumentBuilder, Literal);
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(LiteralArgumentBuilder);

    // single builder
    template<typename CharT, typename S>
    inline LiteralArgumentBuilder<CharT, S> GetBuilder(std::shared_ptr<LiteralCommandNode<CharT, S>> node)
    {
        return LiteralArgumentBuilder<CharT, S>(std::move(node));
    }

    // new single builder
    template<typename CharT, typename S, typename... Args>
    inline LiteralArgumentBuilder<CharT, S> MakeLiteral(Args&&... args)
    {
        return LiteralArgumentBuilder<CharT, S>(std::make_shared<LiteralCommandNode<CharT, S>>(std::forward<Args>(args)...));
    }
}
