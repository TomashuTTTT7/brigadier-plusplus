#pragma once

#include <memory>
#include <optional>

#include "ParsedArgument.hpp"
#include "ParsedCommandNode.hpp"
#include "SuggestionContext.hpp"

namespace brigadier
{
    template<typename CharT, typename S>
    class BasicCommandDispatcher;
    template<typename CharT, typename S>
    class BasicLiteralCommandNode;
    template<typename CharT, typename S>
    class BasicCommandNode;
    template<typename CharT, typename S, typename T>
    class BasicArgumentCommandNode;
    namespace detail
    {
        template<typename CharT, typename S>
        class BasicCommandContextInternal;
    }

    template<typename CharT, typename S>
    class BasicCommandContext
    {
    public:
        BasicCommandContext(S source, BasicCommandNode<CharT, S>* root, int start) : source(std::move(source)), context(std::make_unique<detail::BasicCommandContextInternal<CharT, S>>(root, start)) {}
        BasicCommandContext(S source, BasicCommandNode<CharT, S>* root, BasicStringRange<CharT> range) : source(std::move(source)), context(std::make_unique<detail::BasicCommandContextInternal<CharT, S>>(root, range)) {}

        inline BasicCommandContext<CharT, S> GetFor(S source) const;
        inline BasicCommandContext<CharT, S>* GetChild() const;
        inline BasicCommandContext<CharT, S>* GetLastChild() const;
        inline BasicCommandContext<CharT, S>* GetParent() const;
        inline BasicCommandContext<CharT, S>* GetLastParent() const;
        inline BasicCommand<CharT, S> GetCommand() const;
        inline S& GetSource();
        inline S const& GetSource() const;
        inline BasicRedirectModifier<CharT, S> GetRedirectModifier() const;
        inline BasicStringRange<CharT> GetRange() const;
        inline std::basic_string_view<CharT> GetInput() const;
        inline BasicCommandNode<CharT, S>* GetRootNode() const;
        inline std::vector<BasicParsedCommandNode<CharT, S>>& GetNodes() const;
    protected:
        inline detail::BasicCommandContextInternal<CharT, S>* GetInternalContext() const;
    public:
        inline bool HasNodes() const;
        inline bool IsForked() const;

        template<typename ArgType>
        typename ArgType::type GetArgument(std::basic_string_view<CharT> name);
        template<typename ArgType>
        typename ArgType::type GetArgumentOr(std::basic_string_view<CharT> name, typename ArgType::type default_value);

        ~BasicCommandContext();
    protected:
        inline BasicCommandContext<CharT, S>& WithInput(std::basic_string_view<CharT> input);
        inline BasicCommandContext<CharT, S>& WithSource(S source);
        inline BasicCommandContext<CharT, S>& WithArgument(std::basic_string_view<CharT> name, std::shared_ptr<BasicIParsedArgument<CharT, S>> argument);
        inline BasicCommandContext<CharT, S>& WithCommand(BasicCommand<CharT, S> command);
        inline BasicCommandContext<CharT, S>& WithNode(BasicCommandNode<CharT, S>* node, BasicStringRange<CharT> range);
        inline BasicCommandContext<CharT, S>& WithChildContext(BasicCommandContext<CharT, S> childContext);

        inline void Reset()
        {
            detail::BasicCommandContextInternal<CharT, S>& ctx = *context;
            input = {};
            ctx.arguments.clear();
            ctx.command = nullptr;
            ctx.nodes.clear();
            ctx.parent = nullptr;
            ctx.child = {};
            ctx.modifier = nullptr;
            ctx.forks = false;
        }
        inline void Reset(S src, BasicCommandNode<CharT, S>* root)
        {
            Reset();
            source = std::move(src);
            detail::BasicCommandContextInternal<CharT, S>& ctx = *context;
            ctx.rootNode = root;
        }
    public:
        inline void Reset(S source, BasicCommandNode<CharT, S>* root, int start)
        {
            Reset(source, root);
            detail::BasicCommandContextInternal<CharT, S>& ctx = *context;
            ctx.range = BasicStringRange<CharT>::At(start);
        }
        inline void Reset(S source, BasicCommandNode<CharT, S>* root, BasicStringRange<CharT> range)
        {
            Reset(source, root);
            detail::BasicCommandContextInternal<CharT, S>& ctx = *context;
            ctx.range = std::move(range);
        }
    protected:
        BasicSuggestionContext<CharT, S> FindSuggestionContext(int cursor);

