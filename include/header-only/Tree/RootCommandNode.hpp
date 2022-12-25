#pragma once

#include "LiteralCommandNode.hpp"
#include "ArgumentCommandNode.hpp"
#include "../ParseResults.hpp"
#include <set>

//builders
namespace brigadier
{
    template<typename CharT, typename S>
    template<typename Type, typename... Args>
    auto& CommandNode<CharT, S>::Then(std::basic_string_view<CharT> name, Args&&... args)
    {
        if (redirect != nullptr) {
            throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Cannot add children to a redirected node");
        }

        if constexpr (std::is_same_v<Type, void>) {
            auto arg = children.find(name);
            if (arg == children.end()) {
                auto new_node = std::make_shared<LiteralCommandNode<CharT, S>>(name, args...);
                AddChild(new_node);
                return *new_node;
            }
            else {
                auto& arg_ptr = arg->second;
                if (arg_ptr) {
                    if (arg_ptr->GetNodeType() != CommandNodeType::LiteralCommandNode) {
                        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");
                    }
                }
                return *std::static_pointer_cast<LiteralCommandNode<CharT, S>>(arg_ptr);
            }
        }
        else
        {
            auto arg = children.find(name);
            if (arg == children.end()) {
                auto new_node = std::make_shared<ArgumentCommandNode<CharT, S, Type>>(name, args...);
                AddChild(new_node);
                return *new_node;
            }
            else {
                auto& arg_ptr = arg->second;
                if (arg_ptr) {
                    if (arg_ptr->GetNodeType() != CommandNodeType::ArgumentCommandNode) {
                        throw RuntimeError<CharT>() << BRIGADIER_LITERAL(CharT, "Node type (literal/argument) mismatch!");
                    }
                }
                return *std::static_pointer_cast<ArgumentCommandNode<CharT, S, Type>>(arg_ptr);
            }
        }
    }
}

namespace brigadier
{
    template<typename CharT, typename S>
    class RootCommandNode : public CommandNode<CharT, S>
    {
    public:
        RootCommandNode() : CommandNode<CharT, S>(nullptr, [](S&) { return true; }, nullptr, [](CommandContext<CharT, S>& s)->std::vector<S> { return { s.GetSource() }; }, false) {}

        virtual ~RootCommandNode() = default;
        virtual std::basic_string<CharT> const& GetName() { static const std::basic_string<CharT> blank; return blank; }
        virtual std::basic_string<CharT> GetUsageText() { return {}; }
        virtual std::vector<std::basic_string_view<CharT>> GetExamples() { return {}; }
        virtual void Parse(StringReader<CharT>& reader, CommandContext<CharT, S>& contextBuilder) {}
        virtual std::future<Suggestions<CharT>> ListSuggestions(CommandContext<CharT, S>& context, SuggestionsBuilder<CharT>& builder)
        {
            return Suggestions<CharT>::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::RootCommandNode; }
    protected:
        virtual bool IsValidInput(std::basic_string_view<CharT> input) { return false; }
        virtual std::basic_string_view<CharT> GetSortedKey() { return {}; }
    public:
        /**
        The string required to separate individual arguments in an input string
        */
        static constexpr std::basic_string_view<CharT> ARGUMENT_SEPARATOR = BRIGADIER_LITERAL(CharT, " ");

        /**
        The char required to separate individual arguments in an input string
        */
        static constexpr CharT ARGUMENT_SEPARATOR_CHAR = CharT(' ');
    private:
        static constexpr std::basic_string_view<CharT> USAGE_OPTIONAL_OPEN = BRIGADIER_LITERAL(CharT, "[");
        static constexpr std::basic_string_view<CharT> USAGE_OPTIONAL_CLOSE = BRIGADIER_LITERAL(CharT, "]");
        static constexpr std::basic_string_view<CharT> USAGE_REQUIRED_OPEN = BRIGADIER_LITERAL(CharT, "(");
        static constexpr std::basic_string_view<CharT> USAGE_REQUIRED_CLOSE = BRIGADIER_LITERAL(CharT, ")");
        static constexpr std::basic_string_view<CharT> USAGE_OR = BRIGADIER_LITERAL(CharT, "|");
    public:
        /**
        Sets a callback to be informed of the result of every command.

        \param consumer the new result consumer to be called
        */
        void SetConsumer(ResultConsumer<CharT, S> consumer)
        {
            this->consumer = std::move(consumer);
        }

