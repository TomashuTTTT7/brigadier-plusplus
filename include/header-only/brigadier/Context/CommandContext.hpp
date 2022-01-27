#pragma once

#include <memory>
#include <optional>

#include "ParsedArgument.hpp"
#include "ParsedCommandNode.hpp"
#include "SuggestionContext.hpp"

namespace brigadier
{
    template<typename S>
    class CommandDispatcher;
    template<typename S>
    class LiteralCommandNode;
    template<typename S>
    class CommandNode;
    template<typename S, typename T>
    class ArgumentCommandNode;
    namespace detail
    {
        template<typename S>
        class CommandContextInternal;
    }

    template<typename S>
    class CommandContext
    {
    public:
        CommandContext(S source, CommandNode<S>* root, int start) : source(std::move(source)), context(std::make_unique<detail::CommandContextInternal<S>>(root, start)) {}
        CommandContext(S source, CommandNode<S>* root, StringRange range) : source(std::move(source)), context(std::make_unique<detail::CommandContextInternal<S>>(root, range)) {}

        inline CommandContext<S> GetFor(S source) const;
        inline CommandContext<S>* GetChild() const;
        inline CommandContext<S>* GetLastChild() const;
        inline CommandContext<S>* GetParent() const;
        inline CommandContext<S>* GetLastParent() const;
        inline Command<S> GetCommand() const;
        inline S& GetSource();
        inline S const& GetSource() const;
        inline RedirectModifier<S> GetRedirectModifier() const;
        inline StringRange GetRange() const;
        inline std::string_view GetInput() const;
        inline CommandNode<S>* GetRootNode() const;
        inline std::vector<ParsedCommandNode<S>>& GetNodes() const;
    protected:
        inline detail::CommandContextInternal<S>* GetInternalContext() const;
    public:
        inline bool HasNodes() const;
        inline bool IsForked() const;

        template<typename ArgType>
        typename ArgType::type GetArgument(std::string_view name);
        template<typename ArgType>
        typename ArgType::type GetArgumentOr(std::string_view name, typename ArgType::type default_value);

        ~CommandContext();
    protected:
        inline CommandContext<S>& WithInput(std::string_view input);
        inline CommandContext<S>& WithSource(S source);
        inline CommandContext<S>& WithArgument(std::string_view name, std::shared_ptr<IParsedArgument<S>> argument);
        inline CommandContext<S>& WithCommand(Command<S> command);
        inline CommandContext<S>& WithNode(CommandNode<S>* node, StringRange range);
        inline CommandContext<S>& WithChildContext(CommandContext<S> childContext);

        inline void Reset()
        {
            detail::CommandContextInternal<S>& ctx = *context;
            input = {};
            ctx.arguments.clear();
            ctx.command = nullptr;
            ctx.nodes.clear();
            ctx.parent = nullptr;
            ctx.child = {};
            ctx.modifier = nullptr;
            ctx.forks = false;
        }
        inline void Reset(S src, CommandNode<S>* root)
        {
            Reset();
            source = std::move(src);
            detail::CommandContextInternal<S>& ctx = *context;
            ctx.rootNode = root;
        }
    public:
        inline void Reset(S source, CommandNode<S>* root, int start)
        {
            Reset(source, root);
            detail::CommandContextInternal<S>& ctx = *context;
            ctx.range = StringRange::At(start);
        }
        inline void Reset(S source, CommandNode<S>* root, StringRange range)
        {
            Reset(source, root);
            detail::CommandContextInternal<S>& ctx = *context;
            ctx.range = std::move(range);
        }
    protected:
        SuggestionContext<S> FindSuggestionContext(int cursor);

        void Merge(CommandContext<S> other);
    private:
        friend class CommandDispatcher<S>;
        friend class LiteralCommandNode<S>;
        friend class CommandNode<S>;
        template<typename _S, typename T>
        friend class ArgumentCommandNode;

        S source;
        std::string_view input = {};
        std::shared_ptr<detail::CommandContextInternal<S>> context = nullptr;
    };

    namespace detail
    {
        template<typename S>
        class CommandContextInternal
        {
        public:
            CommandContextInternal(CommandNode<S>* root, int start) : rootNode(root), range(StringRange::At(start)) {}
            CommandContextInternal(CommandNode<S>* root, StringRange range) : rootNode(root), range(std::move(range)) {}
        protected:
            friend class CommandContext<S>;

            std::map<std::string, std::shared_ptr<IParsedArgument<S>>, std::less<>> arguments;
            Command<S> command = nullptr;
            CommandNode<S>* rootNode = nullptr;
            std::vector<ParsedCommandNode<S>> nodes;
            StringRange range;
            CommandContext<S>* parent = nullptr;
            std::optional<CommandContext<S>> child = {};
            RedirectModifier<S> modifier = nullptr;
            bool forks = false;
        };
    }

