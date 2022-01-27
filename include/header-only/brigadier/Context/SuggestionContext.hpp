#pragma once

#include "../Tree/CommandNode.hpp"

namespace brigadier
{
    template<typename S>
    class SuggestionContext
    {
    public:
        SuggestionContext(CommandNode<S>* parent, int startPos) : parent(parent), startPos(startPos) {}

        CommandNode<S>* parent;
        int startPos;
    };
}
