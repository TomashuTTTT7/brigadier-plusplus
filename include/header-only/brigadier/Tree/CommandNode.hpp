#pragma once

#include <map>
#include <set>
#include <string>
#include <tuple>

#include "../Functional.hpp"
#include "../StringReader.hpp"
#include "../Suggestion/SuggestionsBuilder.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicCommandNode;
    template<typename CharT, typename S>
    class BasicIArgumentCommandNode;
    template<typename CharT, typename S>
    class BasicLiteralCommandNode;
    template<typename CharT, typename S>
    class BasicRootCommandNode;
    template<typename CharT, typename S>
    class BasicCommandDispatcher;

    template<typename CharT, typename S, typename T, typename node_type>
    class BasicArgumentBuilder;
    template<typename CharT, typename S>
    class BasicMultiArgumentBuilder;
    template<typename CharT, typename S>
    class BasicLiteralArgumentBuilder;
    template<typename CharT, typename S, typename T>
    class BasicRequiredArgumentBuilder;

    template<typename CharT, typename S>
    class BasicCommandContext;

    enum class CommandNodeType
    {
        RootCommandNode,
        BasicLiteralCommandNode,
        BasicArgumentCommandNode
    };

    template<typename CharT, typename S>
    class BasicCommandNode
    {
    public:
        BasicCommandNode(BasicCommand<CharT, S> command, Predicate<S&> requirement, std::shared_ptr<BasicCommandNode<CharT, S>> redirect, BasicRedirectModifier<CharT, S> modifier, const bool forks)
            : command(std::move(command))
            , requirement(std::move(requirement))
            , redirect(std::move(redirect))
            , modifier(std::move(modifier))
            , forks(std::move(forks))
        {}
        BasicCommandNode() {}
        virtual ~BasicCommandNode() = default;
    public:
        inline BasicCommand<CharT, S> GetCommand() const
        {
            return command;
        }

        inline std::map<std::basic_string<CharT>, std::shared_ptr<BasicCommandNode<CharT, S>>, std::less<>> const& GetChildren() const
        {
            return children;
        }

        inline std::shared_ptr<BasicCommandNode<CharT, S>> GetChild(std::basic_string_view<CharT> name) const
        {
            auto found = children.find(name);
            if (found != children.end())
                return found->second;
            return nullptr;
        }

        inline std::shared_ptr<BasicCommandNode<CharT, S>> GetRedirect() const
        {
            return redirect;
        }

        inline BasicRedirectModifier<CharT, S> GetRedirectModifier() const 
        {
            return modifier;
        }

        inline Predicate<S&> GetRequirement()
        {
            return requirement;
        }

        inline bool IsFork() const
        {
            return forks;
        }

        inline bool CanUse(S& source)
        {
            if (requirement)
                return requirement(source);
            else return true;
        }

        void AddChild(std::shared_ptr<BasicCommandNode<CharT, S>> node)
        {
            if (node == nullptr)
                return;

            if (node->GetNodeType() == CommandNodeType::RootCommandNode) {
                throw std::runtime_error("Cannot add a RootCommandNode as a child to any other BasicCommandNode");
            }

            auto child = children.find(node->GetName());
            if (child != children.end()) {
                // We've found something to merge onto
                auto child_node = child->second;

                if (child_node->GetNodeType() != node->GetNodeType())
                    throw std::runtime_error("Node type (literal/argument) mismatch!");

                auto node_command = node->GetCommand();
                if (node_command != nullptr) {
                    child_node->command = node_command;
                }
                for (auto& [name, grandchild] : node->GetChildren()) {
                    child_node->AddChild(grandchild);
                }
            }
            else {
                children.emplace(node->GetName(), node);
                if (node->GetNodeType() == CommandNodeType::BasicLiteralCommandNode) {
                    literals.emplace_back(std::move(std::static_pointer_cast<BasicLiteralCommandNode<CharT, S>>(std::move(node))));
                }
                else if (node->GetNodeType() == CommandNodeType::BasicArgumentCommandNode) {
                    arguments.emplace_back(std::move(std::static_pointer_cast<BasicIArgumentCommandNode<CharT, S>>(std::move(node))));
                }
            }
        }

        void FindAmbiguities(BasicAmbiguityConsumer<CharT, S> consumer)
        {
            for (auto [child_name, child] : children) {
                for (auto [sibling_name, sibling] : children) {
                    if (child == sibling)
                        continue;

                    std::set<std::basic_string_view<CharT>> matches;

                    for (auto input : child->GetExamples()) {
                        if (sibling->IsValidInput(input)) {
                            matches.insert(input);
                        }
                    }

                    if (matches.size() > 0) {
                        consumer(this, child, sibling, matches);
                    }
                }

                child.FindAmbiguities(consumer);
            }
        }

        std::tuple<std::shared_ptr<BasicCommandNode<CharT, S>>*, size_t> GetRelevantNodes(BasicStringReader<CharT>& input)
        {
            if (literals.size() > 0) {
                int cursor = input.GetCursor();
                while (input.CanRead() && input.Peek() != ' ') {
                    input.Skip();
                }
                std::basic_string_view<CharT> text = input.GetString().substr(cursor, input.GetCursor() - cursor);
                input.SetCursor(cursor);
                auto literal = children.find(text);
                if (literal != children.end() && literal->second->GetNodeType() == CommandNodeType::BasicLiteralCommandNode) {
                    return std::tuple<std::shared_ptr<BasicCommandNode<CharT, S>>*, size_t>(&literal->second, 1);
                }
                else {
                    return std::tuple<std::shared_ptr<BasicCommandNode<CharT, S>>*, size_t>((std::shared_ptr<BasicCommandNode<CharT, S>>*)arguments.data(), arguments.size());
                }
            }
            else {
                return std::tuple<std::shared_ptr<BasicCommandNode<CharT, S>>*, size_t>((std::shared_ptr<BasicCommandNode<CharT, S>>*)arguments.data(), arguments.size());
            }
        }

        bool HasCommand()
        {
            if (GetCommand() != nullptr) return true;
            for (auto [name, child] : children)
                if (child && child->HasCommand())
                    return true;
            return false;
        }
    public:
        virtual std::basic_string<CharT> const& GetName() = 0;
        virtual std::basic_string<CharT> GetUsageText() = 0;
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() = 0;
        virtual void Parse(BasicStringReader<CharT>& reader, BasicCommandContext<CharT, S>& contextBuilder) = 0;
        virtual std::future<BasicSuggestions<CharT>> ListSuggestions(BasicCommandContext<CharT, S>& context, BasicSuggestionsBuilder<CharT>& builder) = 0;

        virtual CommandNodeType GetNodeType() = 0;
    protected:
        template<typename, typename, typename, typename>
        friend class BasicArgumentBuilder;
        template<typename, typename>
        friend class BasicMultiArgumentBuilder;
        template<typename, typename>
        friend class BasicCommandDispatcher;
        template<typename, typename, typename>
        friend class BasicRequiredArgumentBuilder;
        template<typename, typename>
        friend class BasicLiteralArgumentBuilder;

        virtual bool IsValidInput(std::basic_string_view<CharT> input) = 0;
        virtual std::basic_string_view<CharT> GetSortedKey() = 0;
    private:
        std::map<std::basic_string<CharT>, std::shared_ptr<BasicCommandNode<CharT, S>>, std::less<>> children;
        std::vector<std::shared_ptr<BasicLiteralCommandNode<CharT, S>>> literals;
        std::vector<std::shared_ptr<BasicIArgumentCommandNode<CharT, S>>> arguments;
        BasicCommand<CharT, S> command = nullptr;
        Predicate<S&> requirement = nullptr;
        std::shared_ptr<BasicCommandNode<CharT, S>> redirect = nullptr;
        BasicRedirectModifier<CharT, S> modifier = nullptr;
        bool forks = false;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandNode);
}