        /**
        Parses and executes a given command.

        This is a shortcut to first Parse(StringReader, Object) and then Execute(ParseResults).

        It is recommended to parse and execute as separate steps, as parsing is often the most expensive step, and easiest to cache.

        If this command returns a value, then it successfully executed something. If it could not parse the command, or the execution was a failure,
        then an exception will be thrown. Most exceptions will be of type CommandSyntaxException, but it is possible that a RuntimeError
        may bubble up from the result of a command. The meaning behind the returned result is arbitrary, and will depend
        entirely on what command was performed.

        If the command passes through a node that is CommandNode::IsFork() then it will be 'forked'.
        A forked command will not bubble up any CommandSyntaxException's, and the 'result' returned will turn into
        'amount of successful commands executes'.

        After each and any command is ran, a registered callback given to SetConsumer(ResultConsumer)
        will be notified of the result and success of the command. You can use that method to gather more meaningful
        results than this method will return, especially when a command forks.

        \param input a command string to parse and execute
        \param source a custom "source" object, usually representing the originator of this command
        \return a numeric result from a "command" that was performed
        \throws CommandSyntaxException if the command failed to parse or execute
        \throws RuntimeError if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(StringReader, Object)
        */
        int Execute(std::basic_string_view<CharT> input, S source)
        {
            return Execute(StringReader<CharT>(input), std::move(source));
        }

        /**
        Parses and executes a given command.

        This is a shortcut to first Parse(StringReader, Object) and then Execute(ParseResults).

        It is recommended to parse and execute as separate steps, as parsing is often the most expensive step, and easiest to cache.

        If this command returns a value, then it successfully executed something. If it could not parse the command, or the execution was a failure,
        then an exception will be thrown. Most exceptions will be of type CommandSyntaxException, but it is possible that a RuntimeError
        may bubble up from the result of a command. The meaning behind the returned result is arbitrary, and will depend
        entirely on what command was performed.

        If the command passes through a node that is CommandNode::IsFork() then it will be 'forked'.
        A forked command will not bubble up any CommandSyntaxException's, and the 'result' returned will turn into
        'amount of successful commands executes'.

        After each and any command is ran, a registered callback given to SetConsumer(ResultConsumer)
        will be notified of the result and success of the command. You can use that method to gather more meaningful
        results than this method will return, especially when a command forks.

        \param input a command string to parse & execute
        \param source a custom "source" object, usually representing the originator of this command
        \return a numeric result from a "command" that was performed
        \throws CommandSyntaxException if the command failed to parse or execute
        \throws RuntimeError if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(String, Object)
        */
        int Execute(StringReader<CharT> input, S source)
        {
            return Execute(Parse(std::move(input), std::move(source)));
        }

