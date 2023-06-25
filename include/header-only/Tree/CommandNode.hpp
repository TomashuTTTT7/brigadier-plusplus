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
    class CommandNode;
    template<typename CharT, typename S>
    class IArgumentCommandNode;
    template<typename CharT, typename S>
    class LiteralCommandNode;
    template<typename CharT, typename S>
    class RootCommandNode;
    template<typename CharT, typename S>
    class RootCommandNode;

    template<typename CharT, typename S, typename T, typename node_type>
    class ArgumentBuilder;
    template<typename CharT, typename S>
    class MultiArgumentBuilder;
    template<typename CharT, typename S>
    class LiteralArgumentBuilder;
    template<typename CharT, typename S, typename T>
    class RequiredArgumentBuilder;

    template<typename CharT, typename S>
    class CommandContext;

    enum class CommandNodeType
    {
        RootCommandNode,
        LiteralCommandNode,
        ArgumentCommandNode
    };

    template<typename CharT, typename S>
    class CommandNode
    {
    public:
        CommandNode(Command<CharT, S> command, Predicate<S&> requirement, CommandNode<CharT, S>* redirect, RedirectModifier<CharT, S> modifier, const bool forks)
            : command(std::move(command))
            , requirement(std::move(requirement))
            , redirect(redirect)
            , modifier(std::move(modifier))
            , forks(std::move(forks))
        {}
        CommandNode() {}
        explicit CommandNode(CommandNode<CharT, S>&) = default;
        virtual ~CommandNode() = default;
    public:
        inline Command<CharT, S> GetCommand() const
        {
            return command;
        }

        inline std::map<std::basic_string<CharT>, std::shared_ptr<CommandNode<CharT, S>>, std::less<>> const& GetChildren() const
        {
            return children;
        }

        inline std::shared_ptr<CommandNode<CharT, S>> GetChild(std::basic_string_view<CharT> name) const
        {
            auto found = children.find(name);
            if (found != children.end())
                return found->second;
            return nullptr;
        }

        inline CommandNode<CharT, S>* GetRedirect() const
        {
            return redirect;
        }

        inline RedirectModifier<CharT, S> GetRedirectModifier() const 
        {
            return modifier;
        }

        inline Predicate<S&> GetRequirement() const
        {
            return requirement;
        }

        inline bool IsFork() const
        {
            return forks;
        }

        inline bool CanUse(S& source) const
        {
            if (requirement)
                return requirement(source);
            else return true;
        }

        void AddChild(std::shared_ptr<CommandNode<CharT, S>> node)
        {
            if (node == nullptr)
                return;

            if (node->GetNodeType() == CommandNodeType::RootCommandNode) {
                throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add a RootCommandNode as a child to any other CommandNode");
            }

            auto child = children.find(node->GetName());
            if (child != children.end()) {
                // We've found something to merge onto
                auto child_node = child->second;

                if (child_node->GetNodeType() != node->GetNodeType())
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");

                auto node_command = node->GetCommand();
                if (node_command != nullptr) {
                    child_node->command = node_command;
                }
                for (auto& [name, grandchild] : node->GetChildren()) {
                    child_node->AddChild(grandchild);
                }
            }
            else {
                node->parent = this;
                children.emplace(node->GetName(), node);
                if (node->GetNodeType() == CommandNodeType::LiteralCommandNode) {
                    literals.emplace_back(std::move(std::static_pointer_cast<LiteralCommandNode<CharT, S>>(std::move(node))));
                }
                else if (node->GetNodeType() == CommandNodeType::ArgumentCommandNode) {
                    arguments.emplace_back(std::move(std::static_pointer_cast<IArgumentCommandNode<CharT, S>>(std::move(node))));
                }
            }
        }

        /**
        Scans the command tree for potential ambiguous commands.

        This is a shortcut for CommandNode::FindAmbiguities(AmbiguityConsumer) on GetRoot().

        Ambiguities are detected by testing every CommandNode::GetExamples() on one node verses every sibling
        node. This is not fool proof, and relies a lot on the providers of the used argument types to give good examples.

        \param consumer a callback to be notified of potential ambiguities
        */
        void FindAmbiguities(AmbiguityConsumer<CharT, S> consumer) const
        {
            if (!consumer)
                return;

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

        std::tuple<std::shared_ptr<CommandNode<CharT, S>> const*, size_t> GetRelevantNodes(StringReader<CharT>& input) const
        {
            if (literals.size() > 0) {
                size_t cursor = input.GetCursor();
                while (input.CanRead() && input.Peek() != ' ') {
                    input.Skip();
                }
                std::basic_string_view<CharT> text = input.GetString().substr(cursor, input.GetCursor() - cursor);
                input.SetCursor(cursor);
                auto literal = children.find(text);
                if (literal != children.end() && literal->second->GetNodeType() == CommandNodeType::LiteralCommandNode) {
                    return std::tuple<std::shared_ptr<CommandNode<CharT, S>> const*, size_t>(&literal->second, 1);
                }
                else {
                    return std::tuple<std::shared_ptr<CommandNode<CharT, S>> const*, size_t>((std::shared_ptr<CommandNode<CharT, S>>*)arguments.data(), arguments.size());
                }
            }
            else {
                return std::tuple<std::shared_ptr<CommandNode<CharT, S>> const*, size_t>((std::shared_ptr<CommandNode<CharT, S>>*)arguments.data(), arguments.size());
            }
        }

        bool HasCommand() const
        {
            if (GetCommand() != nullptr) return true;
            for (auto [name, child] : children)
                if (child && child->HasCommand())
                    return true;
            return false;
        }
    public:
        virtual std::basic_string<CharT> const& GetName() const = 0;
        virtual std::basic_string<CharT> GetUsageText() const = 0;
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() const = 0;
        virtual void Parse(StringReader<CharT>& reader, CommandContext<CharT, S>& contextBuilder) const = 0;
        virtual std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder) const = 0;

        virtual CommandNodeType GetNodeType() const = 0;
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) const = 0;
        virtual std::basic_string_view<CharT> GetSortedKey() const = 0;
    public:
        template<template<typename> typename Type, typename... Args>
        auto& Then(std::basic_string_view<CharT> name, Args&&... args) {
            return Then<Type<CharT>, Args...>(name, std::forward<Args>(args)...);
        }
        template<typename Type = void, typename... Args>
        auto& Then(std::basic_string_view<CharT> name, Args&&... args);

        //auto Then(std::shared_ptr<LiteralCommandNode<CharT, S>> argument)
        //{
        //    if (node->redirect != nullptr) {
        //        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
        //    }
        //    if (argument != nullptr) {
        //        node->AddChild(std::move(argument));
        //    }
        //    return GetBuilder<CharT, S>(std::move(node->GetChild(argument)));
        //}

        //template<typename T>
        //auto Then(std::shared_ptr<ArgumentCommandNode<CharT, S, T>> argument)
        //{
        //    if (node->redirect != nullptr) {
        //        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
        //    }
        //    if (argument != nullptr) {
        //        node->AddChild(std::move(argument));
        //    }
        //    return GetBuilder<CharT, S>(std::move(node->GetChild(argument)));
        //}

        auto Executes(Command<CharT, S> command) -> decltype(*this)
        {
            this->command = command;
            return *this;
        }

        auto Requires(Predicate<S&> requirement) -> decltype(*this)
        {
            this->requirement = requirement;
            return *this;
        }

        CommandNode<CharT, S>& Optional()
        {
            if (!parent) {
                throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot make optional of RootCommandNode node");
            }
            this->command = parent->command; // TODO wrapper to call parent's command so it will call the correct one if parent's one gets updated, should we even care?
            Redirect(*parent);
            return *parent;
        }

        inline auto Redirect(CommandNode<CharT, S>& target)
        {
            return Forward(target, nullptr, false);
        }

        inline auto Redirect(CommandNode<CharT, S>& target, SingleRedirectModifier<CharT, S> modifier)
        {
            return Forward(target, modifier ? [modifier](CommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, false);
        }

        inline auto Fork(CommandNode<CharT, S>& target, SingleRedirectModifier<CharT, S> modifier)
        {
            return Forward(target, modifier ? [modifier](CommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, true);
        }

        inline auto Fork(CommandNode<CharT, S>& target, RedirectModifier<CharT, S> modifier)
        {
            return Forward(target, modifier, true);
        }

        void Forward(CommandNode<CharT, S>& target, RedirectModifier<CharT, S> modifier, bool fork)
        {
            if (GetChildren().size() > 0)
            {
                throw RuntimeError<CharT>() << "Cannot forward a node with children";
            }
            this->redirect = &target;
            this->modifier = modifier;
            this->forks = fork;
        }
    private:
        CommandNode<CharT, S>* parent = nullptr;
        std::map<std::basic_string<CharT>, std::shared_ptr<CommandNode<CharT, S>>, std::less<>> children;
        std::vector<std::shared_ptr<LiteralCommandNode<CharT, S>>> literals;
        std::vector<std::shared_ptr<IArgumentCommandNode<CharT, S>>> arguments;
        Command<CharT, S> command = nullptr;
        Predicate<S&> requirement = nullptr;
        CommandNode<CharT, S>* redirect = nullptr;
        RedirectModifier<CharT, S> modifier = nullptr;
        bool forks = false;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandNode);
}
