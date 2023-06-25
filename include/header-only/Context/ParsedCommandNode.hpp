#pragma once

#include "../Tree/CommandNode.hpp"
#include "StringRange.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class ParsedCommandNode
    {
    public:
        ParsedCommandNode(CommandNode<CharT, S> const* node, StringRange range) : node(node), range(std::move(range)) {}

        inline CommandNode<CharT, S> const* GetNode()  const { return node;  }
        inline StringRange                  GetRange() const { return range; }
    private:
        CommandNode<CharT, S> const* node;
        StringRange range;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParsedCommandNode);
}
