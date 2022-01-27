#pragma once

#include "Suggestions.hpp"

namespace brigadier
{
    class SuggestionsBuilder
    {
    public:
        SuggestionsBuilder(std::string_view input, std::string_view inputLowerCase, int start, bool* cancel = nullptr) : input(input), inputLowerCase(inputLowerCase), start(start), remaining(input.substr(start)), remainingLowerCase(inputLowerCase.substr(start)), cancel(cancel) {}

        inline int GetStart() const { return start; }
        inline std::string_view GetInput() const { return input; }
        inline std::string_view GetInputLowerCase() const { return inputLowerCase; }
        inline std::string_view GetRemaining() const { return remaining; }
        inline std::string_view GetRemainingLowerCase() const { return remainingLowerCase; }

        Suggestions Build(bool* cancel = nullptr)
        {
            auto ret = Suggestions::Create(input, result, cancel);
            if (cancel != nullptr) *cancel = false;
            result.clear();
            return ret;
        }
        inline std::future<Suggestions> BuildFuture()
        {
            return std::async(std::launch::async, &SuggestionsBuilder::Build, this, this->cancel);
        }

        inline SuggestionsBuilder& Suggest(std::string_view text)
        {
            if (text == remaining) return *this;

            result.emplace_back(StringRange::Between(start, input.length()), text);
            return *this;
        }
        inline SuggestionsBuilder& Suggest(std::string_view text, std::string_view tooltip)
        {
            if (text == remaining) return *this;

            result.emplace_back(StringRange::Between(start, input.length()), text, tooltip);
            return *this;
        }
        template<typename T>
        SuggestionsBuilder& Suggest(T value)
        {
            result.emplace_back(std::move(std::to_string(value)), StringRange::Between(start, input.length()));
            return *this;
        }
        template<typename T>
        SuggestionsBuilder& Suggest(T value, std::string_view tooltip)
        {
            result.emplace_back(std::move(std::to_string(value)), StringRange::Between(start, input.length()), tooltip);
            return *this;
        }
        inline int AutoSuggest(std::string_view text, std::string_view input)
        {
            if (text.rfind(input.substr(0, text.length()), 0) == 0)
            {
                Suggest(text);
                return 1;
            }
            return 0;
        }
        inline int AutoSuggest(std::string_view text, std::string_view tooltip, std::string_view input)
        {
            if (text.rfind(input.substr(0, text.length()), 0) == 0)
            {
                Suggest(text, tooltip);
                return 1;
            }
            return 0;
        }
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        int AutoSuggest(T value, std::string_view input)
        {
            std::string val = std::to_string(value);
            if (val.rfind(input.substr(0, val.length()), 0) == 0)
            {
                result.emplace_back(std::move(val), StringRange::Between(start, input.length()));
                return 1;
            }
            return 0;
        }
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        int AutoSuggest(T value, std::string_view tooltip, std::string_view input)
        {
            std::string val = std::to_string(value);

            if (val.rfind(input.substr(0, val.length()), 0) == 0)
            {
                result.emplace_back(std::move(val), StringRange::Between(start, input.length()), tooltip);
                return 1;
            }
            return 0;
        }
        template<typename Container>
        inline int AutoSuggest(Container const& init)
        {
            int counter = 0;
            for (auto& val : init)
            {
                counter += AutoSuggest(val, GetRemaining());
            }
            return counter;
        }
        template<typename Container>
        inline int AutoSuggestLowerCase(Container const& init)
        {
            int counter = 0;
            for (auto& val : init)
            {
                counter += AutoSuggest(val, GetRemainingLowerCase());
            }
            return counter;
        }

        inline SuggestionsBuilder& Add(SuggestionsBuilder const& other)
        {
            result.insert(result.end(), other.result.begin(), other.result.end());
            return *this;
        }

        inline void SetOffset(int start)
        {
            this->start = start;
            remaining = input.substr(start);
            remainingLowerCase = inputLowerCase.substr(start);

            Restart();
        }
        inline void Restart()
        {
            result.clear();
        }

        ~SuggestionsBuilder() = default;
    private:
        int start = 0;
        std::string_view input;
        std::string_view inputLowerCase;
        std::string_view remaining;
        std::string_view remainingLowerCase;
        std::vector<Suggestion> result;
        bool* cancel = nullptr;
    };
}
