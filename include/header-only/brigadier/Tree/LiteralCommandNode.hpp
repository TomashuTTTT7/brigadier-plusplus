#pragma once

#include "CommandNode.hpp"

namespace brigadier
{
    template<typename S>
    class LiteralCommandNode : public CommandNode<S>
    {
    public:
        LiteralCommandNode(std::string_view literal, std::shared_ptr<Command<S>> command, Predicate<S&> requirement, std::shared_ptr<CommandNode<S>> redirect, RedirectModifier<S> modifier, const bool forks)
            : CommandNode<S>(command, requirement, redirect, modifier, forks)
            , literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](char c) { return std::tolower(c); });
        }
        LiteralCommandNode(std::string_view literal)
            : literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](char c) { return std::tolower(c); });
        }
        virtual ~LiteralCommandNode() = default;
        virtual std::string const& GetName() { return literal; }
        virtual std::string GetUsageText() { return literal; }
        virtual std::vector<std::string_view> GetExamples() { return { literal }; }
        virtual void Parse(StringReader& reader, CommandContext<S>& contextBuilder)
        {
            int start = reader.GetCursor();
            int end = Parse(reader);
            if (end > -1) {
                contextBuilder.WithNode(this, StringRange::Between(start, end));
                return;
            }

            throw CommandSyntaxException::BuiltInExceptions::LiteralIncorrect(reader, literal);
        }
        virtual std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            if (builder.AutoSuggest(literalLowerCase, builder.GetRemainingLowerCase()))
                return builder.BuildFuture();
            else
                return Suggestions::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::LiteralCommandNode; }
    protected:
        virtual bool IsValidInput(std::string_view input) {
            StringReader reader(input);
            return Parse(reader) > -1;
        }
        virtual std::string_view GetSortedKey() { return literal; }
    private:
        int Parse(StringReader& reader)
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
        std::string literal;
        std::string literalLowerCase;
    };
}
