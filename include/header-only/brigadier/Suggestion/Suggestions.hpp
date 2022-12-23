#pragma once

#include "../Context/StringRange.hpp"
#include "Suggestion.hpp"
#include <set>
#include <vector>
#include <algorithm>
#include <limits>
#include <future>

namespace brigadier
{
    template<typename CharT>
    class BasicSuggestions
    {
    public:
        BasicSuggestions(StringRange range, std::set<BasicSuggestion<CharT>, CompareNoCase<CharT>> suggestions) : range(std::move(range)), suggestions(std::move(suggestions)) {}
        BasicSuggestions() : BasicSuggestions<CharT>(StringRange::At(0), {}) {}

        inline StringRange GetRange() const { return range; }
        inline std::set<BasicSuggestion<CharT>, CompareNoCase<CharT>> const& GetList() const { return suggestions; }
        inline bool IsEmpty() { return suggestions.empty(); }

        static inline std::future<BasicSuggestions<CharT>> Empty()
        {
            std::promise<BasicSuggestions<CharT>> f;
            f.set_value(BasicSuggestions<CharT>());
            return f.get_future();
        }

        static inline BasicSuggestions<CharT> Merge(std::basic_string_view<CharT> command, std::vector<BasicSuggestions<CharT>> const& input, bool* cancel = nullptr)
        {
            /**/ if (input.empty()) return {};
            else if (input.size() == 1) return input.front();

            std::vector<BasicSuggestion<CharT>> suggestions;

            for (auto& sugs : input)
            {
                suggestions.insert(suggestions.end(), sugs.GetList().begin(), sugs.GetList().end());
            }
            return BasicSuggestions<CharT>::Create(command, suggestions, cancel);
        }
        static inline BasicSuggestions<CharT> Create(std::basic_string_view<CharT> command, std::vector<BasicSuggestion<CharT>>& suggestions, bool* cancel = nullptr)
        {
            BasicSuggestions<CharT> ret;
            if (suggestions.empty()) return ret;
            size_t start = std::numeric_limits<size_t>::max();
            size_t end = std::numeric_limits<size_t>::min();
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return ret;
                start = (std::min)(suggestion.GetRange().GetStart(), start);
                end = (std::max)(suggestion.GetRange().GetEnd(), end);
            }
            ret.range = StringRange(start, end);
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return ret;
                suggestion.Expand(command, ret.range);
            }
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return ret;
                ret.suggestions.insert(std::move(suggestion));
            }
            return ret;
        }
    private:
        StringRange range;
        std::set<BasicSuggestion<CharT>, CompareNoCase<CharT>> suggestions;
    };
    BRIGADIER_SPECIALIZE_BASIC(Suggestions);
}
