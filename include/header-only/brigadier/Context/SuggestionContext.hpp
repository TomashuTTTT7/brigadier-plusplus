#pragma once

#include "../Tree/CommandNode.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicSuggestionContext
    {
    public:
        BasicSuggestionContext(BasicCommandNode<CharT, S>* parent, size_t startPos) : parent(parent), startPos(startPos) {}

        BasicCommandNode<CharT, S>* parent;
        size_t startPos;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(SuggestionContext);
}
