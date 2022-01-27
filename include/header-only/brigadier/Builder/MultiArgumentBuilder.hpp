#pragma once

#include <type_traits>
#include "../Tree/RootCommandNode.hpp"
#include "../Tree/LiteralCommandNode.hpp"
#include "../Tree/ArgumentCommandNode.hpp"

namespace brigadier
{
    template<typename S>
    class MultiArgumentBuilder
    {
    public:
        using B = MultiArgumentBuilder<S>;
    public:
        MultiArgumentBuilder(std::vector<std::shared_ptr<CommandNode<S>>> nodes, int master = -1) : nodes(std::move(nodes)), master(master) {}
        MultiArgumentBuilder(MultiArgumentBuilder const&) = delete; // no copying. Use reference or GetThis().

        inline B* GetThis() { return this; }

        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto Then(Args&&... args)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw std::runtime_error("Cannot add children to a redirected node");
                }
            }

            if constexpr (std::is_same_v<Type, void>) {
                using next_node = typename Next<S>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto new_node = std::make_shared<next_node>(std::move(node_builder));
                for (auto& node : nodes) {
                    node->AddChild(new_node);
                }
                return Next<S>(std::move(new_node));
            }
            else {
                using next_node = typename Next<S, Type>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto new_node = std::make_shared<next_node>(std::move(node_builder));
                for (auto& node : nodes) {
                    node->AddChild(new_node);
                }
                return Next<S, Type>(std::move(new_node));
            }
        }

        auto Then(std::shared_ptr<LiteralCommandNode<S>> argument)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw std::runtime_error("Cannot add children to a redirected node");
                }
            }
            for (auto& node : nodes) {
                if (argument != nullptr) {
                    node->AddChild(argument);
                }
            }
            return GetBuilder(std::move(argument));
        }

        template<typename T>
        auto Then(std::shared_ptr<ArgumentCommandNode<S, T>> argument)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw std::runtime_error("Cannot add children to a redirected node");
                }
            }
            for (auto& node : nodes) {
                if (argument != nullptr) {
                    node->AddChild(argument);
                }
            }
            return GetBuilder(std::move(argument));
        }

        B& Executes(Command<S> command, bool only_master = true)
        {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    node->command = command;
                }
            }
            return *GetThis();
        }

        B& Requires(Predicate<S&> requirement, bool only_master = true)
        {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    node->requirement = requirement;
                }
            }
            return *GetThis();
        }

        inline auto Redirect(std::shared_ptr<CommandNode<S>> target, bool only_master = true)
        {
            return Forward(std::move(target), nullptr, false, only_master);
        }

        inline auto Redirect(std::shared_ptr<CommandNode<S>> target, SingleRedirectModifier<S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<S>& context) -> std::vector<S> {
                return { modifier(context) }; } : nullptr, false, only_master);
        }

        inline auto Fork(std::shared_ptr<CommandNode<S>> target, SingleRedirectModifier<S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<S>& context) -> std::vector<S> {
                return { modifier(context) }; } : nullptr, true, only_master);
        }

        inline auto Fork(std::shared_ptr<CommandNode<S>> target, RedirectModifier<S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier, true, only_master);
        }

        void Forward(std::shared_ptr<CommandNode<S>> target, RedirectModifier<S> modifier, bool fork, bool only_master = true)
        {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    if (node->GetChildren().size() > 0)
                    {
                        throw std::runtime_error("Cannot forward a node with children");
                    }
                }
            }
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    node->redirect = std::move(target);
                    node->modifier = modifier;
                    node->forks = fork;
                }
            }
        }
    protected:
        std::vector<std::shared_ptr<CommandNode<S>>> nodes;
        int master = -1;
    };

    // multi builder
    template<typename S>
    inline MultiArgumentBuilder<S> GetBuilder(std::vector<std::shared_ptr<CommandNode<S>>> nodes, int master = -1)
    {
        return MultiArgumentBuilder<S>(std::move(nodes), master);
    }
}
