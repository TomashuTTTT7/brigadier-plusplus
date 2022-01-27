#pragma once

#include "Tree/RootCommandNode.hpp"
#include "Builder/LiteralArgumentBuilder.hpp"
#include "Builder/RequiredArgumentBuilder.hpp"
#include "ParseResults.hpp"
#include <set>

namespace brigadier
{
    /**
    The core command dispatcher, for registering, parsing, and executing commands.
    
    \param <S> a custom "source" type, such as a user or originator of a command
    */
    template<typename S>
    class CommandDispatcher
    {
    public:
        /**
        The string required to separate individual arguments in an input string
        */
        static constexpr std::string_view ARGUMENT_SEPARATOR = " ";

        /**
        The char required to separate individual arguments in an input string
        */
        static constexpr char ARGUMENT_SEPARATOR_CHAR = ' ';
    private:
        static constexpr std::string_view USAGE_OPTIONAL_OPEN = "[";
        static constexpr std::string_view USAGE_OPTIONAL_CLOSE = "]";
        static constexpr std::string_view USAGE_REQUIRED_OPEN = "(";
        static constexpr std::string_view USAGE_REQUIRED_CLOSE = ")";
        static constexpr std::string_view USAGE_OR = "|";
    public:
        /**
        Create a new CommandDispatcher with the specified root node.
        
        This is often useful to copy existing or pre-defined command trees.
        
        \param root the existing RootCommandNode to use as the basis for this tree
        */
        CommandDispatcher(RootCommandNode<S>* root) : root(std::make_shared<RootCommandNode<S>>(*root)) {}

        /**
        Creates a new CommandDispatcher with an empty command tree.
        */
        CommandDispatcher() : root(std::make_shared<RootCommandNode<S>>()) {}

