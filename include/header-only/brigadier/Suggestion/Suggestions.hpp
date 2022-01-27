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
    class Suggestions
    {
    public:
        Suggestions(StringRange range, std::set<Suggestion, CompareNoCase> suggestions) : range(std::move(range)), suggestions(std::move(suggestions)) {}
        Suggestions() : Suggestions(StringRange::At(0), {}) {}

        inline StringRange GetRange() const { return range; }
        inline std::set<Suggestion, CompareNoCase> const& GetList() const { return suggestions; }
        inline bool IsEmpty() { return suggestions.empty(); }

        static inline std::future<Suggestions> Empty();

        static inline Suggestions Merge(std::string_view command, std::vector<Suggestions> const& input, bool* cancel = nullptr)
        {
            /**/ if (input.empty()) return {};
            else if (input.size() == 1) return input.front();

            std::vector<Suggestion> suggestions;

            for (auto& sugs : input)
            {
                suggestions.insert(suggestions.end(), sugs.GetList().begin(), sugs.GetList().end());
            }
            return Suggestions::Create(command, suggestions, cancel);
        }
        static inline Suggestions Create(std::string_view command, std::vector<Suggestion>& suggestions, bool* cancel = nullptr)
        {
            if (suggestions.empty()) return {};
            int start = std::numeric_limits<int>::max();
            int end = std::numeric_limits<int>::min();
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return {};
                start = (std::min)(suggestion.GetRange().GetStart(), start);
                end = (std::max)(suggestion.GetRange().GetEnd(), end);
            }
            std::set<Suggestion, CompareNoCase> suggest;
            StringRange range = StringRange(start, end);
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return {}; // Suggestions(range, std::move(suggest));
                suggestion.Expand(command, range);
            }
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return Suggestions(range, std::move(suggest));
                suggest.insert(std::move(suggestion));
            }
            return Suggestions(range, std::move(suggest));
        }
    private:
        StringRange range;
        std::set<Suggestion, CompareNoCase> suggestions;
    };

    std::future<Suggestions> Suggestions::Empty()
    {
        std::promise<Suggestions> f;
        f.set_value(Suggestions());
        return f.get_future();
    }
}
