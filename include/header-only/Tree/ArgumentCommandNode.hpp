#pragma once

#include <string>
#include <vector>

#include "CommandNode.hpp"
#include "../Arguments/ArgumentType.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class IArgumentCommandNode : public CommandNode<CharT, S>
    {
    protected:
        IArgumentCommandNode(std::basic_string_view<CharT> name) : name(name) {}
        explicit IArgumentCommandNode(IArgumentCommandNode<CharT, S>&) = default;
        virtual ~IArgumentCommandNode() = default;
    public:
        virtual std::basic_string<CharT> const& GetName() const {
            return name;
        }
        virtual CommandNodeType GetNodeType() const { return CommandNodeType::ArgumentCommandNode; }
    protected:
        virtual std::basic_string_view<CharT> GetSortedKey() const {
            return name;
        }
        auto Suggests(SuggestionProvider<CharT, S> provider) -> decltype(*this)
        {
            this->customSuggestions = provider;
            return *this;
        }
    protected:
        std::basic_string<CharT> name;
        SuggestionProvider<CharT, S> customSuggestions = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(IArgumentCommandNode);

    template<typename CharT, typename S, typename T>
    class ArgumentCommandNode : public IArgumentCommandNode<CharT, S>
    {
    private:
        static constexpr std::basic_string_view<CharT> USAGE_ARGUMENT_OPEN = BRIGADIER_LITERAL(CharT, "<");
        static constexpr std::basic_string_view<CharT> USAGE_ARGUMENT_CLOSE = BRIGADIER_LITERAL(CharT, ">");
    public:
        template<typename... Args>
        ArgumentCommandNode(std::basic_string_view<CharT> name, Args&&... args)
            : IArgumentCommandNode<CharT, S>(name)
            , type(std::forward<Args>(args)...)
        {}
        explicit ArgumentCommandNode(ArgumentCommandNode<CharT, S, T>&) = default;
        virtual ~ArgumentCommandNode() = default;
    public:
        inline SuggestionProvider<CharT, S> const& GetCustomSuggestions() const {
            return this->customSuggestions;
        }

        inline T const& GetType() const {
            return type;
        }
    public:
        virtual std::basic_string<CharT> GetUsageText() const {
            std::basic_string<CharT> ret;
            constexpr auto typeName = T::template GetTypeName<CharT>();
            ret.reserve(this->name.size() + USAGE_ARGUMENT_OPEN.size() + USAGE_ARGUMENT_CLOSE.size() + typeName.size() > 0 ? typeName.size() + 2 : 0);
            ret = USAGE_ARGUMENT_OPEN;
            if constexpr (typeName.size() > 0)
            {
                ret += typeName;
                ret += BRIGADIER_LITERAL(CharT, ": ");
            }
            ret += this->name;
            ret += USAGE_ARGUMENT_CLOSE;
            return ret;
        }
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() const {
            return type.template GetExamples<CharT>();
        }
        virtual void Parse(StringReader<CharT>& reader, CommandContext<CharT, S>& contextBuilder) const {
            size_t start = reader.GetCursor();
            using Type = typename T::type;
            Type result = type.Parse(reader);
            std::shared_ptr<ParsedArgument<CharT, S, T>> parsed = std::make_shared<ParsedArgument<CharT, S, T>>(start, reader.GetCursor(), std::move(result));

            contextBuilder.WithArgument(this->name, parsed);
            contextBuilder.WithNode(this, parsed->GetRange());
        }
        virtual std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder) const
        {
            if (this->customSuggestions == nullptr) {
                return this->type.template ListSuggestions<CharT, S>(context, builder);
            }
            else {
                return this->customSuggestions(context, builder);
            }
        }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) const {
            try {
                StringReader<CharT> reader = StringReader<CharT>(input);
                type.Parse(reader);
                return !reader.CanRead() || reader.Peek() == ' ';
            }
            catch (CommandSyntaxException<CharT> const&) {
                return false;
            }
        }
    private:
        T type;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ArgumentCommandNode);
}