        void Merge(BasicCommandContext<CharT, S> other);
    private:
        friend class BasicCommandDispatcher<CharT, S>;
        friend class BasicLiteralCommandNode<CharT, S>;
        friend class BasicCommandNode<CharT, S>;
        template<typename, typename, typename>
        friend class BasicArgumentCommandNode;

        S source;
        std::basic_string_view<CharT> input = {};
        std::shared_ptr<detail::BasicCommandContextInternal<CharT, S>> context = nullptr;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandContext);

    namespace detail
    {
        template<typename CharT, typename S>
        class BasicCommandContextInternal
        {
        public:
            BasicCommandContextInternal(BasicCommandNode<CharT, S>* root, int start) : rootNode(root), range(BasicStringRange<CharT>::At(start)) {}
            BasicCommandContextInternal(BasicCommandNode<CharT, S>* root, BasicStringRange<CharT> range) : rootNode(root), range(std::move(range)) {}
        protected:
            friend class BasicCommandContext<CharT, S>;

            std::map<std::basic_string<CharT>, std::shared_ptr<BasicIParsedArgument<CharT, S>>, std::less<>> arguments;
            BasicCommand<CharT, S> command = nullptr;
            BasicCommandNode<CharT, S>* rootNode = nullptr;
            std::vector<BasicParsedCommandNode<CharT, S>> nodes;
            BasicStringRange<CharT> range;
            BasicCommandContext<CharT, S>* parent = nullptr;
            std::optional<BasicCommandContext<CharT, S>> child = {};
            BasicRedirectModifier<CharT, S> modifier = nullptr;
            bool forks = false;
        };
        BRIGADIER_SPECIALIZE_BASIC_TEMPL(CommandContextInternal);
    }

