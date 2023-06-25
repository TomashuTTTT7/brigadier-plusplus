#pragma once

#include "../Tree/CommandNode.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class SuggestionContext
    {
    public:
        SuggestionContext(CommandNode<CharT, S> const* parent, size_t startPos) : parent(parent), startPos(startPos) {}

        CommandNode<CharT, S> const* parent;
        size_t startPos;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(SuggestionContext);
}
