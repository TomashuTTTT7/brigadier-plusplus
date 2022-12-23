#pragma once

#include "../Context/StringRange.hpp"
#include <cstring>

namespace brigadier
{
    template<typename CharT>
    class BasicSuggestions;
    template<typename CharT>
    class BasicSuggestionsBuilder;

    template<typename CharT>
    class BasicSuggestion
    {
    public:
        BasicSuggestion(StringRange range, std::basic_string_view<CharT> text, std::basic_string_view<CharT> tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        BasicSuggestion(StringRange range, std::basic_string_view<CharT> text) : range(std::move(range)), text(std::move(text)) {}

        inline StringRange GetRange() const { return range; }
        inline std::basic_string<CharT> const& GetText() const { return text; }
        inline std::basic_string_view<CharT> GetTooltip() const { return tooltip; }

        std::basic_string<CharT> Apply(std::basic_string_view<CharT> input) const
        {
            if (range.GetStart() == 0 && range.GetEnd() == input.length()) {
                return text;
            }
            std::basic_string<CharT> result;
            result.reserve(range.GetStart() + text.length() + input.length() - (std::min)(range.GetEnd(), input.length()));
            if (range.GetStart() > 0) {
                result.append(input.substr(0, range.GetStart()));
            }
            result.append(text);
            if ((size_t)range.GetEnd() < input.length()) {
                result.append(input.substr(range.GetEnd()));
            }
            return result;
        }

        void Expand(std::basic_string_view<CharT> command, StringRange range)
        {
            if (this->range == range)
                return;

            if (range.GetStart() < this->range.GetStart()) {
                text.insert(0, command.substr(range.GetStart(), this->range.GetStart() - range.GetStart()));
            }
            if (range.GetEnd() > this->range.GetEnd()) {
                text.append(command.substr(this->range.GetEnd(), range.GetEnd() - this->range.GetEnd()));
            }

            this->range = range;
        }
    protected:
        friend class BasicSuggestions<CharT>;
        friend class BasicSuggestionsBuilder<CharT>;
        BasicSuggestion<CharT>(std::basic_string<CharT> text, StringRange range, std::basic_string_view<CharT> tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        BasicSuggestion<CharT>(std::basic_string<CharT> text, StringRange range) : range(std::move(range)), text(std::move(text)) {}
    private:
        StringRange range;
        std::basic_string<CharT> text;
        std::basic_string_view<CharT> tooltip;
    };
    BRIGADIER_SPECIALIZE_BASIC(Suggestion);

    template<typename CharT>
    struct CompareNoCase {
        inline bool operator() (BasicSuggestion<CharT> const& a, BasicSuggestion<CharT> const& b) const
        {
            CharT const* s1 = a.GetText().data();
            CharT const* s2 = b.GetText().data();

            for (size_t i = 0; s1[i] && s2[i]; ++i)
            {
                int diff = std::tolower(s1[i]) - std::tolower(s2[i]);
                if (diff) return diff < 0;
            }

            return false;
        }
    };
}