        /**
        Executes a given pre-parsed command.

        If this command returns a value, then it successfully executed something. If the execution was a failure,
        then an exception will be thrown.
        Most exceptions will be of type CommandSyntaxException, but it is possible that a RuntimeError
        may bubble up from the result of a command. The meaning behind the returned result is arbitrary, and will depend
        entirely on what command was performed.

        If the command passes through a node that is CommandNode::IsFork() then it will be 'forked'.
        A forked command will not bubble up any CommandSyntaxException<CharT>'s, and the 'result' returned will turn into
        'amount of successful commands executes'.

        After each and any command is ran, a registered callback given to SetConsumer(ResultConsumer)
        will be notified of the result and success of the command. You can use that method to gather more meaningful
        results than this method will return, especially when a command forks.

        \param parse the result of a successful Parse(StringReader, Object)
        \return a numeric result from a "command" that was performed.
        \throws CommandSyntaxException if the command failed to parse or execute
        \throws RuntimeError if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(String, Object)
        \see Execute(StringReader, Object)
        */
        int Execute(ParseResults<CharT, S> const& parse)
        {
            if (parse.GetReader().CanRead()) {
                if (parse.GetExceptions().size() == 1) {
                    throw CommandSyntaxException<CharT>(parse.GetExceptions().begin()->second);
                }
                else if (parse.GetContext().GetRange().IsEmpty()) {
                    throw exceptions::DispatcherUnknownCommand(parse.GetReader());
                }
                else {
                    throw exceptions::DispatcherUnknownArgument(parse.GetReader());
                }
            }

            int result = 0;
            int successfulForks = 0;
            bool forked = false;
            bool foundCommand = false;
            auto command = parse.GetReader().GetString();
            auto original = CommandContext<CharT, S>(parse.GetContext());
            original.WithInput(command);
            std::vector<CommandContext<CharT, S>> contexts = { CommandContext<CharT, S>(original) };
            std::vector<CommandContext<CharT, S>> next;

            while (!contexts.empty()) {
                for (auto& context : contexts) {
                    CommandContext<CharT, S>* child = context.GetChild();
                    if (child != nullptr) {
                        forked |= context.IsForked();
                        if (child->HasNodes()) {
                            foundCommand = true;
                            RedirectModifier<CharT, S> modifier = context.GetRedirectModifier();
                            if (modifier == nullptr) {
                                next.push_back(child->GetFor(context.GetSource()));
                            }
                            else {
                                try {
                                    auto results = modifier(context);
                                    if (!results.empty()) {
                                        for (auto& source : results) {
                                            next.push_back(child->GetFor(std::move(source)));
                                        }
                                    }
                                }
                                catch (CommandSyntaxException<CharT> const&) {
                                    consumer(context, false, 0);
                                    if (!forked) {
                                        throw;
                                    }
                                }
                            }
                        }
                    }
                    else if (context.GetCommand() != nullptr) {
                        foundCommand = true;
                        try {
                            int value = context.GetCommand()(context);
                            result += value;
                            consumer(context, true, value);
                            successfulForks++;
                        }
                        catch (CommandSyntaxException<CharT> const&) {
                            consumer(context, false, 0);
                            if (!forked) {
                                throw;
                            }
                        }
                    }
                }

                contexts = std::move(next);
            }

            if (!foundCommand) {
                consumer(original, false, 0);
                throw exceptions::DispatcherUnknownCommand(parse.GetReader());
            }

            return forked ? successfulForks : result;
        }

        /**
        Parses a given command.

        The result of this method can be cached, and it is advised to do so where appropriate. Parsing is often the
        most expensive step, and this allows you to essentially "precompile" a command if it will be ran often.

        If the command passes through a node that is CommandNode::IsFork() then the resulting context will be marked as 'forked'.
        Forked contexts may contain child contexts, which may be modified by the RedirectModifier attached to the fork.

        Parsing a command can never fail, you will always be provided with a new ParseResults.
        However, that does not mean that it will always parse into a valid command. You should inspect the returned results
        to check for validity. If its ParseResults::GetReader() StringReader<CharT>::CanRead() then it did not finish
        parsing successfully. You can use that position as an indicator to the user where the command stopped being valid.
        You may inspect ParseResults::GetExceptions() if you know the parse failed, as it will explain why it could
        not find any valid commands. It may contain multiple exceptions, one for each "potential node" that it could have visited,
        explaining why it did not go down that node.

        When you eventually call Execute(ParseResults) with the result of this method, the above error checking
        will occur. You only need to inspect it yourself if you wish to handle that yourself.

        \param command a command string to parse
        \param source a custom "source" object, usually representing the originator of this command
        \return the result of parsing this command
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(String, Object)
        */
        ParseResults<CharT, S> Parse(std::basic_string_view<CharT> command, S source)
        {
            StringReader<CharT> reader = StringReader<CharT>(command);
            return Parse(reader, std::move(source));
        }

