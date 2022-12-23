#pragma once

#include "ArgumentBuilder.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicLiteralArgumentBuilder : public BasicArgumentBuilder<CharT, S, BasicLiteralArgumentBuilder<CharT, S>, BasicLiteralCommandNode<CharT, S>>
    {
    public:
        BasicLiteralArgumentBuilder(std::shared_ptr<BasicLiteralCommandNode<CharT, S>> node) : BasicArgumentBuilder<CharT, S, BasicLiteralArgumentBuilder<CharT, S>, BasicLiteralCommandNode<CharT, S>>(std::move(node)) {}
    };
    BRIGADIER_REGISTER_ARGTYPE_TEMPL(BasicLiteralArgumentBuilder, Literal);
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(LiteralArgumentBuilder);

    // single builder
    template<typename CharT, typename S>
    inline BasicLiteralArgumentBuilder<CharT, S> GetBuilder(std::shared_ptr<BasicLiteralCommandNode<CharT, S>> node)
    {
        return BasicLiteralArgumentBuilder<CharT, S>(std::move(node));
    }

    // new single builder
    template<typename CharT, typename S, typename... Args>
    inline BasicLiteralArgumentBuilder<CharT, S> MakeLiteral(Args&&... args)
    {
        return BasicLiteralArgumentBuilder<CharT, S>(std::make_shared<BasicLiteralCommandNode<CharT, S>>(std::forward<Args>(args)...));
    }
}
