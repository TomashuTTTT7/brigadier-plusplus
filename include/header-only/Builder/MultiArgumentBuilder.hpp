#pragma once

#include <type_traits>
#include "../Tree/RootCommandNode.hpp"
#include "../Tree/LiteralCommandNode.hpp"
#include "../Tree/ArgumentCommandNode.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class MultiArgumentBuilder
    {
    public:
        using B = MultiArgumentBuilder<CharT, S>;
    public:
        MultiArgumentBuilder(std::vector<std::shared_ptr<CommandNode<CharT, S>>> nodes, int master = -1) : nodes(std::move(nodes)), master(master) {}
        MultiArgumentBuilder(MultiArgumentBuilder const&) = delete; // no copying. Use reference or GetThis().

        inline B* GetThis() { return this; }

        template<template<typename...> typename Next, template<typename> typename Type, typename... Args>
        auto Then(Args&&... args) {
            return Then<Next, Type<CharT>, Args...>(std::forward<Args>(args)...);
        }
        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto Then(Args&&... args)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
                }
            }

            if constexpr (std::is_same_v<Type, void>) {
                using next_node = typename Next<CharT, S>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto new_node = std::make_shared<next_node>(std::move(node_builder));
                for (auto& node : nodes) {
                    node->AddChild(new_node);
                }
                return Next<CharT, S>(std::move(new_node));
            }
            else {
                using next_node = typename Next<CharT, S, Type>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto new_node = std::make_shared<next_node>(std::move(node_builder));
                for (auto& node : nodes) {
                    node->AddChild(new_node);
                }
                return Next<CharT, S, Type>(std::move(new_node));
            }
        }

        auto Then(std::shared_ptr<LiteralCommandNode<CharT, S>> argument)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
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
        auto Then(std::shared_ptr<ArgumentCommandNode<CharT, S, T>> argument)
        {
            for (auto& node : nodes) {
                if (node->redirect != nullptr) {
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
                }
            }
            for (auto& node : nodes) {
                if (argument != nullptr) {
                    node->AddChild(argument);
                }
            }
            return GetBuilder(std::move(argument));
        }

        B& Executes(Command<CharT, S> command, bool only_master = true)
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

        inline auto Redirect(std::shared_ptr<CommandNode<CharT, S>> target, bool only_master = true)
        {
            return Forward(std::move(target), nullptr, false, only_master);
        }

        inline auto Redirect(std::shared_ptr<CommandNode<CharT, S>> target, SingleRedirectModifier<CharT, S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, false, only_master);
        }

        inline auto Fork(std::shared_ptr<CommandNode<CharT, S>> target, SingleRedirectModifier<CharT, S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier ? [modifier](CommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, true, only_master);
        }

        inline auto Fork(std::shared_ptr<CommandNode<CharT, S>> target, RedirectModifier<CharT, S> modifier, bool only_master = true)
        {
            return Forward(std::move(target), modifier, true, only_master);
        }

        void Forward(std::shared_ptr<CommandNode<CharT, S>> target, RedirectModifier<CharT, S> modifier, bool fork, bool only_master = true)
        {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (master == -1 || master == i || !only_master) {
                    auto& node = nodes[i];
                    if (node->GetChildren().size() > 0)
                    {
                        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot forward a node with children");
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
        std::vector<std::shared_ptr<CommandNode<CharT, S>>> nodes;
        int master = -1;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(MultiArgumentBuilder);

    // multi builder
    template<typename CharT, typename S>
    inline MultiArgumentBuilder<CharT, S> GetBuilder(std::vector<std::shared_ptr<CommandNode<CharT, S>>> nodes, int master = -1)
    {
        return MultiArgumentBuilder<CharT, S>(std::move(nodes), master);
    }
}
