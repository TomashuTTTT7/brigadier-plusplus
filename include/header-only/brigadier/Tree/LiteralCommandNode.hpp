#pragma once

#include "CommandNode.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicLiteralCommandNode : public BasicCommandNode<CharT, S>
    {
    public:
        BasicLiteralCommandNode(std::basic_string_view<CharT> literal, std::shared_ptr<BasicCommand<CharT, S>> command, Predicate<S&> requirement, std::shared_ptr<BasicCommandNode<CharT, S>> redirect, BasicRedirectModifier<CharT, S> modifier, const bool forks)
            : BasicCommandNode<CharT, S>(command, requirement, redirect, modifier, forks)
            , literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](char c) { return std::tolower(c); });
        }
        BasicLiteralCommandNode(std::basic_string_view<CharT> literal)
            : literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](char c) { return std::tolower(c); });
        }
        virtual ~BasicLiteralCommandNode() = default;
        virtual std::basic_string<CharT> const& GetName() { return literal; }
        virtual std::basic_string<CharT> GetUsageText() { return literal; }
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() { return { literal }; }
        virtual void Parse(BasicStringReader<CharT>& reader, BasicCommandContext<CharT, S>& contextBuilder)
        {
            int start = reader.GetCursor();
            int end = Parse(reader);
            if (end > -1) {
                contextBuilder.WithNode(this, BasicStringRange<CharT>::Between(start, end));
                return;
            }

            throw exceptions::LiteralIncorrect(reader, literal);
        }
        virtual std::future<BasicSuggestions<CharT>> ListSuggestions(BasicCommandContext<CharT, S>& context, BasicSuggestionsBuilder<CharT>& builder)
        {
            if (builder.AutoSuggest(literalLowerCase, builder.GetRemainingLowerCase()))
                return builder.BuildFuture();
            else
                return BasicSuggestions<CharT>::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::BasicLiteralCommandNode; }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) {
            BasicStringReader<CharT> reader(input);
            return Parse(reader) > -1;
        }
        virtual std::basic_string_view<CharT> GetSortedKey() { return literal; }
    private:
        int Parse(BasicStringReader<CharT>& reader)
        {
            int start = reader.GetCursor();
            if (reader.CanRead(literal.length())) {
                if (reader.GetString().substr(start, literal.length()) == literal) {
                    int end = start + literal.length();
                    reader.SetCursor(end);
                    if (!reader.CanRead() || reader.Peek() == ' ') {
                        return end;
                    }
                    else {
                        reader.SetCursor(start);
                    }
                }
            }
            return -1;
        }
    private:
        std::basic_string<CharT> literal;
        std::basic_string<CharT> literalLowerCase;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(LiteralCommandNode);
}
