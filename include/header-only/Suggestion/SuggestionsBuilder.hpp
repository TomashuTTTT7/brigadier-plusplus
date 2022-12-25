#pragma once

#include "Suggestions.hpp"

namespace brigadier
{
    template<typename CharT>
    class SuggestionsBuilder
    {
    public:
        SuggestionsBuilder(std::basic_string_view<CharT> input, std::basic_string_view<CharT> inputLowerCase, size_t start, bool* cancel = nullptr) : start(start), input(input), inputLowerCase(inputLowerCase), remaining(input.substr(start)), remainingLowerCase(inputLowerCase.substr(start)), cancel(cancel) {}

        inline size_t GetStart() const { return start; }
        inline std::basic_string_view<CharT> GetInput() const { return input; }
        inline std::basic_string_view<CharT> GetInputLowerCase() const { return inputLowerCase; }
        inline std::basic_string_view<CharT> GetRemaining() const { return remaining; }
        inline std::basic_string_view<CharT> GetRemainingLowerCase() const { return remainingLowerCase; }

        Suggestions<CharT> Build(bool* cancel = nullptr)
        {
            auto ret = Suggestions<CharT>::Create(input, result, cancel);
            if (cancel != nullptr) *cancel = false;
            result.clear();
            return ret;
        }
        inline std::future<Suggestions<CharT>> BuildFuture()
        {
            return std::async(std::launch::async, &SuggestionsBuilder<CharT>::Build, this, this->cancel);
        }

        inline SuggestionsBuilder<CharT>& Suggest(std::basic_string_view<CharT> text)
        {
            if (text == remaining) return *this;

            result.emplace_back(StringRange::Between(start, input.length()), text);
            return *this;
        }
        inline SuggestionsBuilder<CharT>& Suggest(std::basic_string_view<CharT> text, std::basic_string_view<CharT> tooltip)
        {
            if (text == remaining) return *this;

            result.emplace_back(StringRange::Between(start, input.length()), text, tooltip);
            return *this;
        }
        template<typename T>
        SuggestionsBuilder<CharT>& Suggest(T value)
        {
            result.emplace_back(std::move(std::to_string(value)), StringRange::Between(start, input.length()));
            return *this;
        }
        template<typename T>
        SuggestionsBuilder<CharT>& Suggest(T value, std::basic_string_view<CharT> tooltip)
        {
            result.emplace_back(std::move(std::to_string(value)), StringRange::Between(start, input.length()), tooltip);
            return *this;
        }
        inline int AutoSuggest(std::basic_string_view<CharT> text, std::basic_string_view<CharT> input)
        {
            if (text.rfind(input.substr(0, text.length()), 0) == 0)
            {
                Suggest(text);
                return 1;
            }
            return 0;
        }
        inline int AutoSuggest(std::basic_string_view<CharT> text, std::basic_string_view<CharT> tooltip, std::basic_string_view<CharT> input)
        {
            if (text.rfind(input.substr(0, text.length()), 0) == 0)
            {
                Suggest(text, tooltip);
                return 1;
            }
            return 0;
        }
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        int AutoSuggest(T value, std::basic_string_view<CharT> input)
        {
            std::basic_string<CharT> val = std::to_string(value);
            if (val.rfind(input.substr(0, val.length()), 0) == 0)
            {
                result.emplace_back(std::move(val), StringRange::Between(start, input.length()));
                return 1;
            }
            return 0;
        }
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        int AutoSuggest(T value, std::basic_string_view<CharT> tooltip, std::basic_string_view<CharT> input)
        {
            std::basic_string<CharT> val = std::to_string(value);

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

        inline SuggestionsBuilder<CharT>& Add(SuggestionsBuilder<CharT> const& other)
        {
            result.insert(result.end(), other.result.begin(), other.result.end());
            return *this;
        }

        inline void SetOffset(size_t start)
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

        ~SuggestionsBuilder<CharT>() = default;
    private:
        size_t start = 0;
        std::basic_string_view<CharT> input;
        std::basic_string_view<CharT> inputLowerCase;
        std::basic_string_view<CharT> remaining;
        std::basic_string_view<CharT> remainingLowerCase;
        std::vector<Suggestion<CharT>> result;
        bool* cancel = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC(SuggestionsBuilder);
}
