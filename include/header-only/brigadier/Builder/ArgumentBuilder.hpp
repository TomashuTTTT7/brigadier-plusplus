#pragma once

#include "MultiArgumentBuilder.hpp"

namespace brigadier
{
    template<typename S, typename B, typename NodeType>
    class ArgumentBuilder
    {
    public:
        using node_type = NodeType;
    public:
        ArgumentBuilder(std::shared_ptr<node_type> node)
        {
            if (node) this->node = std::move(node);
            else throw std::runtime_error("Cannot build empty node");
        }
        ArgumentBuilder(ArgumentBuilder const&) = delete; // no copying. Use reference or GetThis().

        inline B* GetThis() { return static_cast<B*>(this); }

        inline std::shared_ptr<node_type> GetNode() const { return node; }
        inline std::shared_ptr<CommandNode<S>> GetCommandNode() const { return std::static_pointer_cast<CommandNode<S>>(node); }
        inline operator std::shared_ptr<node_type>() const { return GetNode(); }
        inline operator std::shared_ptr<CommandNode<S>>() const { return GetCommandNode(); }

        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto Then(Args&&... args)
        {
            if (node->redirect != nullptr) {
                throw std::runtime_error("Cannot add children to a redirected node");
            }

            if constexpr (std::is_same_v<Type, void>) {
                using next_node = typename Next<S>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = node->children.find(name);
                if (arg == node->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    node->AddChild(new_node);
                    return Next<S>(std::move(new_node));
                }
                else {
                    auto& arg_ptr = arg->second;
                    if (arg_ptr) {
                        if (node_builder.GetNodeType() != arg_ptr->GetNodeType()) {
                            throw std::runtime_error("Node type (literal/argument) mismatch!");
                        }
                    }
                    return Next<S>(std::static_pointer_cast<next_node>(arg_ptr));
                }
            }
            else
            {
                using next_node = typename Next<S, Type>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = node->children.find(name);
                if (arg == node->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    node->AddChild(new_node);
                    return Next<S, Type>(std::move(new_node));
                }
                else {
                    auto& arg_ptr = arg->second;
                    if (arg_ptr) {
                        if (node_builder.GetNodeType() != arg_ptr->GetNodeType()) {
                            throw std::runtime_error("Node type (literal/argument) mismatch!");
                        }
                    }
                    return Next<S, Type>(std::static_pointer_cast<next_node>(arg_ptr));
                }
            }
        }

        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto ThenOptional(Args&&... args)
        {
            auto opt = Then<Next, Type, Args...>(std::forward<Args>(args)...);
            return MultiArgumentBuilder<S>({ opt.GetCommandNode(), GetCommandNode() }, 0);
        }

        auto Then(std::shared_ptr<LiteralCommandNode<S>> argument)
        {
            if (node->redirect != nullptr) {
                throw std::runtime_error("Cannot add children to a redirected node");
            }
            if (argument != nullptr) {
                node->AddChild(std::move(argument));
            }
            return GetBuilder(std::move(node->GetChild(argument)));
        }

        template<typename T>
        auto Then(std::shared_ptr<ArgumentCommandNode<S, T>> argument)
        {
            if (node->redirect != nullptr) {
                throw std::runtime_error("Cannot add children to a redirected node");
            }
            if (argument != nullptr) {
                node->AddChild(std::move(argument));
            }
            return GetBuilder(std::move(node->GetChild(argument)));
        }

        B& Executes(Command<S> command)
        {
            node->command = command;
            return *GetThis();
        }

        B& Requires(Predicate<S&> requirement)
        {
            node->requirement = requirement;
            return *GetThis();
        }

        inline auto Redirect(std::shared_ptr<CommandNode<S>> target)
        {
            return Forward(std::move(target), nullptr, false);
        }

        inline auto Redirect(std::shared_ptr<CommandNode<S>> target, SingleRedirectModifier<S> modifier)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<S>& context) -> std::vector<S> {
                return { modifier(context) }; } : nullptr, false);
        }

        inline auto Fork(std::shared_ptr<CommandNode<S>> target, SingleRedirectModifier<S> modifier)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<S>& context) -> std::vector<S> {
                return { modifier(context) }; } : nullptr, true);
        }

        inline auto Fork(std::shared_ptr<CommandNode<S>> target, RedirectModifier<S> modifier)
        {
            return Forward(std::move(target), modifier, true);
        }

        void Forward(std::shared_ptr<CommandNode<S>> target, RedirectModifier<S> modifier, bool fork)
        {
            if (node->GetChildren().size() > 0)
            {
                throw std::runtime_error("Cannot forward a node with children");
            }
            node->redirect = std::move(target);
            node->modifier = modifier;
            node->forks = fork;
        }
    protected:
        template<typename _S, typename _B>
        friend class RequiredArgumentBuilder;

        std::shared_ptr<node_type> node;
    };
}
