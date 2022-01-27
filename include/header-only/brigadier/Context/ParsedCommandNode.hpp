#pragma once

#include "../Tree/CommandNode.hpp"
#include "StringRange.hpp"

namespace brigadier
{
    template<typename S>
    class ParsedCommandNode
    {
    public:
        ParsedCommandNode(CommandNode<S>* node, StringRange range) : node(node), range(std::move(range)) {}

        inline CommandNode<S>* GetNode()  const { return node;  }
        inline StringRange     GetRange() const { return range; }
    private:
        CommandNode<S>* node;
        StringRange range;
    };
}