        /**
        Utility method for registering new commands.

        \param args these arguments are forwarded to builder to node constructor. The first param is always node name.
        \return the builder with node added to this tree
        */
        template<template<typename...> typename Next = Literal, typename Type = void, typename... Args>
        auto Register(Args&&... args)
        {
            if constexpr (std::is_same_v<Type, void>) {
                using next_node = typename Next<S>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = root->children.find(name);
                if (arg == root->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    root->AddChild(new_node);
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
            else {
                using next_node = typename Next<S, Type>::node_type;
                next_node node_builder(std::forward<Args>(args)...);
                auto& name = node_builder.GetName();
                auto arg = root->children.find(name);
                if (arg == root->children.end()) {
                    auto new_node = std::make_shared<next_node>(std::move(node_builder));
                    root->AddChild(new_node);
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

        /**
        Sets a callback to be informed of the result of every command.

        \param consumer the new result consumer to be called
        */
        void SetConsumer(ResultConsumer<S> consumer)
        {
            this->consumer = consumer;
        }

        /**
        Gets the root of this command tree.

        This is often useful as a target of a ArgumentBuilder::Redirect(CommandNode),
        GetAllUsage(CommandNode, Object, boolean) or GetSmartUsage(CommandNode, Object).
        You may also use it to clone the command tree via CommandDispatcher(RootCommandNode).

        \return root of the command tree
        */
        std::shared_ptr<RootCommandNode<S>> GetRoot() const
        {
            return root;
        }

        /**
        Parses and executes a given command.

        This is a shortcut to first Parse(StringReader, Object) and then Execute(ParseResults).

        It is recommended to parse and execute as separate steps, as parsing is often the most expensive step, and easiest to cache.

        If this command returns a value, then it successfully executed something. If it could not parse the command, or the execution was a failure,
        then an exception will be thrown. Most exceptions will be of type CommandSyntaxException, but it is possible that a std::runtime_error
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
        \throws std::runtime_error if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(StringReader, Object)
        */
        int Execute(std::string_view input, S source)
        {
            StringReader reader = StringReader(input);
            return Execute(reader, source);
        }

        /**
        Parses and executes a given command.

        This is a shortcut to first Parse(StringReader, Object) and then Execute(ParseResults).

        It is recommended to parse and execute as separate steps, as parsing is often the most expensive step, and easiest to cache.

        If this command returns a value, then it successfully executed something. If it could not parse the command, or the execution was a failure,
        then an exception will be thrown. Most exceptions will be of type CommandSyntaxException, but it is possible that a std::runtime_error
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
        \throws std::runtime_error if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(String, Object)
        */
        int Execute(StringReader& input, S source)
        {
            auto parse = Parse(input, source);
            return Execute(parse);
        }

        /**
        Executes a given pre-parsed command.

        If this command returns a value, then it successfully executed something. If the execution was a failure,
        then an exception will be thrown.
        Most exceptions will be of type CommandSyntaxException, but it is possible that a std::runtime_error
        may bubble up from the result of a command. The meaning behind the returned result is arbitrary, and will depend
        entirely on what command was performed.

        If the command passes through a node that is CommandNode::IsFork() then it will be 'forked'.
        A forked command will not bubble up any CommandSyntaxException's, and the 'result' returned will turn into
        'amount of successful commands executes'.

        After each and any command is ran, a registered callback given to SetConsumer(ResultConsumer)
        will be notified of the result and success of the command. You can use that method to gather more meaningful
        results than this method will return, especially when a command forks.

        \param parse the result of a successful Parse(StringReader, Object)
        \return a numeric result from a "command" that was performed.
        \throws CommandSyntaxException if the command failed to parse or execute
        \throws std::runtime_error if the command failed to execute and was not handled gracefully
        \see Parse(String, Object)
        \see Parse(StringReader, Object)
        \see Execute(String, Object)
        \see Execute(StringReader, Object)
        */
        int Execute(ParseResults<S>& parse)
        {
            if (parse.GetReader().CanRead()) {
                if (parse.GetExceptions().size() == 1) {
                    throw *parse.GetExceptions().begin();
                }
                else if (parse.GetContext().GetRange().IsEmpty()) {
                    throw CommandSyntaxException::BuiltInExceptions::DispatcherUnknownCommand(parse.GetReader());
                }
                else {
                    throw CommandSyntaxException::BuiltInExceptions::DispatcherUnknownArgument(parse.GetReader());
                }
            }

            int result = 0;
            int successfulForks = 0;
            bool forked = false;
            bool foundCommand = false;
            auto command = parse.GetReader().GetString();
            auto original = parse.GetContext();
            original.WithInput(command);
            std::vector<CommandContext<S>> contexts = { original };
            std::vector<CommandContext<S>> next;

            while (!contexts.empty()) {
                for (auto& context : contexts) {
                    CommandContext<S>* child = context.GetChild();
                    if (child != nullptr) {
                        forked |= context.IsForked();
                        if (child->HasNodes()) {
                            foundCommand = true;
                            RedirectModifier<S> modifier = context.GetRedirectModifier();
                            if (modifier == nullptr) {
                                next.push_back(child->GetFor(context.GetSource()));
                            } else {
                                try {
                                    auto results = modifier(context);
                                    if (!results.empty()) {
                                        for (auto& source : results) {
                                            next.push_back(child->GetFor(std::move(source)));
                                        }
                                    }
                                }
                                catch (CommandSyntaxException const& ex) {
                                    consumer(context, false, 0);
                                    if (!forked) {
                                        throw ex;
                                    }
                                }
                            }
                        }
                    } else if (context.GetCommand() != nullptr) {
                        foundCommand = true;
                        try {
                            int value = context.GetCommand()(context);
                            result += value;
                            consumer(context, true, value);
                            successfulForks++;
                        }
                        catch (CommandSyntaxException const& ex) {
                            consumer(context, false, 0);
                            if (!forked) {
                                throw ex;
                            }
                        }
                    }
                }

                contexts = std::move(next);
            }

            if (!foundCommand) {
                consumer(original, false, 0);
                throw CommandSyntaxException::BuiltInExceptions::DispatcherUnknownCommand(parse.GetReader());
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
        \see Parse(StringReader, Object)
        \see Execute(ParseResults)
        \see Execute(String, Object)
        */
        ParseResults<S> Parse(std::string_view command, S source)
        {
            StringReader reader = StringReader(command);
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
        ParseResults<S> Parse(StringReader& command, S source)
        {
            ParseResults<S> result(CommandContext<S>(std::move(source), root.get(), command.GetCursor()), command);
            ParseNodes(root.get(), result);
            return result;
        }

    private:
        void ParseNodes(CommandNode<S>* node, ParseResults<S>& result)
        {
            if (!node)
                return;

            S& source = result.context.GetSource();

            std::optional<ParseResults<S>> best_potential = {};
            std::optional<ParseResults<S>> current_result_ctx = {}; // delay initialization

            int cursor = result.reader.GetCursor();

            auto [relevant_nodes, relevant_node_count] = node->GetRelevantNodes(result.reader);

            for (size_t i = 0; i < relevant_node_count; ++i) {
                auto& child = relevant_nodes[i];

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
                    current_result_ctx = ParseResults<S>(CommandContext<S>(source, result.GetContext().GetRootNode(), result.GetContext().GetRange()), result.GetReader());
                }

                auto& current_result = current_result_ctx.value();

                StringReader& reader = current_result.reader;
                CommandContext<S>& context = current_result.context;

                try {
                    try {
                        child->Parse(reader, context);
                    }
                    catch (std::runtime_error const& ex) {
                        throw CommandSyntaxException::BuiltInExceptions::DispatcherParseException(reader, ex.what());
                    }
                    if (reader.CanRead() && reader.Peek() != ARGUMENT_SEPARATOR_CHAR) {
                        throw CommandSyntaxException::BuiltInExceptions::DispatcherExpectedArgumentSeparator(reader);
                    }
                }
                catch (CommandSyntaxException ex) {
                    result.exceptions.emplace(child.get(), std::move(ex));
                    reader.SetCursor(cursor);
                    continue;
                }

                context.WithCommand(child->GetCommand());

                if (reader.CanRead(child->GetRedirect() == nullptr ? 2 : 1)) {
                    reader.Skip();
                    if (child->GetRedirect() != nullptr) {
                        ParseResults<S> child_result(CommandContext<S>(source, child->GetRedirect().get(), reader.GetCursor()), reader);
                        ParseNodes(child->GetRedirect().get(), child_result);
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
                        best_potential = std::move(current_result_ctx);
                    }
                }
                else {
                    best_potential = std::move(current_result_ctx);
                }
            }

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
        std::vector<std::string> GetAllUsage(CommandNode<S>* node, S source, bool restricted)
        {
            std::vector<std::string> result;
            GetAllUsage(node, std::move(source), result, {}, restricted);
            return result;
        }

    private:
        void GetAllUsage(CommandNode<S>* node, S source, std::vector<std::string>& result, std::string prefix, bool restricted)
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
                if (node->GetRedirect() == root) {
                    prefix += "...";
                }
                else {
                    prefix += "-> ";
                    prefix += node->GetRedirect()->GetUsageText();
                }
                result.emplace_back(std::move(prefix));
            }
            else if (!node->GetChildren().empty()) {
                for (auto const& [name, child] : node->GetChildren()) {
                    std::string next_prefix = prefix;
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
        std::map<CommandNode<S>*, std::string> GetSmartUsage(CommandNode<S>* node, S source)
        {
            std::map<CommandNode<S>*, std::string> result;

            for (auto const& [name, child] : node->GetChildren()) {
                std::string usage = GetSmartUsage(child.get(), std::move(source), node->GetCommand() != nullptr, false);
                if (!usage.empty()) {
                    result[child.get()] = std::move(usage);
                }
            }
            return result;
        }

    private:
        std::string GetSmartUsage(CommandNode<S>* node, S source, bool optional, bool deep)
        {
            if (!node)
                return {};

            if (!node->CanUse(source))
                return {};

            std::string self;
            if (optional) {
                self = USAGE_OPTIONAL_OPEN;
                self += node->GetUsageText();
                self += USAGE_OPTIONAL_CLOSE;
            }
            else {
                self = node->GetUsageText();
            }
            const bool childOptional = node->GetCommand() != nullptr;
            std::string_view open = childOptional ? USAGE_OPTIONAL_OPEN : USAGE_REQUIRED_OPEN;
            std::string_view close = childOptional ? USAGE_OPTIONAL_CLOSE : USAGE_REQUIRED_CLOSE;

            if (!deep) {
                if (node->GetRedirect()) {
                    self += ARGUMENT_SEPARATOR;
                    if (node->GetRedirect() == root) {
                        self += "...";
                    }
                    else {
                        self += "-> ";
                        self += node->GetRedirect()->GetUsageText();
                    }
                    return self;
                }
                else {
                    std::vector<CommandNode<S>*> children;
                    for (auto const& [name, child] : node->GetChildren()) {
                        if (child->CanUse(source)) {
                            children.push_back(child.get());
                        }
                    }
                    if (children.size() == 1) {
                        std::string usage = GetSmartUsage(children[0], source, childOptional, childOptional);
                        if (!usage.empty()) {
                            self += ARGUMENT_SEPARATOR;
                            self += std::move(usage);
                            return self;
                        }
                    }
                    else if (children.size() > 1) {
                        std::set<std::string> childUsage;
                        for (auto child : children) {
                            std::string usage = GetSmartUsage(child, source, childOptional, true);
                            if (!usage.empty()) {
                                childUsage.insert(usage);
                            }
                        }
                        if (childUsage.size() == 1) {
                            std::string usage = *childUsage.begin();
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
                            std::string builder(open);
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
        std::future<Suggestions> GetCompletionSuggestions(ParseResults<S>& parse, bool* cancel = nullptr)
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
        std::future<Suggestions> GetCompletionSuggestions(ParseResults<S>& parse, int cursor, bool* cancel = nullptr)
        {
            return std::async(std::launch::async, [](ParseResults<S>* parse, int cursor, bool* cancel) {
                auto context = parse->GetContext();

                SuggestionContext<S> nodeBeforeCursor = context.FindSuggestionContext(cursor);
                CommandNode<S>* parent = nodeBeforeCursor.parent;
                int start = (std::min)(nodeBeforeCursor.startPos, cursor);

                std::string_view fullInput = parse->GetReader().GetString();
                std::string_view truncatedInput = fullInput.substr(0, cursor);
                std::string truncatedInputLowerCase(truncatedInput);
                std::transform(truncatedInputLowerCase.begin(), truncatedInputLowerCase.end(), truncatedInputLowerCase.begin(), [](char c) { return std::tolower(c); });

                context.WithInput(truncatedInput);

                std::vector<std::future<Suggestions>> futures;
                std::vector<SuggestionsBuilder> builders;
                size_t max_size = parent->GetChildren().size();
                futures.reserve(max_size);
                builders.reserve(max_size);
                for (auto const& [name, node] : parent->GetChildren()) {
                    try {
                        builders.emplace_back(truncatedInput, truncatedInputLowerCase, start, cancel);
                        futures.push_back(node->ListSuggestions(context, builders.back()));
                    }
                    catch (CommandSyntaxException const&) {}
                }

                std::vector<Suggestions> suggestions;
                for (auto& future : futures)
                {
                    suggestions.emplace_back(future.get());
                }
                return Suggestions::Merge(fullInput, suggestions);
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
        std::vector<std::string> GetPath(CommandNode<S>* target)
        {
            std::vector<std::vector<CommandNode<S>*>> nodes;
            AddPaths(root.get(), nodes, {});

            for (std::vector<CommandNode<S>*>& list : nodes) {
                if (list.back() == target) {
                    std::vector<std::string> result;
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
        CommandNode<S>* FindNode(std::vector<std::string> const& path) {
            CommandNode<S>* node = root.get();
            for (auto& name : path) {
                node = node->GetChild(name);
                if (node == nullptr) {
                    return nullptr;
                }
            }
            return node;
        }

    public:
        /**
        Scans the command tree for potential ambiguous commands.

        This is a shortcut for CommandNode::FindAmbiguities(AmbiguityConsumer) on GetRoot().

        Ambiguities are detected by testing every CommandNode::GetExamples() on one node verses every sibling
        node. This is not fool proof, and relies a lot on the providers of the used argument types to give good examples.

        \param consumer a callback to be notified of potential ambiguities
        */
        void FindAmbiguities(AmbiguityConsumer<S> consumer) {
            if (!consumer) return;
            root->FindAmbiguities(consumer);
        }

    private:
        void AddPaths(CommandNode<S>* node, std::vector<std::vector<CommandNode<S>*>>& result, std::vector<CommandNode<S>*> parents) {
            parents.push_back(node);
            result.push_back(parents);

            for (auto const& [name, child] : node->GetChildren()) {
                AddPaths(child.get(), result, parents);
            }
        }

    private:
        std::shared_ptr<RootCommandNode<S>> root;
        ResultConsumer<S> consumer = [](CommandContext<S>& context, bool success, int result) {};
    };
}