    template<typename S>
    template<typename ArgType>
    typename ArgType::type CommandContext<S>::GetArgument(std::string_view name)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            throw std::runtime_error("No such argument '" + std::string(name) + "' exists on this command");
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<ArgType>())) {
            throw std::runtime_error("Argument '" + std::string(name) + "' has been acquired using wrong type");
        }

        return ((ParsedArgument<S, ArgType>*)parsed.get())->GetResult();
    }

    template<typename S>
    template<typename ArgType>
    typename ArgType::type CommandContext<S>::GetArgumentOr(std::string_view name, typename ArgType::type default_value)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            return std::move(default_value);
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<ArgType>())) {
            throw std::runtime_error("Argument '" + std::string(name) + "' has been acquired using wrong type");
        }

        return ((ParsedArgument<S, ArgType>*)parsed.get())->GetResult();
    }

    template<typename S>
    inline CommandContext<S> CommandContext<S>::GetFor(S source) const
    {
        CommandContext<S> result = *this;
        result.source = std::move(source);
        result.input = input;
        return result;
    }

    template<typename S>
    inline CommandContext<S>* CommandContext<S>::GetChild() const
    {
        if (context->child.has_value())
            return &context->child.value();
        else return nullptr;
    }

    template<typename S>
    inline CommandContext<S>* CommandContext<S>::GetParent() const
    {
        return context->parent;
    }

    template<typename S>
    inline CommandContext<S>* CommandContext<S>::GetLastParent() const
    {
        CommandContext<S>* result = this;
        while (result->GetParent() != nullptr) {
            result = result->GetParent();
        }
        return result;
    }

    template<typename S>
    inline detail::CommandContextInternal<S>* CommandContext<S>::GetInternalContext() const
    {
        return context.get();
    }

    template<typename S>
    inline CommandContext<S>* CommandContext<S>::GetLastChild() const
    {
        CommandContext<S>* result = this;
        while (result->GetChild() != nullptr) {
            result = result->GetChild();
        }
        return result;
    }

    template<typename S>
    inline Command<S> CommandContext<S>::GetCommand() const
    {
        return context->command;
    }


    template<typename S>
    inline S& CommandContext<S>::GetSource()
    {
        return source;
    }

    template<typename S>
    inline S const& CommandContext<S>::GetSource() const
    {
        return source;
    }

    template<typename S>
    inline RedirectModifier<S> CommandContext<S>::GetRedirectModifier() const
    {
        return context->modifier;
    }

    template<typename S>
    inline StringRange CommandContext<S>::GetRange() const
    {
        return context->range;
    }

    template<typename S>
    inline std::string_view CommandContext<S>::GetInput() const
    {
        return context->input;
    }

    template<typename S>
    inline CommandNode<S>* CommandContext<S>::GetRootNode() const
    {
        return context->rootNode;
    }

    template<typename S>
    inline std::vector<ParsedCommandNode<S>>& CommandContext<S>::GetNodes() const
    {
        return context->nodes;
    }

    template<typename S>
    inline bool CommandContext<S>::HasNodes() const
    {
        return context->nodes.size() > 0;
    }

    template<typename S>
    inline bool CommandContext<S>::IsForked() const
    {
        return context->forks;
    }

    template<typename S>
    inline CommandContext<S>::~CommandContext()
    {
        if (context)
        {
            if (context->child.has_value())
            {
                context->child->context->parent = nullptr;
            }
        }
    }

    template<typename S>
    inline CommandContext<S>& CommandContext<S>::WithInput(std::string_view input)
    {
        this->input = std::move(input);
        return *this;
    }

    template<typename S>
    inline CommandContext<S>& CommandContext<S>::WithSource(S source)
    {
        context->source = source;
        return *this;
    }

    template<typename S>
    inline CommandContext<S>& CommandContext<S>::WithArgument(std::string_view name, std::shared_ptr<IParsedArgument<S>> argument)
    {
        context->arguments.emplace(name, std::move(argument));
        return *this;
    }

    template<typename S>
    inline CommandContext<S>& CommandContext<S>::WithCommand(Command<S> command)
    {
        context->command = command;
        return *this;
    }

    template<typename S>
    inline CommandContext<S>& CommandContext<S>::WithNode(CommandNode<S>* node, StringRange range)
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

    template<typename S>
    inline CommandContext<S>& CommandContext<S>::WithChildContext(CommandContext<S> childContext)
    {
        context->child = childContext;
        context->child->context->parent = this;
        return *this;
    }

    template<typename S>
    SuggestionContext<S> CommandContext<S>::FindSuggestionContext(int cursor)
    {
        auto& ctx = context;
        if (ctx->range.GetStart() <= cursor) {
            if (ctx->range.GetEnd() < cursor) {
                if (ctx->child.has_value()) {
                    return ctx->child->FindSuggestionContext(cursor);
                }
                else if (HasNodes()) {
                    auto& last = ctx->nodes.back();
                    return SuggestionContext<S>(last.GetNode(), last.GetRange().GetEnd() + 1);
                }
                else {
                    return SuggestionContext<S>(ctx->rootNode, ctx->range.GetStart());
                }
            }
            else {
                auto prev = ctx->rootNode;
                for (auto& node : ctx->nodes) {
                    auto nodeRange = node.GetRange();
                    if (nodeRange.GetStart() <= cursor && cursor <= nodeRange.GetEnd()) {
                        return SuggestionContext<S>(prev, nodeRange.GetStart());
                    }
                    prev = node.GetNode();
                }
                if (prev == nullptr) {
                    throw std::runtime_error("Can't find node before cursor");
                }
                return SuggestionContext<S>(prev, ctx->range.GetStart());
            }
        }
        throw std::runtime_error("Can't find node before cursor");
    }

    template<typename S>
    void CommandContext<S>::Merge(CommandContext<S> other)
    {
        detail::CommandContextInternal<S>* ctx = other.GetInternalContext();
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