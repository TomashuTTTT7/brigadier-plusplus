#pragma once

// Test compatibility with Windows.h header
#ifdef __has_include
#if __has_include(<Windows.h>)
#include <Windows.h>
#endif
#endif

// Tests
#include "brigadier/StringReader.hpp"
#include "brigadier/CommandDispatcher.hpp"
#include "brigadier/Suggestion/Suggestion.hpp"
#include "brigadier/Context/ParsedArgument.hpp"
#include "brigadier/Arguments/ArgumentType.hpp"

// not yet implemented
#include "brigadier/Context/CommandContext.hpp"
#include "brigadier/Context/ParsedCommandNode.hpp"
#include "brigadier/Context/StringRange.hpp"
#include "brigadier/Context/SuggestionContext.hpp"
#include "brigadier/Suggestion/Suggestions.hpp"
#include "brigadier/Suggestion/SuggestionsBuilder.hpp"
#include "brigadier/Tree/CommandNode.hpp"
#include "brigadier/Tree/ArgumentCommandNode.hpp"
#include "brigadier/Tree/LiteralCommandNode.hpp"
#include "brigadier/Tree/RootCommandNode.hpp"