        /**
        Parses a given command.

        The result of this method can be cached, and it is advised to do so where appropriate. Parsing is often the
        most expensive step, and this allows you to essentially "precompile" a command if it will be ran often.

        If the command passes through a node that is CommandNode::IsFork() then the resulting context will be marked as 'forked'.
        Forked contexts may contain child contexts, which may be modified by the RedirectModifier attached to the fork.

        Parsing a command can never fail, you will always be provided with a new ParseResults.
        However, that does not mean that it will always parse into a valid command. You should inspect the returned results
        to check for validity. If its ParseResults::GetReader() StringReader::CanRead() then it did not finish
        parsing successfully. You can use that position as an indicator to the user where the command stopped being valid.
        You may inspect ParseResults::GetExceptions() if you know the parse failed, as it will explain why it could
        not find any valid commands. It may contain multiple exceptions, one for each "potential node" that it could have visited,
        explaining why it did not go down that node.

        When you eventually call Execute(ParseResults) with the result of this method, the above error checking
        will occur. You only need to inspect it yourself if you wish to handle that yourself.

        \param command a command string to parse
        \param source a custom "source" object, usually representing the originator of this command
        \return the result of parsing this command
        \see Parse(String, Object)
        \see Execute(ParseResults)
        \see Execute(String, Object)
        */
        ParseResults<CharT, S> Parse(StringReader<CharT> command, S source)
        {
            ParseResults<CharT, S> result(CommandContext<CharT, S>(std::move(source), this, command.GetCursor()), std::move(command));
            ParseNodes(this, result);
            return result;
        }

    private:
        void ParseNodes(CommandNode<CharT, S>* node, ParseResults<CharT, S>& result)
        {
            if (!node)
                return;

            S& source = result.context.GetSource();

            std::optional<ParseResults<CharT, S>> ctxs[2] = {};

            bool best = 0;

            size_t cursor = result.reader.GetCursor();

            auto [relevant_nodes, relevant_node_count] = node->GetRelevantNodes(result.reader);

            for (size_t i = 0; i < relevant_node_count; ++i) {
                auto& child = relevant_nodes[i];
                auto& current_result_ctx = ctxs[!best];
                auto& best_potential = ctxs[best];

                if (!child->CanUse(source)) {
                    continue;
                }

                // initialize current context
                if (current_result_ctx.has_value()) {
                    // context already exists so we have to reset it (avoid memory reallocation)
                    current_result_ctx->Reset(source, result.context.GetRootNode(), result.GetContext().GetRange(), result.GetReader());
                }
                else {
                    // create context
                    current_result_ctx.emplace(CommandContext<CharT, S>(source, result.GetContext().GetRootNode(), result.GetContext().GetRange()), result.GetReader());
                }

                auto& current_result = current_result_ctx.value();

                StringReader<CharT>& reader = current_result.reader;
                CommandContext<CharT, S>& context = current_result.context;

                try {
                    try {
                        child->Parse(reader, context);
                    }
                    catch (RuntimeError<CharT> const& ex) {
                        throw exceptions::DispatcherParseException(reader, ex.What());
                    }
                    if (reader.CanRead() && reader.Peek() != ARGUMENT_SEPARATOR_CHAR) {
                        throw exceptions::DispatcherExpectedArgumentSeparator(reader);
                    }
                }
                catch (CommandSyntaxException<CharT> const& ex) {
                    result.exceptions.emplace(child.get(), std::move(ex));
                    reader.SetCursor(cursor);
                    continue;
                }

                context.WithCommand(child->GetCommand());

                if (reader.CanRead(child->GetRedirect() == nullptr ? 2 : 1)) {
                    reader.Skip();
                    if (child->GetRedirect() != nullptr) {
                        ParseResults<CharT, S> child_result(CommandContext<CharT, S>(source, child->GetRedirect(), reader.GetCursor()), reader);
                        ParseNodes(child->GetRedirect(), child_result);
                        result.context.Merge(std::move(context));
                        result.context.WithChildContext(std::move(child_result.context));
                        result.exceptions = std::move(child_result.exceptions);
                        result.reader = std::move(child_result.reader);
                        return;
                    }
                    else {
                        ParseNodes(child.get(), current_result);
                    }
                }

                if (best_potential.has_value()) {
                    if (current_result.IsBetterThan(*best_potential)) {
                        best = !best;
                    }
                }
                else {
                    best = !best;
                }
            }

            auto& best_potential = ctxs[best];
            if (best_potential.has_value()) {
                result.exceptions.clear();
                result.reader = std::move(best_potential->reader);
                result.context.Merge(std::move(best_potential->context));
            }
        }