    template<typename CharT, typename S>
    template<typename ArgType>
    typename ArgType::type BasicCommandContext<CharT, S>::GetArgument(std::basic_string_view<CharT> name)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            throw std::runtime_error("No such argument '" + std::basic_string<CharT>(name) + "' exists on this command");
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<ArgType>())) {
            throw std::runtime_error("Argument '" + std::basic_string<CharT>(name) + "' has been acquired using wrong type");
        }

        return ((BasicParsedArgument<S, ArgType>*)parsed.get())->GetResult();
    }

    template<typename CharT, typename S>
    template<typename ArgType>
    typename ArgType::type BasicCommandContext<CharT, S>::GetArgumentOr(std::basic_string_view<CharT> name, typename ArgType::type default_value)
    {
        auto argument = context->arguments.find(name);

        if (argument == context->arguments.end()) {
            return std::move(default_value);
        }
        auto& parsed = argument->second;
        if (parsed->GetTypeInfo() != TypeInfo(TypeInfo::Create<ArgType>())) {
            throw std::runtime_error("Argument '" + std::basic_string<CharT>(name) + "' has been acquired using wrong type");
        }

        return ((BasicParsedArgument<S, ArgType>*)parsed.get())->GetResult();
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S> BasicCommandContext<CharT, S>::GetFor(S source) const
    {
        BasicCommandContext<CharT, S> result = *this;
        result.source = std::move(source);
        result.input = input;
        return result;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>* BasicCommandContext<CharT, S>::GetChild() const
    {
        if (context->child.has_value())
            return &context->child.value();
        else return nullptr;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>* BasicCommandContext<CharT, S>::GetParent() const
    {
        return context->parent;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>* BasicCommandContext<CharT, S>::GetLastParent() const
    {
        BasicCommandContext<CharT, S>* result = this;
        while (result->GetParent() != nullptr) {
            result = result->GetParent();
        }
        return result;
    }

    template<typename CharT, typename S>
    inline detail::BasicCommandContextInternal<CharT, S>* BasicCommandContext<CharT, S>::GetInternalContext() const
    {
        return context.get();
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>* BasicCommandContext<CharT, S>::GetLastChild() const
    {
        BasicCommandContext<CharT, S>* result = this;
        while (result->GetChild() != nullptr) {
            result = result->GetChild();
        }
        return result;
    }

    template<typename CharT, typename S>
    inline BasicCommand<CharT, S> BasicCommandContext<CharT, S>::GetCommand() const
    {
        return context->command;
    }


    template<typename CharT, typename S>
    inline S& BasicCommandContext<CharT, S>::GetSource()
    {
        return source;
    }

    template<typename CharT, typename S>
    inline S const& BasicCommandContext<CharT, S>::GetSource() const
    {
        return source;
    }

    template<typename CharT, typename S>
    inline BasicRedirectModifier<CharT, S> BasicCommandContext<CharT, S>::GetRedirectModifier() const
    {
        return context->modifier;
    }

    template<typename CharT, typename S>
    inline BasicStringRange<CharT> BasicCommandContext<CharT, S>::GetRange() const
    {
        return context->range;
    }

    template<typename CharT, typename S>
    inline std::basic_string_view<CharT> BasicCommandContext<CharT, S>::GetInput() const
    {
        return context->input;
    }

    template<typename CharT, typename S>
    inline BasicCommandNode<CharT, S>* BasicCommandContext<CharT, S>::GetRootNode() const
    {
        return context->rootNode;
    }

    template<typename CharT, typename S>
    inline std::vector<BasicParsedCommandNode<CharT, S>>& BasicCommandContext<CharT, S>::GetNodes() const
    {
        return context->nodes;
    }

    template<typename CharT, typename S>
    inline bool BasicCommandContext<CharT, S>::HasNodes() const
    {
        return context->nodes.size() > 0;
    }

    template<typename CharT, typename S>
    inline bool BasicCommandContext<CharT, S>::IsForked() const
    {
        return context->forks;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>::~BasicCommandContext()
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
    inline BasicCommandContext<CharT, S>& BasicCommandContext<CharT, S>::WithInput(std::basic_string_view<CharT> input)
    {
        this->input = std::move(input);
        return *this;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>& BasicCommandContext<CharT, S>::WithSource(S source)
    {
        context->source = std::move(source);
        return *this;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>& BasicCommandContext<CharT, S>::WithArgument(std::basic_string_view<CharT> name, std::shared_ptr<BasicIParsedArgument<CharT, S>> argument)
    {
        context->arguments.emplace(name, std::move(argument));
        return *this;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>& BasicCommandContext<CharT, S>::WithCommand(BasicCommand<CharT, S> command)
    {
        context->command = command;
        return *this;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>& BasicCommandContext<CharT, S>::WithNode(BasicCommandNode<CharT, S>* node, BasicStringRange<CharT> range)
    {
        if (node)
        {
            context->nodes.emplace_back(node, range);
            context->range = BasicStringRange<CharT>::Encompassing(context->range, range);
            context->modifier = node->GetRedirectModifier();
            context->forks = node->IsFork();
        }
        return *this;
    }

    template<typename CharT, typename S>
    inline BasicCommandContext<CharT, S>& BasicCommandContext<CharT, S>::WithChildContext(BasicCommandContext<CharT, S> childContext)
    {
        context->child = childContext;
        context->child->context->parent = this;
        return *this;
    }

    template<typename CharT, typename S>
    BasicSuggestionContext<CharT, S> BasicCommandContext<CharT, S>::FindSuggestionContext(int cursor)
    {
        auto& ctx = context;
        if (ctx->range.GetStart() <= cursor) {
            if (ctx->range.GetEnd() < cursor) {
                if (ctx->child.has_value()) {
                    return ctx->child->FindSuggestionContext(cursor);
                }
                else if (HasNodes()) {
                    auto& last = ctx->nodes.back();
                    return BasicSuggestionContext<CharT, S>(last.GetNode(), last.GetRange().GetEnd() + 1);
                }
                else {
                    return BasicSuggestionContext<CharT, S>(ctx->rootNode, ctx->range.GetStart());
                }
            }
            else {
                auto prev = ctx->rootNode;
                for (auto& node : ctx->nodes) {
                    auto nodeRange = node.GetRange();
                    if (nodeRange.GetStart() <= cursor && cursor <= nodeRange.GetEnd()) {
                        return BasicSuggestionContext<CharT, S>(prev, nodeRange.GetStart());
                    }
                    prev = node.GetNode();
                }
                if (prev == nullptr) {
                    throw std::runtime_error("Can't find node before cursor");
                }
                return BasicSuggestionContext<CharT, S>(prev, ctx->range.GetStart());
            }
        }
        throw std::runtime_error("Can't find node before cursor");
    }

    template<typename CharT, typename S>
    void BasicCommandContext<CharT, S>::Merge(BasicCommandContext<CharT, S> other)
    {
        detail::BasicCommandContextInternal<CharT, S>* ctx = other.GetInternalContext();
        for (auto& arg : ctx->arguments)
            context->arguments.emplace(std::move(arg));
        context->command = std::move(ctx->command);
        source = std::move(other.source);
        context->nodes.reserve(context->nodes.size() + ctx->nodes.size());
        for (auto& node : ctx->nodes)
        {
            context->range = BasicStringRange<CharT>::Encompassing(context->range, ctx->range);
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