#pragma once

#include <memory>
#include <optional>

#include "ParsedArgument.hpp"
#include "ParsedCommandNode.hpp"
#include "SuggestionContext.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class CommandDispatcher;
    template<typename CharT, typename S>
    class LiteralCommandNode;
    template<typename CharT, typename S>
    class CommandNode;
    template<typename CharT, typename S, typename T>
    class ArgumentCommandNode;
    namespace detail
    {
        template<typename CharT, typename S>
        class CommandContextInternal;
    }

    template<typename CharT, typename S>
    class CommandContext
    {
    public:
        CommandContext(S source, CommandNode<CharT, S>* root, size_t start) : source(std::move(source)), context(std::make_unique<detail::CommandContextInternal<CharT, S>>(root, start)) {}
        CommandContext(S source, CommandNode<CharT, S>* root, StringRange range) : source(std::move(source)), context(std::make_unique<detail::CommandContextInternal<CharT, S>>(root, range)) {}

        inline CommandContext<CharT, S> GetFor(S source) const;
        inline CommandContext<CharT, S>* GetChild() const;
        inline CommandContext<CharT, S>* GetLastChild() const;
        inline CommandContext<CharT, S>* GetParent() const;
        inline CommandContext<CharT, S>* GetLastParent() const;
        inline Command<CharT, S> GetCommand() const;
        inline S& GetSource();
        inline S const& GetSource() const;
        inline RedirectModifier<CharT, S> GetRedirectModifier() const;
        inline StringRange GetRange() const;
        inline std::basic_string_view<CharT> GetInput() const;
        inline CommandNode<CharT, S>* GetRootNode() const;
        inline std::vector<ParsedCommandNode<CharT, S>>& GetNodes() const;
    protected:
        inline detail::CommandContextInternal<CharT, S>* GetInternalContext() const;
    public:
        inline bool HasNodes() const;
        inline bool IsForked() const;

        template<template<typename> typename ArgType>
        typename ArgType<CharT>::type GetArgument(std::basic_string_view<CharT> name);
        template<template<typename> typename ArgType>
        typename ArgType<CharT>::type GetArgumentOr(std::basic_string_view<CharT> name, typename ArgType<CharT>::type default_value);

        ~CommandContext();
    protected:
        inline CommandContext<CharT, S>& WithInput(std::basic_string_view<CharT> input);
        inline CommandContext<CharT, S>& WithSource(S source);
        inline CommandContext<CharT, S>& WithArgument(std::basic_string_view<CharT> name, std::shared_ptr<IParsedArgument<CharT, S>> argument);
        inline CommandContext<CharT, S>& WithCommand(Command<CharT, S> command);
        inline CommandContext<CharT, S>& WithNode(CommandNode<CharT, S>* node, StringRange range);
        inline CommandContext<CharT, S>& WithChildContext(CommandContext<CharT, S> childContext);

        inline void Reset()
        {
            detail::CommandContextInternal<CharT, S>& ctx = *context;
            input = {};
            ctx.arguments.clear();
            ctx.command = nullptr;
            ctx.nodes.clear();
            ctx.parent = nullptr;
            ctx.child = {};
            ctx.modifier = nullptr;
            ctx.forks = false;
        }
        inline void Reset(S src, CommandNode<CharT, S>* root)
        {
            Reset();
            source.~S();
            ::new (&source)S(std::move(src));
            detail::CommandContextInternal<CharT, S>& ctx = *context;
            ctx.rootNode = root;
        }
    public:
        inline void Reset(S source, CommandNode<CharT, S>* root, size_t start)
        {
            Reset(source, root);
            detail::CommandContextInternal<CharT, S>& ctx = *context;
            ctx.range = StringRange::At(start);
        }
        inline void Reset(S source, CommandNode<CharT, S>* root, StringRange range)
        {
            Reset(source, root);
            detail::CommandContextInternal<CharT, S>& ctx = *context;
            ctx.range = std::move(range);
        }
    protected:
        SuggestionContext<CharT, S> FindSuggestionContext(size_t cursor);

        void Merge(CommandContext<CharT, S> other);
    private:
        friend class CommandDispatcher<CharT, S>;
        friend class LiteralCommandNode<CharT, S>;
        friend class CommandNode<CharT, S>;
        template<typename, typename, typename>
        friend class ArgumentCommandNode;

        S source;
        std::basic_string_view<CharT> input = {};
        std::shared_ptr<detail::CommandContextInternal<CharT, S>> context = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandContext);

    namespace detail
    {
        template<typename CharT, typename S>
        class CommandContextInternal
        {
        public:
            CommandContextInternal(CommandNode<CharT, S>* root, size_t start) : rootNode(root), range(StringRange::At(start)) {}
            CommandContextInternal(CommandNode<CharT, S>* root, StringRange range) : rootNode(root), range(std::move(range)) {}
        protected:
            friend class CommandContext<CharT, S>;

            std::map<std::basic_string<CharT>, std::shared_ptr<IParsedArgument<CharT, S>>, std::less<>> arguments;
            Command<CharT, S> command = nullptr;
            CommandNode<CharT, S>* rootNode = nullptr;
            std::vector<ParsedCommandNode<CharT, S>> nodes;
            StringRange range;
            CommandContext<CharT, S>* parent = nullptr;
            std::optional<CommandContext<CharT, S>> child = {};
            RedirectModifier<CharT, S> modifier = nullptr;
            bool forks = false;
        };
        BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandContextInternal);
    }

    template<typename CharT, typename S>
    template<template<typename> typename ArgType>
    typename ArgType<CharT>::type CommandContext<CharT, S>::GetArgument(std::basic_string_view<CharT> name)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "No such argument '") << std::basic_string<CharT>(name) << BRIGADIER_LITERAL(CharT, "' exists on this command");
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<ArgType<CharT>>())) {
            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Argument '") << std::basic_string<CharT>(name) << BRIGADIER_LITERAL(CharT, "' has been acquired using wrong type");
        }

        return ((ParsedArgument<CharT, S, ArgType<CharT>>*)parsed.get())->GetResult();
    }

    template<typename CharT, typename S>
    template<template<typename> typename ArgType>
    typename ArgType<CharT>::type CommandContext<CharT, S>::GetArgumentOr(std::basic_string_view<CharT> name, typename ArgType<CharT>::type default_value)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            return std::move(default_value);
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<ArgType<CharT>>())) {
            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Argument '") << std::basic_string<CharT>(name) << BRIGADIER_LITERAL(CharT, "' has been acquired using wrong type");
        }

        return ((ParsedArgument<CharT, S, ArgType<CharT>>*)parsed.get())->GetResult();
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S> CommandContext<CharT, S>::GetFor(S source) const
    {
        CommandContext<CharT, S> result = *this;
        result.source = std::move(source);
        result.input = input;
        return result;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>* CommandContext<CharT, S>::GetChild() const
    {
        if (context->child.has_value())
            return &context->child.value();
        else return nullptr;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>* CommandContext<CharT, S>::GetParent() const
    {
        return context->parent;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>* CommandContext<CharT, S>::GetLastParent() const
    {
        CommandContext<CharT, S>* result = this;
        while (result->GetParent() != nullptr) {
            result = result->GetParent();
        }
        return result;
    }

    template<typename CharT, typename S>
    inline detail::CommandContextInternal<CharT, S>* CommandContext<CharT, S>::GetInternalContext() const
    {
        return context.get();
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>* CommandContext<CharT, S>::GetLastChild() const
    {
        CommandContext<CharT, S>* result = this;
        while (result->GetChild() != nullptr) {
            result = result->GetChild();
        }
        return result;
    }

    template<typename CharT, typename S>
    inline Command<CharT, S> CommandContext<CharT, S>::GetCommand() const
    {
        return context->command;
    }


    template<typename CharT, typename S>
    inline S& CommandContext<CharT, S>::GetSource()
    {
        return source;
    }

    template<typename CharT, typename S>
    inline S const& CommandContext<CharT, S>::GetSource() const
    {
        return source;
    }

    template<typename CharT, typename S>
    inline RedirectModifier<CharT, S> CommandContext<CharT, S>::GetRedirectModifier() const
    {
        return context->modifier;
    }

    template<typename CharT, typename S>
    inline StringRange CommandContext<CharT, S>::GetRange() const
    {
        return context->range;
    }

    template<typename CharT, typename S>
    inline std::basic_string_view<CharT> CommandContext<CharT, S>::GetInput() const
    {
        return context->input;
    }

    template<typename CharT, typename S>
    inline CommandNode<CharT, S>* CommandContext<CharT, S>::GetRootNode() const
    {
        return context->rootNode;
    }

    template<typename CharT, typename S>
    inline std::vector<ParsedCommandNode<CharT, S>>& CommandContext<CharT, S>::GetNodes() const
    {
        return context->nodes;
    }

    template<typename CharT, typename S>
    inline bool CommandContext<CharT, S>::HasNodes() const
    {
        return context->nodes.size() > 0;
    }

    template<typename CharT, typename S>
    inline bool CommandContext<CharT, S>::IsForked() const
    {
        return context->forks;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>::~CommandContext()
    {
        if (context)
        {
            if (context->child.has_value())
            {
                context->child->context->parent = nullptr;
            }
        }
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithInput(std::basic_string_view<CharT> input)
    {
        this->input = std::move(input);
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithSource(S source)
    {
        context->source = std::move(source);
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithArgument(std::basic_string_view<CharT> name, std::shared_ptr<IParsedArgument<CharT, S>> argument)
    {
        context->arguments.emplace(name, std::move(argument));
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithCommand(Command<CharT, S> command)
    {
        context->command = command;
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithNode(CommandNode<CharT, S>* node, StringRange range)
    {
        if (node)
        {
            context->nodes.emplace_back(node, range);
            context->range = StringRange::Encompassing(context->range, range);
            context->modifier = node->GetRedirectModifier();
            context->forks = node->IsFork();
        }
        return *this;
    }

    template<typename CharT, typename S>
    inline CommandContext<CharT, S>& CommandContext<CharT, S>::WithChildContext(CommandContext<CharT, S> childContext)
    {
        context->child = childContext;
        context->child->context->parent = this;
        return *this;
    }

    template<typename CharT, typename S>
    SuggestionContext<CharT, S> CommandContext<CharT, S>::FindSuggestionContext(size_t cursor)
    {
        auto& ctx = context;
        if (ctx->range.GetStart() <= cursor) {
            if (ctx->range.GetEnd() < cursor) {
                if (ctx->child.has_value()) {
                    return ctx->child->FindSuggestionContext(cursor);
                }
                else if (HasNodes()) {
                    auto& last = ctx->nodes.back();
                    return SuggestionContext<CharT, S>(last.GetNode(), last.GetRange().GetEnd() + 1);
                }
                else {
                    return SuggestionContext<CharT, S>(ctx->rootNode, ctx->range.GetStart());
                }
            }
            else {
                auto prev = ctx->rootNode;
                for (auto& node : ctx->nodes) {
                    auto nodeRange = node.GetRange();
                    if (nodeRange.GetStart() <= cursor && cursor <= nodeRange.GetEnd()) {
                        return SuggestionContext<CharT, S>(prev, nodeRange.GetStart());
                    }
                    prev = node.GetNode();
                }
                if (prev == nullptr) {
                    throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Can't find node before cursor");
                }
                return SuggestionContext<CharT, S>(prev, ctx->range.GetStart());
            }
        }
        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Can't find node before cursor");
    }

    template<typename CharT, typename S>
    void CommandContext<CharT, S>::Merge(CommandContext<CharT, S> other)
    {
        detail::CommandContextInternal<CharT, S>* ctx = other.GetInternalContext();
        for (auto& arg : ctx->arguments)
            context->arguments.emplace(std::move(arg));
        context->command = std::move(ctx->command);
        source = std::move(other.source);
        context->nodes.reserve(context->nodes.size() + ctx->nodes.size());
        for (auto& node : ctx->nodes)
        {
            context->range = StringRange::Encompassing(context->range, ctx->range);
            context->modifier = node.GetNode()->GetRedirectModifier();
            context->forks = node.GetNode()->IsFork();

            context->nodes.emplace_back(std::move(node));
        }
        if (ctx->child.has_value())
        {
            ctx->child->context->parent = this;
            if (context->child.has_value())
            {
                context->child->Merge(std::move(*ctx->child));
            }
            else context->child = std::move(ctx->child);
        }
    }
}