#pragma once

#include "../Tree/CommandNode.hpp"
#include "StringRange.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicParsedCommandNode
    {
    public:
        BasicParsedCommandNode(BasicCommandNode<CharT, S>* node, BasicStringRange<CharT> range) : node(node), range(std::move(range)) {}

        inline BasicCommandNode<CharT, S>* GetNode()  const { return node;  }
        inline BasicStringRange<CharT>     GetRange() const { return range; }
    private:
        BasicCommandNode<CharT, S>* node;
        BasicStringRange<CharT> range;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParsedCommandNode);
}
