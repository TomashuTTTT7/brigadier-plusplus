#pragma once

#include <string>
#include <vector>

#include "CommandNode.hpp"
#include "../Arguments/ArgumentType.hpp"

namespace brigadier
{
    template<typename S, typename T>
    class RequiredArgumentBuilder;

    template<typename S>
    class IArgumentCommandNode : public CommandNode<S>
    {
    protected:
        IArgumentCommandNode(std::string_view name) : name(name) {}
        virtual ~IArgumentCommandNode() = default;
    public:
        virtual std::string const& GetName() {
            return name;
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::ArgumentCommandNode; }
    protected:
        virtual std::string_view GetSortedKey() {
            return name;
        }
    protected:
        std::string name;
    };

    template<typename S, typename T>
    class ArgumentCommandNode : public IArgumentCommandNode<S>
    {
    private:
        static constexpr std::string_view USAGE_ARGUMENT_OPEN = "<";
        static constexpr std::string_view USAGE_ARGUMENT_CLOSE = ">";
    public:
        template<typename... Args>
        ArgumentCommandNode(std::string_view name, Args&&... args)
            : IArgumentCommandNode<S>(name)
            , type(std::forward<Args>(args)...)
        {}
        virtual ~ArgumentCommandNode() = default;
    public:
        inline SuggestionProvider<S> const& GetCustomSuggestions() const {
            return customSuggestions;
        }

        inline T const& GetType() {
            return type;
        }
    public:
        virtual std::string GetUsageText() {
            std::string ret;
            constexpr auto typeName = T::GetTypeName();
            ret.reserve(this->name.size() + USAGE_ARGUMENT_OPEN.size() + USAGE_ARGUMENT_CLOSE.size() + typeName.size() > 0 ? typeName.size() + 2 : 0);
            ret = USAGE_ARGUMENT_OPEN;
            if constexpr (typeName.size() > 0)
            {
                ret += typeName;
                ret += ": ";
            }
            ret += this->name;
            ret += USAGE_ARGUMENT_CLOSE;
            return ret;
        }
        virtual std::vector<std::string_view> GetExamples() {
            return type.GetExamples();
        }
        virtual void Parse(StringReader& reader, CommandContext<S>& contextBuilder) {
            int start = reader.GetCursor();
            using Type = typename T::type;
            Type result = type.Parse(reader);
            std::shared_ptr<ParsedArgument<S, T>> parsed = std::make_shared<ParsedArgument<S, T>>(start, reader.GetCursor(), std::move(result));

            contextBuilder.WithArgument(this->name, parsed);
            contextBuilder.WithNode(this, parsed->GetRange());
        }
        virtual std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            if (customSuggestions == nullptr) {
                return type.template ListSuggestions<S>(context, builder);
            }
            else {
                return customSuggestions(context, builder);
            }
        }
    protected:
        virtual bool IsValidInput(std::string_view input) {
            try {
                StringReader reader = StringReader(input);
                type.Parse(reader);
                return !reader.CanRead() || reader.Peek() == ' ';
            }
            catch (CommandSyntaxException const&) {
                return false;
            }
        }
    private:
        friend class RequiredArgumentBuilder<S, T>;
        T type;
        SuggestionProvider<S> customSuggestions = nullptr;
    };
}