    public:
        /**
        Gets all possible executable commands following the given node.

        You may use GetRoot() as a target to get all usage data for the entire command tree.

        The returned syntax will be in "simple" form: `<param>` and `literal`. "Optional" nodes will be
        listed as multiple entries: the parent node, and the child nodes.
        For example, a required literal "foo" followed by an optional param "int" will be two nodes:
          foo
          foo <int>

        The path to the specified node will NOT be prepended to the output, as there can theoretically be many
        ways to reach a given node. It will only give you paths relative to the specified node, not absolute from root.

        \param node target node to get child usage strings for
        \param source a custom "source" object, usually representing the originator of this command
        \param restricted if true, commands that the source cannot access will not be mentioned
        \return array of full usage strings under the target node
        */
        std::vector<std::basic_string<CharT>> GetAllUsage(CommandNode<CharT, S>* node, S source, bool restricted)
        {
            std::vector<std::basic_string<CharT>> result;
            GetAllUsage(node, std::move(source), result, {}, restricted);
            return result;
        }

    private:
        void GetAllUsage(CommandNode<CharT, S>* node, S source, std::vector<std::basic_string<CharT>>& result, std::basic_string<CharT> prefix, bool restricted)
        {
            if (!node)
                return;

            if (restricted && !node->CanUse(source))
                return;

            if (node->GetCommand())
                result.push_back(prefix);

            if (node->GetRedirect()) {
                if (prefix.empty()) {
                    prefix = node->GetUsageText();
                }
                prefix = node->GetUsageText();
                prefix += ARGUMENT_SEPARATOR;
                if (node->GetRedirect() == this) {
                    prefix += BRIGADIER_LITERAL(CharT, "...");
                }
                else {
                    prefix += BRIGADIER_LITERAL(CharT, "-> ");
                    prefix += node->GetRedirect()->GetUsageText();
                }
                result.emplace_back(std::move(prefix));
            }
            else if (!node->GetChildren().empty()) {
                for (auto const& [name, child] : node->GetChildren()) {
                    std::basic_string<CharT> next_prefix = prefix;
                    if (!next_prefix.empty()) {
                        next_prefix += ARGUMENT_SEPARATOR;
                    }
                    next_prefix += child->GetUsageText();
                    GetAllUsage(child.get(), std::move(source), result, std::move(next_prefix), restricted);
                }
            }
        }

    public:
        /**
        Gets the possible executable commands from a specified node.

        You may use GetRoot() as a target to get usage data for the entire command tree.

        The returned syntax will be in "smart" form: `<param>`, `literal`, `[optional]` and `(either|or)`.
        These forms may be mixed and matched to provide as much information about the child nodes as it can, without being too verbose.
        For example, a required literal "foo" followed by an optional param "int" can be compressed into one string:
            `foo [<int>]`

        The path to the specified node will NOT be prepended to the output, as there can theoretically be many
        ways to reach a given node. It will only give you paths relative to the specified node, not absolute from root.

        The returned usage will be restricted to only commands that the provided source can use.

        \param node target node to get child usage strings for
        \param source a custom "source" object, usually representing the originator of this command
        \return array of full usage strings under the target node
        */
        std::map<CommandNode<CharT, S>*, std::basic_string<CharT>> GetSmartUsage(CommandNode<CharT, S>* node, S source)
        {
            std::map<CommandNode<CharT, S>*, std::basic_string<CharT>> result;

            for (auto const& [name, child] : node->GetChildren()) {
                std::basic_string<CharT> usage = GetSmartUsage(child.get(), std::move(source), node->GetCommand() != nullptr, false);
                if (!usage.empty()) {
                    result[child.get()] = std::move(usage);
                }
            }
            return result;
        }

