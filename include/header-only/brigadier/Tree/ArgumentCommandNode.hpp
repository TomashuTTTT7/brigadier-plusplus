#pragma once

#include <string>
#include <vector>

#include "CommandNode.hpp"
#include "../Arguments/ArgumentType.hpp"

namespace brigadier
{
    template<typename CharT, typename S, typename T>
    class BasicRequiredArgumentBuilder;

    template<typename CharT, typename S>
    class BasicIArgumentCommandNode : public BasicCommandNode<CharT, S>
    {
    protected:
        BasicIArgumentCommandNode(std::basic_string_view<CharT> name) : name(name) {}
        virtual ~BasicIArgumentCommandNode() = default;
    public:
        virtual std::basic_string<CharT> const& GetName() {
            return name;
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::BasicArgumentCommandNode; }
    protected:
        virtual std::basic_string_view<CharT> GetSortedKey() {
            return name;
        }
    protected:
        std::basic_string<CharT> name;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(IArgumentCommandNode);

    template<typename CharT, typename S, typename T>
    class BasicArgumentCommandNode : public BasicIArgumentCommandNode<CharT, S>
    {
    private:
        static constexpr std::basic_string_view<CharT> USAGE_ARGUMENT_OPEN = "<";
        static constexpr std::basic_string_view<CharT> USAGE_ARGUMENT_CLOSE = ">";
    public:
        template<typename... Args>
        BasicArgumentCommandNode(std::basic_string_view<CharT> name, Args&&... args)
            : BasicIArgumentCommandNode<CharT, S>(name)
            , type(std::forward<Args>(args)...)
        {}
        virtual ~BasicArgumentCommandNode() = default;
    public:
        inline BasicSuggestionProvider<CharT, S> const& GetCustomSuggestions() const {
            return customSuggestions;
        }

        inline T const& GetType() {
            return type;
        }
    public:
        virtual std::basic_string<CharT> GetUsageText() {
            std::basic_string<CharT> ret;
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
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() {
            return type.GetExamples();
        }
        virtual void Parse(BasicStringReader<CharT>& reader, BasicCommandContext<CharT, S>& contextBuilder) {
            int start = reader.GetCursor();
            using Type = typename T::type;
            Type result = type.Parse(reader);
            std::shared_ptr<BasicParsedArgument<S, T>> parsed = std::make_shared<BasicParsedArgument<S, T>>(start, reader.GetCursor(), std::move(result));

            contextBuilder.WithArgument(this->name, parsed);
            contextBuilder.WithNode(this, parsed->GetRange());
        }
        virtual std::future<BasicSuggestions<CharT>> ListSuggestions(BasicCommandContext<CharT, S>& context, BasicSuggestionsBuilder<CharT>& builder)
        {
            if (customSuggestions == nullptr) {
                return type.template ListSuggestions<CharT, S>(context, builder);
            }
            else {
                return customSuggestions(context, builder);
            }
        }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) {
            try {
                BasicStringReader<CharT> reader = BasicStringReader<CharT>(input);
                type.Parse(reader);
                return !reader.CanRead() || reader.Peek() == ' ';
            }
            catch (BasicCommandSyntaxException<CharT> const&) {
                return false;
            }
        }
    private:
        friend class BasicRequiredArgumentBuilder<CharT, S, T>;
        T type;
        BasicSuggestionProvider<CharT, S> customSuggestions = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ArgumentCommandNode);
}
