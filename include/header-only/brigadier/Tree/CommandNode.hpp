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
    template<typename S>
    class CommandNode;
    template<typename S>
    class IArgumentCommandNode;
    template<typename S>
    class LiteralCommandNode;
    template<typename S>
    class RootCommandNode;
    template<typename S>
    class CommandDispatcher;

    template<typename S, typename T, typename node_type>
    class ArgumentBuilder;
    template<typename S>
    class MultiArgumentBuilder;
    template<typename S>
    class LiteralArgumentBuilder;
    template<typename S, typename T>
    class RequiredArgumentBuilder;

    template<typename S>
    class CommandContext;

    enum class CommandNodeType
    {
        RootCommandNode,
        LiteralCommandNode,
        ArgumentCommandNode
    };

    template<typename S>
    class CommandNode
    {
    public:
        CommandNode(Command<S> command, Predicate<S&> requirement, std::shared_ptr<CommandNode<S>> redirect, RedirectModifier<S> modifier, const bool forks)
            : command(std::move(command))
            , requirement(std::move(requirement))
            , redirect(std::move(redirect))
            , modifier(std::move(modifier))
            , forks(std::move(forks))
        {}
        CommandNode() {}
        virtual ~CommandNode() = default;
    public:
        inline Command<S> GetCommand() const
        {
            return command;
        }

        inline std::map<std::string, std::shared_ptr<CommandNode<S>>, std::less<>> const& GetChildren() const
        {
            return children;
        }

        inline std::shared_ptr<CommandNode<S>> GetChild(std::string_view name) const
        {
            auto found = children.find(name);
            if (found != children.end())
                return found->second;
            return nullptr;
        }

        inline std::shared_ptr<CommandNode<S>> GetRedirect() const
        {
            return redirect;
        }

        inline RedirectModifier<S> GetRedirectModifier() const 
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

        void AddChild(std::shared_ptr<CommandNode<S>> node)
        {
            if (node == nullptr)
                return;

            if (node->GetNodeType() == CommandNodeType::RootCommandNode) {
                throw std::runtime_error("Cannot add a RootCommandNode as a child to any other CommandNode");
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
                if (node->GetNodeType() == CommandNodeType::LiteralCommandNode) {
                    literals.emplace_back(std::move(std::static_pointer_cast<LiteralCommandNode<S>>(std::move(node))));
                }
                else if (node->GetNodeType() == CommandNodeType::ArgumentCommandNode) {
                    arguments.emplace_back(std::move(std::static_pointer_cast<IArgumentCommandNode<S>>(std::move(node))));
                }
            }
        }

        void FindAmbiguities(AmbiguityConsumer<S> consumer)
        {
            for (auto [child_name, child] : children) {
                for (auto [sibling_name, sibling] : children) {
                    if (child == sibling)
                        continue;

                    std::set<std::string_view> matches;

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

        std::tuple<std::shared_ptr<CommandNode<S>>*, size_t> GetRelevantNodes(StringReader& input)
        {
            if (literals.size() > 0) {
                int cursor = input.GetCursor();
                while (input.CanRead() && input.Peek() != ' ') {
                    input.Skip();
                }
                std::string_view text = input.GetString().substr(cursor, input.GetCursor() - cursor);
                input.SetCursor(cursor);
                auto literal = children.find(text);
                if (literal != children.end() && literal->second->GetNodeType() == CommandNodeType::LiteralCommandNode) {
                    return std::tuple<std::shared_ptr<CommandNode<S>>*, size_t>(&literal->second, 1);
                }
                else {
                    return std::tuple<std::shared_ptr<CommandNode<S>>*, size_t>((std::shared_ptr<CommandNode<S>>*)arguments.data(), arguments.size());
                }
            }
            else {
                return std::tuple<std::shared_ptr<CommandNode<S>>*, size_t>((std::shared_ptr<CommandNode<S>>*)arguments.data(), arguments.size());
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
        virtual std::string const& GetName() = 0;
        virtual std::string GetUsageText() = 0;
        virtual std::vector<std::string_view> GetExamples() = 0;
        virtual void Parse(StringReader& reader, CommandContext<S>& contextBuilder) = 0;
        virtual std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder) = 0;

        virtual CommandNodeType GetNodeType() = 0;
    protected:
        template<typename _S, typename T, typename node_type>
        friend class ArgumentBuilder;
        template<typename _S>
        friend class MultiArgumentBuilder;
        template<typename _S>
        friend class CommandDispatcher;
        template<typename _S, typename T>
        friend class RequiredArgumentBuilder;
        template<typename _S>
        friend class LiteralArgumentBuilder;

        virtual bool IsValidInput(std::string_view input) = 0;
        virtual std::string_view GetSortedKey() = 0;
    private:
        std::map<std::string, std::shared_ptr<CommandNode<S>>, std::less<>> children;
        std::vector<std::shared_ptr<LiteralCommandNode<S>>> literals;
        std::vector<std::shared_ptr<IArgumentCommandNode<S>>> arguments;
        Command<S> command = nullptr;
        Predicate<S&> requirement = nullptr;
        std::shared_ptr<CommandNode<S>> redirect = nullptr;
        RedirectModifier<S> modifier = nullptr;
        bool forks = false;
    };
}
