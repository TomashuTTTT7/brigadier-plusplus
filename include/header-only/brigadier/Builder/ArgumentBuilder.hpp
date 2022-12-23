#pragma once

#include "MultiArgumentBuilder.hpp"

namespace brigadier
{
    template<typename CharT, typename S, typename B, typename NodeType>
    class BasicArgumentBuilder
    {
    public:
        using node_type = NodeType;
    public:
        BasicArgumentBuilder(std::shared_ptr<node_type> node)
        {
            if (node) this->node = std::move(node);
            else throw BasicRuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot build empty node");
        }
        BasicArgumentBuilder(BasicArgumentBuilder const&) = delete; // no copying. Use reference or GetThis().

        inline B* GetThis() { return static_cast<B*>(this); }

        inline std::shared_ptr<node_type> GetNode() const { return node; }
        inline std::shared_ptr<BasicCommandNode<CharT, S>> GetCommandNode() const { return std::static_pointer_cast<BasicCommandNode<CharT, S>>(node); }
        inline operator std::shared_ptr<node_type>() const { return GetNode(); }
        inline operator std::shared_ptr<BasicCommandNode<CharT, S>>() const { return GetCommandNode(); }

        template<template<typename...> typename Next, template<typename> typename Type, typename... Args>
        auto Then(Args&&... args) {
            return Then<Next, Type<CharT>, Args...>(std::forward<Args>(args)...);
        }
        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto Then(Args&&... args)
        {
            if (node->redirect != nullptr) {
                throw BasicRuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
            }

            if constexpr (std::is_same_v<Type, void>) {
                using next_node = typename Next<CharT, S>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = node->children.find(name);
                if (arg == node->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    node->AddChild(new_node);
                    return Next<CharT, S>(std::move(new_node));
                }
                else {
                    auto& arg_ptr = arg->second;
                    if (arg_ptr) {
                        if (node_builder.GetNodeType() != arg_ptr->GetNodeType()) {
                            throw BasicRuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");
                        }
                    }
                    return Next<CharT, S>(std::static_pointer_cast<next_node>(arg_ptr));
                }
            }
            else
            {
                using next_node = typename Next<CharT, S, Type>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = node->children.find(name);
                if (arg == node->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    node->AddChild(new_node);
                    return Next<CharT, S, Type>(std::move(new_node));
                }
                else {
                    auto& arg_ptr = arg->second;
                    if (arg_ptr) {
                        if (node_builder.GetNodeType() != arg_ptr->GetNodeType()) {
                            throw BasicRuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");
                        }
                    }
                    return Next<CharT, S, Type>(std::static_pointer_cast<next_node>(arg_ptr));
                }
            }
        }

        template<template<typename...> typename Next, template<typename> typename Type, typename... Args>
        auto ThenOptional(Args&&... args) {
            return ThenOptional<Next, Type<CharT>, Args...>(std::forward<Args>(args)...);
        }
        template<template<typename...> typename Next, typename Type = void, typename... Args>
        auto ThenOptional(Args&&... args)
        {
            auto opt = Then<Next, Type, Args...>(std::forward<Args>(args)...);
            return BasicMultiArgumentBuilder<CharT, S>({ opt.GetCommandNode(), GetCommandNode() }, 0);
        }

        //auto Then(std::shared_ptr<BasicLiteralCommandNode<CharT, S>> argument)
        //{
        //    if (node->redirect != nullptr) {
        //        throw BasicRuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
        //    }
        //    if (argument != nullptr) {
        //        node->AddChild(std::move(argument));
        //    }
        //    return GetBuilder<CharT, S>(std::move(node->GetChild(argument)));
        //}

        //template<typename T>
        //auto Then(std::shared_ptr<BasicArgumentCommandNode<CharT, S, T>> argument)
        //{
        //    if (node->redirect != nullptr) {
        //        throw BasicRuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
        //    }
        //    if (argument != nullptr) {
        //        node->AddChild(std::move(argument));
        //    }
        //    return GetBuilder<CharT, S>(std::move(node->GetChild(argument)));
        //}

        B& Executes(BasicCommand<CharT, S> command)
        {
            node->command = command;
            return *GetThis();
        }

        B& Requires(Predicate<S&> requirement)
        {
            node->requirement = requirement;
            return *GetThis();
        }

        inline auto Redirect(std::shared_ptr<BasicCommandNode<CharT, S>> target)
        {
            return Forward(std::move(target), nullptr, false);
        }

        inline auto Redirect(std::shared_ptr<BasicCommandNode<CharT, S>> target, BasicSingleRedirectModifier<CharT, S> modifier)
        {
            return Forward(std::move(target), modifier ? [modifier](BasicCommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, false);
        }

        inline auto Fork(std::shared_ptr<BasicCommandNode<CharT, S>> target, BasicSingleRedirectModifier<CharT, S> modifier)
        {
            return Forward(std::move(target), modifier ? [modifier](BasicCommandContext<CharT, S>& context) -> std::vector<CharT, S> {
                return { modifier(context) }; } : nullptr, true);
        }

        inline auto Fork(std::shared_ptr<BasicCommandNode<CharT, S>> target, BasicRedirectModifier<CharT, S> modifier)
        {
            return Forward(std::move(target), modifier, true);
        }

        void Forward(std::shared_ptr<BasicCommandNode<CharT, S>> target, BasicRedirectModifier<CharT, S> modifier, bool fork)
        {
            if (node->GetChildren().size() > 0)
            {
                throw BasicRuntimeError<CharT>() << "Cannot forward a node with children";
            }
            node->redirect = std::move(target);
            node->modifier = modifier;
            node->forks = fork;
        }
    protected:
        template<typename, typename, typename>
        friend class BasicRequiredArgumentBuilder;

        std::shared_ptr<node_type> node;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ArgumentBuilder);
}
