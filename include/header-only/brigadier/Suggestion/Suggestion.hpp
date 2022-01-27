#pragma once

#include "../Context/StringRange.hpp"
#include <cstring>

namespace brigadier
{
    class Suggestions;
    class SuggestionsBuilder;

    class Suggestion
    {
    public:
        Suggestion(StringRange range, std::string_view text, std::string_view tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        Suggestion(StringRange range, std::string_view text) : range(std::move(range)), text(std::move(text)) {}

        inline StringRange GetRange() const { return range; }
        inline std::string const& GetText() const { return text; }
        inline std::string_view GetTooltip() const { return tooltip; }

        std::string Apply(std::string_view input) const
        {
            if (range.GetStart() == 0 && range.GetEnd() == input.length()) {
                return text;
            }
            std::string result;
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

        void Expand(std::string_view command, StringRange range)
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
        friend class Suggestions;
        friend class SuggestionsBuilder;
        Suggestion(std::string text, StringRange range, std::string_view tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        Suggestion(std::string text, StringRange range) : range(std::move(range)), text(std::move(text)) {}
    private:
        StringRange range;
        std::string text;
        std::string_view tooltip;
    };

    struct CompareNoCase {
        inline bool operator() (Suggestion const& a, Suggestion const& b) const
        {
#ifdef __unix__
            return strcasecmp(a.GetText().c_str(), b.GetText().c_str()) < 0;
#else
            return _stricmp  (a.GetText().c_str(), b.GetText().c_str()) < 0;
#endif
        }
    };
}
