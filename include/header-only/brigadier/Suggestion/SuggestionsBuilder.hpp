﻿#pragma once

#include "Suggestions.hpp"

namespace brigadier
{
    template<typename CharT>
    class BasicSuggestionsBuilder
    {
    public:
        BasicSuggestionsBuilder(std::basic_string_view<CharT> input, std::basic_string_view<CharT> inputLowerCase, int start, bool* cancel = nullptr) : input(input), inputLowerCase(inputLowerCase), start(start), remaining(input.substr(start)), remainingLowerCase(inputLowerCase.substr(start)), cancel(cancel) {}

        inline int GetStart() const { return start; }
        inline std::basic_string_view<CharT> GetInput() const { return input; }
        inline std::basic_string_view<CharT> GetInputLowerCase() const { return inputLowerCase; }
        inline std::basic_string_view<CharT> GetRemaining() const { return remaining; }
        inline std::basic_string_view<CharT> GetRemainingLowerCase() const { return remainingLowerCase; }

        BasicSuggestions<CharT> Build(bool* cancel = nullptr)
        {
            auto ret = BasicSuggestions<CharT>::Create(input, result, cancel);
            if (cancel != nullptr) *cancel = false;
            result.clear();
            return ret;
        }
        inline std::future<BasicSuggestions<CharT>> BuildFuture()
        {
            return std::async(std::launch::async, &BasicSuggestionsBuilder<CharT>::Build, this, this->cancel);
        }

        inline BasicSuggestionsBuilder<CharT>& Suggest(std::basic_string_view<CharT> text)
        {
            if (text == remaining) return *this;

            result.emplace_back(BasicStringRange<CharT>::Between(start, input.length()), text);
            return *this;
        }
        inline BasicSuggestionsBuilder<CharT>& Suggest(std::basic_string_view<CharT> text, std::basic_string_view<CharT> tooltip)
        {
            if (text == remaining) return *this;

            result.emplace_back(BasicStringRange<CharT>::Between(start, input.length()), text, tooltip);
            return *this;
        }
        template<typename T>
        BasicSuggestionsBuilder<CharT>& Suggest(T value)
        {
            result.emplace_back(std::move(std::to_string(value)), BasicStringRange<CharT>::Between(start, input.length()));
            return *this;
        }
        template<typename T>
        BasicSuggestionsBuilder<CharT>& Suggest(T value, std::basic_string_view<CharT> tooltip)
        {
            result.emplace_back(std::move(std::to_string(value)), BasicStringRange<CharT>::Between(start, input.length()), tooltip);
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
                result.emplace_back(std::move(val), BasicStringRange<CharT>::Between(start, input.length()));
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
                result.emplace_back(std::move(val), BasicStringRange<CharT>::Between(start, input.length()), tooltip);
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

        inline BasicSuggestionsBuilder<CharT>& Add(BasicSuggestionsBuilder<CharT> const& other)
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

        ~BasicSuggestionsBuilder<CharT>() = default;
    private:
        int start = 0;
        std::basic_string_view<CharT> input;
        std::basic_string_view<CharT> inputLowerCase;
        std::basic_string_view<CharT> remaining;
        std::basic_string_view<CharT> remainingLowerCase;
        std::vector<BasicSuggestion<CharT>> result;
        bool* cancel = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(SuggestionsBuilder);
}