    private:
        std::basic_string<CharT> GetSmartUsage(CommandNode<CharT, S>* node, S source, bool optional, bool deep)
        {
            if (!node)
                return {};

            if (!node->CanUse(source))
                return {};

            std::basic_string<CharT> self;
            if (optional) {
                self = USAGE_OPTIONAL_OPEN;
                self += node->GetUsageText();
                self += USAGE_OPTIONAL_CLOSE;
            }
            else {
                self = node->GetUsageText();
            }
            const bool childOptional = node->GetCommand() != nullptr;
            std::basic_string_view<CharT> open = childOptional ? USAGE_OPTIONAL_OPEN : USAGE_REQUIRED_OPEN;
            std::basic_string_view<CharT> close = childOptional ? USAGE_OPTIONAL_CLOSE : USAGE_REQUIRED_CLOSE;

            if (!deep) {
                if (node->GetRedirect()) {
                    self += ARGUMENT_SEPARATOR;
                    if (node->GetRedirect() == this) {
                        self += BRIGADIER_LITERAL(CharT, "...");
                    }
                    else {
                        self += BRIGADIER_LITERAL(CharT, "-> ");
                        self += node->GetRedirect()->GetUsageText();
                    }
                    return self;
                }
                else {
                    std::vector<CommandNode<CharT, S>*> children;
                    for (auto const& [name, child] : node->GetChildren()) {
                        if (child->CanUse(source)) {
                            children.push_back(child.get());
                        }
                    }
                    if (children.size() == 1) {
                        std::basic_string<CharT> usage = GetSmartUsage(children[0], source, childOptional, childOptional);
                        if (!usage.empty()) {
                            self += ARGUMENT_SEPARATOR;
                            self += std::move(usage);
                            return self;
                        }
                    }
                    else if (children.size() > 1) {
                        std::set<std::basic_string<CharT>> childUsage;
                        for (auto child : children) {
                            std::basic_string<CharT> usage = GetSmartUsage(child, source, childOptional, true);
                            if (!usage.empty()) {
                                childUsage.insert(usage);
                            }
                        }
                        if (childUsage.size() == 1) {
                            std::basic_string<CharT> usage = *childUsage.begin();
                            self += ARGUMENT_SEPARATOR;
                            if (childOptional) {
                                self += USAGE_OPTIONAL_OPEN;
                                self += usage;
                                self += USAGE_OPTIONAL_CLOSE;
                            }
                            else self += usage;
                            return self;
                        }
                        else if (childUsage.size() > 1) {
                            std::basic_string<CharT> builder(open);
                            int count = 0;
                            for (auto child : children) {
                                if (count > 0) {
                                    builder += USAGE_OR;
                                }
                                builder += child->GetUsageText();
                                count++;
                            }
                            if (count > 0) {
                                self += ARGUMENT_SEPARATOR;
                                self += std::move(builder);
                                self += close;
                                return self;
                            }
                        }
                    }
                }
            }

            return self;
        }
    public:
        /**
        Gets suggestions for a parsed input string on what comes next.

        As it is ultimately up to custom argument types to provide suggestions, it may be an asynchronous operation,
        for example getting in-game data or player names etc. As such, this method returns a future and no guarantees
        are made to when or how the future completes.

        The suggestions provided will be in the context of the end of the parsed input string, but may suggest
        new or replacement strings for earlier in the input string. For example, if the end of the string was
        `foobar` but an argument preferred it to be `minecraft:foobar`, it will suggest a replacement for that
        whole segment of the input.

        \param parse the result of a Parse(StringReader, Object)
        \param cancel a pointer to a bool that can cancel future when set to true. Result will be empty in such a case.
        \return a future that will eventually resolve into a Suggestions object
        */
        std::future<Suggestions<CharT>> GetCompletionSuggestions(ParseResults<CharT, S>& parse, bool* cancel = nullptr)
        {
            return GetCompletionSuggestions(parse, parse.GetReader().GetTotalLength(), cancel);
        }

