#pragma once

#include "../Context/StringRange.hpp"
#include <cstring>
#include <cwctype>

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
        BasicSuggestion(BasicStringRange<CharT> range, std::basic_string_view<CharT> text, std::basic_string_view<CharT> tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        BasicSuggestion(BasicStringRange<CharT> range, std::basic_string_view<CharT> text) : range(std::move(range)), text(std::move(text)) {}

        inline BasicStringRange<CharT> GetRange() const { return range; }
        inline std::basic_string<CharT> const& GetText() const { return text; }
        inline std::basic_string_view<CharT> GetTooltip() const { return tooltip; }

        std::basic_string<CharT> Apply(std::basic_string_view<CharT> input) const
        {
            if (range.GetStart() == 0 && range.GetEnd() == input.length()) {
                return text;
            }
            std::basic_string<CharT> result;
            result.reserve(range.GetStart() + text.length() + input.length() - (std::min)(range.GetEnd(), (int)input.length()));
            if (range.GetStart() > 0) {
                result.append(input.substr(0, range.GetStart()));
            }
            result.append(text);
            if ((size_t)range.GetEnd() < input.length()) {
                result.append(input.substr(range.GetEnd()));
            }
            return result;
        }

        void Expand(std::basic_string_view<CharT> command, BasicStringRange<CharT> range)
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
        BasicSuggestion<CharT>(std::basic_string<CharT> text, BasicStringRange<CharT> range, std::basic_string_view<CharT> tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        BasicSuggestion<CharT>(std::basic_string<CharT> text, BasicStringRange<CharT> range) : range(std::move(range)), text(std::move(text)) {}
    private:
        BasicStringRange<CharT> range;
        std::basic_string<CharT> text;
        std::basic_string_view<CharT> tooltip;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(Suggestion);

    template<typename CharT>
    struct CompareNoCase {
        inline bool operator() (BasicSuggestion<CharT> const& a, BasicSuggestion<CharT> const& b) const
        {
            auto& sa = a.GetText();
            auto& sb = b.GetText();
            if (sa.size() != sb.size())
                return false;
            for (size_t i = 0; i < sa.size(); ++i) {
                if constexpr (std::is_same_v<std::remove_cv_t<CharT>, char>)
                {
                    if (std::tolower(sa[i]) != std::tolower(sb[i])) return false;
                }
                else if constexpr (std::is_same_v<std::remove_cv_t<CharT>, wchar_t>)
                {
                    if (std::towlower(sa[i]) != std::towlower(sb[i])) return false;
                }
                else {
                    return sa == sb;
                }
            }
            return true;
        }
    };
}
