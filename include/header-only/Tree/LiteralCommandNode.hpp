#pragma once

#include "CommandNode.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class LiteralCommandNode : public CommandNode<CharT, S>
    {
    public:
        LiteralCommandNode(std::basic_string_view<CharT> literal, std::shared_ptr<Command<CharT, S>> command, Predicate<S&> requirement, std::shared_ptr<CommandNode<CharT, S>> redirect, RedirectModifier<CharT, S> modifier, const bool forks)
            : CommandNode<CharT, S>(command, requirement, redirect, modifier, forks)
            , literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](CharT c) { return std::tolower(c); });
        }
        LiteralCommandNode(std::basic_string_view<CharT> literal)
            : literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](CharT c) { return std::tolower(c); });
        }
        explicit LiteralCommandNode(LiteralCommandNode<CharT, S>&) = default;
        virtual ~LiteralCommandNode() = default;
        virtual std::basic_string<CharT> const& GetName() const { return literal; }
        virtual std::basic_string<CharT> GetUsageText() const { return literal; }
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() const { return { literal }; }
        virtual void Parse(StringReader<CharT>& reader, CommandContext<CharT, S>& contextBuilder) const
        {
            size_t start = reader.GetCursor();
            size_t end = Parse(reader);
            if (end != size_t(-1)) {
                contextBuilder.WithNode(this, StringRange::Between(start, end));
                return;
            }

            throw exceptions::LiteralIncorrect(reader, literal);
        }
        virtual std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder) const
        {
            if (builder.AutoSuggest(literalLowerCase, builder.GetRemainingLowerCase()))
                return builder.BuildFuture();
            else
                return Suggestions<CharT>::Empty();
        }
        virtual CommandNodeType GetNodeType() const { return CommandNodeType::LiteralCommandNode; }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) const {
            StringReader<CharT> reader(input);
            return Parse(reader) != size_t(-1);
        }
        virtual std::basic_string_view<CharT> GetSortedKey() const { return literal; }
    private:
        size_t Parse(StringReader<CharT>& reader) const
        {
            size_t start = reader.GetCursor();
            if (reader.CanRead(literal.length())) {
                if (reader.GetString().substr(start, literal.length()) == literal) {
                    size_t end = start + literal.length();
                    reader.SetCursor(end);
                    if (!reader.CanRead() || reader.Peek() == CharT(' ')) {
                        return end;
                    }
                    else {
                        reader.SetCursor(start);
                    }
                }
            }
            return size_t(-1);
        }
    private:
        std::basic_string<CharT> literal;
        std::basic_string<CharT> literalLowerCase;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(LiteralCommandNode);
}