        /**
        Gets suggestions for a parsed input string on what comes next.

        As it is ultimately up to custom argument types to provide suggestions, it may be an asynchronous operation,
        for example getting in-game data or player names etc. As such, this method returns a future and no guarantees
        are made to when or how the future completes.

        The suggestions provided will be in the context of the end of the parsed input string, but may suggest
        new or replacement strings for earlier in the input string. For example, if the end of the string was
        `foobar` but an argument preferred it to be `minecraft:foobar`, it will suggest a replacement for that
        whole segment of the input.

        \param parse the result of a Parse(StringReader, Object)
        \param cursor the place where the suggestions should be considered
        \param cancel a pointer to a bool that can cancel future when set to true. Result will be empty in such a case.
        \return a future that will eventually resolve into a Suggestions object
        */
        std::future<Suggestions<CharT>> GetCompletionSuggestions(ParseResults<CharT, S>& parse, size_t cursor, bool* cancel = nullptr)
        {
            return std::async(std::launch::async, [](ParseResults<CharT, S>* parse, size_t cursor, bool* cancel)
            {
                auto context = CommandContext<CharT, S>(parse->GetContext());

                SuggestionContext<CharT, S> nodeBeforeCursor = context.FindSuggestionContext(cursor);
                CommandNode<CharT, S>* parent = nodeBeforeCursor.parent;
                size_t start = (std::min)(nodeBeforeCursor.startPos, cursor);

                std::basic_string_view<CharT> fullInput = parse->GetReader().GetString();
                std::basic_string_view<CharT> truncatedInput = fullInput.substr(0, cursor);
                std::basic_string<CharT> truncatedInputLowerCase(truncatedInput);
                std::transform(truncatedInputLowerCase.begin(), truncatedInputLowerCase.end(), truncatedInputLowerCase.begin(), [](CharT c) { return std::tolower(c); });

                context.WithInput(truncatedInput);

                std::vector<std::future<Suggestions<CharT>>> futures;
                std::vector<SuggestionsBuilder<CharT>> builders;
                size_t max_size = parent->GetChildren().size();
                futures.reserve(max_size);
                builders.reserve(max_size);
                for (auto const& [name, node] : parent->GetChildren()) {
                    try {
                        builders.emplace_back(truncatedInput, truncatedInputLowerCase, start, cancel);
                        futures.push_back(node->ListSuggestions(context, builders.back()));
                    }
                    catch (CommandSyntaxException<CharT> const&) {}
                }

                std::vector<Suggestions<CharT>> suggestions;
                for (auto& future : futures)
                {
                    suggestions.emplace_back(future.get());
                }
                return Suggestions<CharT>::Merge(fullInput, suggestions);
            }, &parse, cursor, cancel);
        }

        /**
        Finds a valid path to a given node on the command tree.

        There may theoretically be multiple paths to a node on the tree, especially with the use of forking or redirecting.
        As such, this method makes no guarantees about which path it finds. It will not look at forks or redirects,
        and find the first instance of the target node on the tree.

        The only guarantee made is that for the same command tree and the same version of this library, the result of
        this method will ALWAYS be a valid input for FindNode(Collection), which should return the same node
        as provided to this method.

        \param target the target node you are finding a path for
        \return a path to the resulting node, or an empty list if it was not found
        */
        std::vector<std::basic_string<CharT>> GetPath(CommandNode<CharT, S>* target)
        {
            std::vector<std::vector<CommandNode<CharT, S>*>> nodes;
            AddPaths(root.get(), nodes, {});

            for (std::vector<CommandNode<CharT, S>*>& list : nodes) {
                if (list.back() == target) {
                    std::vector<std::basic_string<CharT>> result;
                    result.reserve(list.size());
                    for (auto node : list) {
                        if (node != root.get()) {
                            result.push_back(node->GetName());
                        }
                    }
                    return result;
                }
            }

            return {};
        }

        /**
        Finds a node by its path

        Paths may be generated with GetPath(CommandNode), and are guaranteed (for the same tree, and the
        same version of this library) to always produce the same valid node by this method.

        If a node could not be found at the specified path, then nullptr will be returned.

        \param path a generated path to a node
        \return the node at the given path, or null if not found
        */
        CommandNode<CharT, S>* FindNode(std::vector<std::basic_string<CharT>> const& path) {
            CommandNode<CharT, S>* node = this;
            for (auto& name : path) {
                node = node->GetChild(name).get();
                if (node == nullptr) {
                    return nullptr;
                }
            }
            return node;
        }

    private:
        void AddPaths(CommandNode<CharT, S>* node, std::vector<std::vector<CommandNode<CharT, S>*>>& result, std::vector<CommandNode<CharT, S>*> parents) {
            parents.push_back(node);
            result.push_back(parents);

            for (auto const& [name, child] : node->GetChildren()) {
                AddPaths(child.get(), result, parents);
            }
        }

    private:
        ResultConsumer<CharT, S> consumer = [](CommandContext<CharT, S>& context, bool success, int result) {};
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(RootCommandNode);
}
