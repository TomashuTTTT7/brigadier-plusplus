/*
MIT License

Copyright (c) 2022 Tomasz Karpiñski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <set>
#include <string>
#include <string_view>
#include <cstring>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <future>
#include <tuple>
#include <algorithm>
#include <limits>
#include <optional>

// Following code makes that you don't have to specify command source type inside arguments.
// Command source type is automatically distributed from dispatcher.

// Default registration for arguments with or without template, without specialization
#define REGISTER_ARGTYPE(type, name)                                      \
using name = type

// Registration for arguments with template parameters
#define REGISTER_ARGTYPE_TEMPL(type, name)                                \
template<typename... Args>                                                \
using name = type<Args...>

// Registration for arguments with specialized templates
#define REGISTER_ARGTYPE_SPEC(type, name, ...)                            \
using name = type<__VA_ARGS__>

// Registration for arguments with specialized templates and template parameters
#define REGISTER_ARGTYPE_SPEC_TEMPL(type, name, ...)                      \
template<typename... Args>                                                \
using name = type<__VA_ARGS__, Args...>

#ifdef __has_include
#  if __has_include("nameof.hpp")
#    include "nameof.hpp"
#    define HAS_NAMEOF
#  endif
#  if __has_include("magic_enum.hpp")
#    include "magic_enum.hpp"
#    define HAS_MAGICENUM
#  endif
#endif

namespace brigadier
{
    template<typename S>
    class CommandNode;
    template<typename S>
    class CommandContext;

    template<typename S>
    class IArgumentCommandNode;
    template<typename S>
    class LiteralCommandNode;
    template<typename S>
    class RootCommandNode;
    template<typename S>
    class CommandDispatcher;

    template<typename S, typename T, typename node_type>
    class ArgumentBuilder;
    template<typename S>
    class MultiArgumentBuilder;
    template<typename S>
    class LiteralArgumentBuilder;
    template<typename S, typename T>
    class RequiredArgumentBuilder;


    class StringReader
    {
    private:
        static constexpr char SYNTAX_ESCAPE = '\\';
        static constexpr char SYNTAX_SINGLE_QUOTE = '\'';
        static constexpr char SYNTAX_DOUBLE_QUOTE = '"';

    public:
        StringReader(std::string_view string) : string(string) {}
        StringReader() {}

        inline std::string_view GetString()           const { return string; }
        inline void             SetCursor(int cursor)       { this->cursor = cursor; }
        inline int              GetRemainingLength()  const { return string.length() - cursor; }
        inline int              GetTotalLength()      const { return string.length(); }
        inline int              GetCursor()           const { return cursor; }
        inline std::string_view GetRead()             const { return string.substr(0, cursor); }
        inline std::string_view GetRemaining()        const { return string.substr(cursor); }
        inline bool             CanRead(int length)   const { return (size_t)(cursor + length) <= string.length(); }
        inline bool             CanRead()             const { return CanRead(1); }
        inline char             Peek()                const { return Peek(0); }
        inline char             Peek(int offset)      const { return string.at(cursor + offset); }
        inline char             Read()                      { return string.at(cursor++); }
        inline void             Skip()                      { cursor++; }

        inline static bool IsQuotedStringStart(char c)
        {
            return c == SYNTAX_DOUBLE_QUOTE || c == SYNTAX_SINGLE_QUOTE;
        }

        inline void SkipWhitespace()
        {
            while (CanRead() && std::isspace(Peek())) {
                Skip();
            }
        }

        template<typename T>
        inline T ReadValue();

        inline static bool IsAllowedInUnquotedString(char c)
        {
            return (c >= '0' && c <= '9')
                || (c >= 'A' && c <= 'Z')
                || (c >= 'a' && c <= 'z')
                || (c == '_' || c == '-')
                || (c == '.' || c == '+');
        }

        template<bool allow_float = true, bool allow_negative = true>
        inline static bool IsAllowedNumber(char c)
        {
            return c >= '0' && c <= '9' || (allow_float && c == '.') || (allow_negative && c == '-');
        }

        inline std::string_view ReadUnquotedString();
        inline std::string      ReadQuotedString();
        inline std::string      ReadStringUntil(char terminator);
        inline std::string      ReadString();
        inline void             Expect(char c);

    private:
        std::string_view string;
        int cursor = 0;
    };

    
    class BuiltInExceptionProvider;

    struct ExceptionContext
    {
        ExceptionContext(std::nullptr_t) {}
        ExceptionContext() {}
        ExceptionContext(StringReader const& reader) : input(reader.GetString()), cursor(reader.GetCursor()) {}
        std::string_view input;
        int cursor = -1;
    };

    class CommandSyntaxException
    {
    public:
        static inline const int context_amount = 10;
        using BuiltInExceptions = BuiltInExceptionProvider;
        template<typename... Args>
        CommandSyntaxException(ExceptionContext ctx, Args&&... args) : ctx(ctx), msg(std::move(CreateMessageApplyContext(ctx, args...))) {}
        std::string const& What() { return msg; }
    private:
        template<typename T>
        static inline void Add(std::ostringstream& stream, T&& value)
        {
            stream << value;
        }
        template<typename T, typename... Args>
        static inline void Add(std::ostringstream& stream, T&& value, Args&&... args)
        {
            stream << value;
            Add(stream, args...);
        }
        template<typename... Args>
        static inline std::string CreateMessage(Args&&... args)
        {
            std::ostringstream s;
            Add(s, args...);
            return s.str();
        }
        template<typename... Args>
        static inline std::string CreateMessageApplyContext(ExceptionContext ctx, Args&&... args)
        {
            if (ctx.cursor < 0) return CreateMessage(args...);
            else return CreateMessage(args..., " at position ", ctx.cursor, ": ", ctx.cursor > context_amount ? "..." : "", ctx.input.substr((std::max)(0, ctx.cursor - context_amount), context_amount), "<--[HERE]");
        }
    public:
        int GetCursor() const
        {
            return ctx.cursor;
        }
        std::string_view GetInput() const
        {
            return ctx.input;
        }
    private:
        ExceptionContext ctx;
        std::string msg;
    };

    class BuiltInExceptionProvider
    {
    public:
        template<typename T0, typename T1> static inline CommandSyntaxException ValueTooLow                        (ExceptionContext ctx, T0 found, T1 min)    { return CommandSyntaxException(ctx, "Value must not be less than ", min, ", found ", found); }
        template<typename T0, typename T1> static inline CommandSyntaxException ValueTooHigh                       (ExceptionContext ctx, T0 found, T1 max)    { return CommandSyntaxException(ctx, "Value must not be more than ", max, ", found ", found); }
        template<typename T0>              static inline CommandSyntaxException LiteralIncorrect                   (ExceptionContext ctx, T0 const& expected)  { return CommandSyntaxException(ctx, "Expected literal ", expected); }
                                           static inline CommandSyntaxException ReaderExpectedStartOfQuote         (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Expected quote to start a string"); }
                                           static inline CommandSyntaxException ReaderExpectedEndOfQuote           (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Unclosed quoted string"); }
        template<typename T0>              static inline CommandSyntaxException ReaderInvalidEscape                (ExceptionContext ctx, T0 const& character) { return CommandSyntaxException(ctx, "Invalid escape sequence '", character, "' in quoted string"); }
        template<typename T0>              static inline CommandSyntaxException ReaderInvalidValue                 (ExceptionContext ctx, T0 const& value)     { return CommandSyntaxException(ctx, "Invalid value '", value, "'"); }
                                           static inline CommandSyntaxException ReaderExpectedValue                (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Expected value"); }
        template<typename T0>              static inline CommandSyntaxException ReaderExpectedSymbol               (ExceptionContext ctx, T0 const& symbol)    { return CommandSyntaxException(ctx, "Expected '", symbol, "'"); }
        template<typename T0>              static inline CommandSyntaxException ReaderExpectedOneOf                (ExceptionContext ctx, T0 const& symbols)   { return CommandSyntaxException(ctx, "Expected one of `", symbols, "`"); }
                                           static inline CommandSyntaxException DispatcherUnknownCommand           (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Unknown command"); }
                                           static inline CommandSyntaxException DispatcherUnknownArgument          (ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Incorrect argument for command"); }
                                           static inline CommandSyntaxException DispatcherExpectedArgumentSeparator(ExceptionContext ctx)                      { return CommandSyntaxException(ctx, "Expected whitespace to end one argument, but found trailing data"); }
        template<typename T0>              static inline CommandSyntaxException DispatcherParseException           (ExceptionContext ctx, T0 const& message)   { return CommandSyntaxException(ctx, "Could not parse command: ", message); }
    };

    std::string_view StringReader::ReadUnquotedString()
    {
        size_t start = cursor;
        while (CanRead() && IsAllowedInUnquotedString(Peek())) {
            Skip();
        }
        return string.substr(start, cursor - start);
    }

    std::string StringReader::ReadQuotedString()
    {
        if (!CanRead()) {
            return {};
        }
        char next = Peek();
        if (!IsQuotedStringStart(next)) {
            throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedStartOfQuote(*this);
        }
        Skip();
        return ReadStringUntil(next);
    }

    std::string StringReader::ReadStringUntil(char terminator)
    {
        std::string result;
        result.reserve(GetRemainingLength());

        bool escaped = false;
        while (CanRead()) {
            char c = Read();
            if (escaped) {
                if (c == terminator || c == SYNTAX_ESCAPE) {
                    result += c;
                    escaped = false;
                }
                else {
                    SetCursor(GetCursor() - 1);
                    throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidEscape(*this, c);
                }
            }
            else if (c == SYNTAX_ESCAPE) {
                escaped = true;
            }
            else if (c == terminator) {
                return result;
            }
            else {
                result += c;
            }
        }

        throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedEndOfQuote(*this);
    }

    std::string StringReader::ReadString()
    {
        if (!CanRead()) {
            return {};
        }
        char next = Peek();
        if (IsQuotedStringStart(next)) {
            Skip();
            return ReadStringUntil(next);
        }
        return std::string(ReadUnquotedString());
    }

    void StringReader::Expect(char c)
    {
        if (!CanRead() || Peek() != c) {
            throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedSymbol(*this, c);
        }
        Skip();
    }

    template<typename T>
    T StringReader::ReadValue()
    {
        int start = cursor;
        std::string value;
        if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
        {
            while (CanRead() && IsAllowedNumber<std::is_floating_point_v<T>, std::is_signed_v<T>>(Peek())) {
                Skip();
            }
            value = string.substr(start, cursor - start);
        }
        else
        {
            value = ReadString();
        }

        if (value.empty()) {
            throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedValue(*this);
        }

        if constexpr (std::is_same_v<T, bool>)
        {
            /**/ if (value == "true")
                return true;
            else if (value == "false")
                return false;
            else
            {
                cursor = start;
                throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidValue(*this, value);
            }
        }
        else
        {
            T ret{};
            std::istringstream s(value);
            s >> ret;

            if (s.eof() && !s.bad() && !s.fail())
                return ret;
            else
            {
                cursor = start;
                throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidValue(*this, value);
            }
        }
    }

    class StringRange
    {
    public:
        StringRange(int start, const int end) : start(start), end(end) {}

        inline int GetStart() const { return start; }
        inline int GetEnd()   const { return end; }

        inline static StringRange At(int pos) { return StringRange(pos, pos); }
        inline static StringRange Between(int start, const int end) { return StringRange(start, end); }
        inline static StringRange Encompassing(StringRange const& a, StringRange const& b) {
            return StringRange((std::min)(a.GetStart(), b.GetStart()), (std::max)(a.GetEnd(), b.GetEnd()));
        }

        inline std::string_view Get(StringReader reader) const {
            return reader.GetString().substr(start, end - start);
        }
        inline std::string_view Get(std::string_view string) const {
            return string.substr(start, end - start);
        }

        inline bool IsEmpty()   const { return start == end; }
        inline int  GetLength() const { return end - start; }

        inline bool operator==(StringRange const& other) const { return (start == other.start && end == other.end); }
    private:
        int start = 0;
        int end = 0;
    };

    class Suggestions;
    class SuggestionsBuilder;

    class Suggestion
    {
    public:
        Suggestion(StringRange range, std::string_view text, std::string_view tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        Suggestion(StringRange range, std::string_view text) : range(std::move(range)), text(std::move(text)) {}

        inline StringRange GetRange() const { return range; }
        inline std::string const& GetText() const { return text; }
        inline std::string_view GetTooltip() const { return tooltip; }

        std::string Apply(std::string_view input) const
        {
            if (range.GetStart() == 0 && range.GetEnd() == input.length()) {
                return text;
            }
            std::string result;
            result.reserve(range.GetStart() + text.length() + input.length() - (std::min)(range.GetEnd(), (int)input.length()));
            if (range.GetStart() > 0) {
                result.append(input.substr(0, range.GetStart()));
            }
            result.append(text);
            if ((size_t)range.GetEnd() < input.length()) {
                result.append(input.substr(range.GetEnd()));
            }
            return result;
        }

        void Expand(std::string_view command, StringRange range)
        {
            if (this->range == range)
                return;

            if (range.GetStart() < this->range.GetStart()) {
                text.insert(0, command.substr(range.GetStart(), this->range.GetStart() - range.GetStart()));
            }
            if (range.GetEnd() > this->range.GetEnd()) {
                text.append(command.substr(this->range.GetEnd(), range.GetEnd() - this->range.GetEnd()));
            }

            this->range = range;
        }
    protected:
        friend class Suggestions;
        friend class SuggestionsBuilder;
        Suggestion(std::string text, StringRange range, std::string_view tooltip) : range(std::move(range)), text(std::move(text)), tooltip(std::move(tooltip)) {}
        Suggestion(std::string text, StringRange range) : range(std::move(range)), text(std::move(text)) {}
    private:
        StringRange range;
        std::string text;
        std::string_view tooltip;
    };

    struct CompareNoCase {
        inline bool operator() (Suggestion const& a, Suggestion const& b) const
        {
#ifdef __unix__
            return strcasecmp(a.GetText().c_str(), b.GetText().c_str()) < 0;
#else
            return _stricmp(a.GetText().c_str(), b.GetText().c_str()) < 0;
#endif
        }
    };

    class Suggestions
    {
    public:
        Suggestions(StringRange range, std::set<Suggestion, CompareNoCase> suggestions) : range(std::move(range)), suggestions(std::move(suggestions)) {}
        Suggestions() : Suggestions(StringRange::At(0), {}) {}

        inline StringRange GetRange() const { return range; }
        inline std::set<Suggestion, CompareNoCase> const& GetList() const { return suggestions; }
        inline bool IsEmpty() { return suggestions.empty(); }

        static inline std::future<Suggestions> Empty();

        static inline Suggestions Merge(std::string_view command, std::vector<Suggestions> const& input, bool* cancel = nullptr)
        {
            /**/ if (input.empty()) return {};
            else if (input.size() == 1) return input.front();

            std::vector<Suggestion> suggestions;

            for (auto& sugs : input)
            {
                suggestions.insert(suggestions.end(), sugs.GetList().begin(), sugs.GetList().end());
            }
            return Suggestions::Create(command, suggestions, cancel);
        }
        static inline Suggestions Create(std::string_view command, std::vector<Suggestion>& suggestions, bool* cancel = nullptr)
        {
            if (suggestions.empty()) return {};
            int start = std::numeric_limits<int>::max();
            int end = std::numeric_limits<int>::min();
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return {};
                start = (std::min)(suggestion.GetRange().GetStart(), start);
                end = (std::max)(suggestion.GetRange().GetEnd(), end);
            }
            std::set<Suggestion, CompareNoCase> suggest;
            StringRange range = StringRange(start, end);
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return {}; // Suggestions(range, std::move(suggest));
                suggestion.Expand(command, range);
            }
            for (auto& suggestion : suggestions) {
                if (cancel && *cancel) return Suggestions(range, std::move(suggest));
                suggest.insert(std::move(suggestion));
            }
            return Suggestions(range, std::move(suggest));
        }
    private:
        StringRange range;
        std::set<Suggestion, CompareNoCase> suggestions;
    };

    std::future<Suggestions> Suggestions::Empty()
    {
        std::promise<Suggestions> f;
        f.set_value(Suggestions());
        return f.get_future();
    }

    class SuggestionsBuilder
    {
    public:
        SuggestionsBuilder(std::string_view input, std::string_view inputLowerCase, int start, bool* cancel = nullptr) : input(input), inputLowerCase(inputLowerCase), start(start), remaining(input.substr(start)), remainingLowerCase(inputLowerCase.substr(start)), cancel(cancel) {}

        inline int GetStart() const { return start; }
        inline std::string_view GetInput() const { return input; }
        inline std::string_view GetInputLowerCase() const { return inputLowerCase; }
        inline std::string_view GetRemaining() const { return remaining; }
        inline std::string_view GetRemainingLowerCase() const { return remainingLowerCase; }

        Suggestions Build(bool* cancel = nullptr)
        {
            auto ret = Suggestions::Create(input, result, cancel);
            if (cancel != nullptr) *cancel = false;
            result.clear();
            return ret;
        }
        inline std::future<Suggestions> BuildFuture()
        {
            return std::async(std::launch::async, &SuggestionsBuilder::Build, this, this->cancel);
        }

        inline SuggestionsBuilder& Suggest(std::string_view text)
        {
            if (text == remaining) return *this;

            result.emplace_back(StringRange::Between(start, input.length()), text);
            return *this;
        }
        inline SuggestionsBuilder& Suggest(std::string_view text, std::string_view tooltip)
        {
            if (text == remaining) return *this;

            result.emplace_back(StringRange::Between(start, input.length()), text, tooltip);
            return *this;
        }
        template<typename T>
        SuggestionsBuilder& Suggest(T value)
        {
            result.emplace_back(std::move(std::to_string(value)), StringRange::Between(start, input.length()));
            return *this;
        }
        template<typename T>
        SuggestionsBuilder& Suggest(T value, std::string_view tooltip)
        {
            result.emplace_back(std::move(std::to_string(value)), StringRange::Between(start, input.length()), tooltip);
            return *this;
        }
        inline int AutoSuggest(std::string_view text, std::string_view input)
        {
            if (text.rfind(input.substr(0, text.length()), 0) == 0)
            {
                Suggest(text);
                return 1;
            }
            return 0;
        }
        inline int AutoSuggest(std::string_view text, std::string_view tooltip, std::string_view input)
        {
            if (text.rfind(input.substr(0, text.length()), 0) == 0)
            {
                Suggest(text, tooltip);
                return 1;
            }
            return 0;
        }
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        int AutoSuggest(T value, std::string_view input)
        {
            std::string val = std::to_string(value);
            if (val.rfind(input.substr(0, val.length()), 0) == 0)
            {
                result.emplace_back(std::move(val), StringRange::Between(start, input.length()));
                return 1;
            }
            return 0;
        }
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        int AutoSuggest(T value, std::string_view tooltip, std::string_view input)
        {
            std::string val = std::to_string(value);

            if (val.rfind(input.substr(0, val.length()), 0) == 0)
            {
                result.emplace_back(std::move(val), StringRange::Between(start, input.length()), tooltip);
                return 1;
            }
            return 0;
        }
        template<typename Container>
        inline int AutoSuggest(Container const& init)
        {
            int counter = 0;
            for (auto& val : init)
            {
                counter += AutoSuggest(val, GetRemaining());
            }
            return counter;
        }
        template<typename Container>
        inline int AutoSuggestLowerCase(Container const& init)
        {
            int counter = 0;
            for (auto& val : init)
            {
                counter += AutoSuggest(val, GetRemainingLowerCase());
            }
            return counter;
        }

        inline SuggestionsBuilder& Add(SuggestionsBuilder const& other)
        {
            result.insert(result.end(), other.result.begin(), other.result.end());
            return *this;
        }

        inline void SetOffset(int start)
        {
            this->start = start;
            remaining = input.substr(start);
            remainingLowerCase = inputLowerCase.substr(start);

            Restart();
        }
        inline void Restart()
        {
            result.clear();
        }

        ~SuggestionsBuilder() = default;
    private:
        int start = 0;
        std::string_view input;
        std::string_view inputLowerCase;
        std::string_view remaining;
        std::string_view remainingLowerCase;
        std::vector<Suggestion> result;
        bool* cancel = nullptr;
    };

    template<typename... Ts>
    using Predicate = bool(*)(Ts&&... args);
    template<typename S>
    using AmbiguityConsumer = void(*)(CommandNode<S>* parent, CommandNode<S>* child, CommandNode<S>* sibling, std::set<std::string_view>& inputs);
    template<typename S>
    using Command = int(*)(CommandContext<S>& context);
    template<typename S>
    using RedirectModifier = std::vector<S>(*)(CommandContext<S>& context);
    template<typename S>
    using SingleRedirectModifier = S(*)(CommandContext<S>& context);
    template<typename S>
    using ResultConsumer = void(*)(CommandContext<S>& context, bool success, int result);
    template<typename S>
    using SuggestionProvider = std::future<Suggestions>(*)(CommandContext<S>& context, SuggestionsBuilder& builder);

    enum class CommandNodeType
    {
        RootCommandNode,
        LiteralCommandNode,
        ArgumentCommandNode
    };

    template<typename S>
    class CommandNode
    {
    public:
        CommandNode(Command<S> command, Predicate<S&> requirement, std::shared_ptr<CommandNode<S>> redirect, RedirectModifier<S> modifier, const bool forks)
            : command(std::move(command))
            , requirement(std::move(requirement))
            , redirect(std::move(redirect))
            , modifier(std::move(modifier))
            , forks(std::move(forks))
        {}
        CommandNode() {}
        virtual ~CommandNode() = default;
    public:
        inline Command<S> GetCommand() const
        {
            return command;
        }

        inline std::map<std::string, std::shared_ptr<CommandNode<S>>, std::less<>> const& GetChildren() const
        {
            return children;
        }

        inline std::shared_ptr<CommandNode<S>> GetChild(std::string_view name) const
        {
            auto found = children.find(name);
            if (found != children.end())
                return found->second;
            return nullptr;
        }

        inline std::shared_ptr<CommandNode<S>> GetRedirect() const
        {
            return redirect;
        }

        inline RedirectModifier<S> GetRedirectModifier() const
        {
            return modifier;
        }

        inline Predicate<S&> GetRequirement()
        {
            return requirement;
        }

        inline bool IsFork() const
        {
            return forks;
        }

        inline bool CanUse(S& source)
        {
            if (requirement)
                return requirement(source);
            else return true;
        }

        void AddChild(std::shared_ptr<CommandNode<S>> node)
        {
            if (node == nullptr)
                return;

            if (node->GetNodeType() == CommandNodeType::RootCommandNode) {
                throw std::runtime_error("Cannot add a RootCommandNode as a child to any other CommandNode");
            }

            auto child = children.find(node->GetName());
            if (child != children.end()) {
                // We've found something to merge onto
                auto child_node = child->second;

                if (child_node->GetNodeType() != node->GetNodeType())
                    throw std::runtime_error("Node type (literal/argument) mismatch!");

                auto node_command = node->GetCommand();
                if (node_command != nullptr) {
                    child_node->command = node_command;
                }
                for (auto& [name, grandchild] : node->GetChildren()) {
                    child_node->AddChild(grandchild);
                }
            }
            else {
                children.emplace(node->GetName(), node);
                if (node->GetNodeType() == CommandNodeType::LiteralCommandNode) {
                    literals.emplace_back(std::move(std::static_pointer_cast<LiteralCommandNode<S>>(std::move(node))));
                }
                else if (node->GetNodeType() == CommandNodeType::ArgumentCommandNode) {
                    arguments.emplace_back(std::move(std::static_pointer_cast<IArgumentCommandNode<S>>(std::move(node))));
                }
            }
        }

        void FindAmbiguities(AmbiguityConsumer<S> consumer)
        {
            for (auto [child_name, child] : children) {
                for (auto [sibling_name, sibling] : children) {
                    if (child == sibling)
                        continue;

                    std::set<std::string_view> matches;

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

        std::tuple<std::shared_ptr<CommandNode<S>>*, size_t> GetRelevantNodes(StringReader& input)
        {
            if (literals.size() > 0) {
                int cursor = input.GetCursor();
                while (input.CanRead() && input.Peek() != ' ') {
                    input.Skip();
                }
                std::string_view text = input.GetString().substr(cursor, input.GetCursor() - cursor);
                input.SetCursor(cursor);
                auto literal = children.find(text);
                if (literal != children.end() && literal->second->GetNodeType() == CommandNodeType::LiteralCommandNode) {
                    return std::tuple<std::shared_ptr<CommandNode<S>>*, size_t>(&literal->second, 1);
                }
                else {
                    return std::tuple<std::shared_ptr<CommandNode<S>>*, size_t>((std::shared_ptr<CommandNode<S>>*)arguments.data(), arguments.size());
                }
            }
            else {
                return std::tuple<std::shared_ptr<CommandNode<S>>*, size_t>((std::shared_ptr<CommandNode<S>>*)arguments.data(), arguments.size());
            }
        }

        bool HasCommand()
        {
            if (GetCommand() != nullptr) return true;
            for (auto [name, child] : children)
                if (child && child->HasCommand())
                    return true;
            return false;
        }
    public:
        virtual std::string const& GetName() = 0;
        virtual std::string GetUsageText() = 0;
        virtual std::vector<std::string_view> GetExamples() = 0;
        virtual void Parse(StringReader& reader, CommandContext<S>& contextBuilder) = 0;
        virtual std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder) = 0;

        virtual CommandNodeType GetNodeType() = 0;
    protected:
        template<typename _S, typename T, typename node_type>
        friend class ArgumentBuilder;
        template<typename _S>
        friend class MultiArgumentBuilder;
        template<typename _S>
        friend class CommandDispatcher;
        template<typename _S, typename T>
        friend class RequiredArgumentBuilder;
        template<typename _S>
        friend class LiteralArgumentBuilder;

        virtual bool IsValidInput(std::string_view input) = 0;
        virtual std::string_view GetSortedKey() = 0;
    private:
        std::map<std::string, std::shared_ptr<CommandNode<S>>, std::less<>> children;
        std::vector<std::shared_ptr<LiteralCommandNode<S>>> literals;
        std::vector<std::shared_ptr<IArgumentCommandNode<S>>> arguments;
        Command<S> command = nullptr;
        Predicate<S&> requirement = nullptr;
        std::shared_ptr<CommandNode<S>> redirect = nullptr;
        RedirectModifier<S> modifier = nullptr;
        bool forks = false;
    };

    template<typename S>
    class RootCommandNode : public CommandNode<S>
    {
    public:
        RootCommandNode() : CommandNode<S>(nullptr, [](S&) { return true; }, nullptr, [](auto s)->std::vector<S> { return { s.GetSource() }; }, false) {}

        virtual ~RootCommandNode() = default;
        virtual std::string const& GetName() { static const std::string blank; return blank; }
        virtual std::string GetUsageText() { return {}; }
        virtual std::vector<std::string_view> GetExamples() { return {}; }
        virtual void Parse(StringReader& reader, CommandContext<S>& contextBuilder) {}
        virtual std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            return Suggestions::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::RootCommandNode; }
    protected:
        virtual bool IsValidInput(std::string_view input) { return false; }
        virtual std::string_view GetSortedKey() { return {}; }
    };

    template<typename S>
    class LiteralCommandNode : public CommandNode<S>
    {
    public:
        LiteralCommandNode(std::string_view literal, std::shared_ptr<Command<S>> command, Predicate<S&> requirement, std::shared_ptr<CommandNode<S>> redirect, RedirectModifier<S> modifier, const bool forks)
            : CommandNode<S>(command, requirement, redirect, modifier, forks)
            , literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](char c) { return std::tolower(c); });
        }
        LiteralCommandNode(std::string_view literal)
            : literal(literal)
            , literalLowerCase(literal)
        {
            std::transform(literalLowerCase.begin(), literalLowerCase.end(), literalLowerCase.begin(), [](char c) { return std::tolower(c); });
        }
        virtual ~LiteralCommandNode() = default;
        virtual std::string const& GetName() { return literal; }
        virtual std::string GetUsageText() { return literal; }
        virtual std::vector<std::string_view> GetExamples() { return { literal }; }
        virtual void Parse(StringReader& reader, CommandContext<S>& contextBuilder)
        {
            int start = reader.GetCursor();
            int end = Parse(reader);
            if (end > -1) {
                contextBuilder.WithNode(this, StringRange::Between(start, end));
                return;
            }

            throw CommandSyntaxException::BuiltInExceptions::LiteralIncorrect(reader, literal);
        }
        virtual std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            if (builder.AutoSuggest(literalLowerCase, builder.GetRemainingLowerCase()))
                return builder.BuildFuture();
            else
                return Suggestions::Empty();
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::LiteralCommandNode; }
    protected:
        virtual bool IsValidInput(std::string_view input) {
            StringReader reader(input);
            return Parse(reader) > -1;
        }
        virtual std::string_view GetSortedKey() { return literal; }
    private:
        int Parse(StringReader& reader)
        {
            int start = reader.GetCursor();
            if (reader.CanRead(literal.length())) {
                if (reader.GetString().substr(start, literal.length()) == literal) {
                    int end = start + literal.length();
                    reader.SetCursor(end);
                    if (!reader.CanRead() || reader.Peek() == ' ') {
                        return end;
                    }
                    else {
                        reader.SetCursor(start);
                    }
                }
            }
            return -1;
        }
    private:
        std::string literal;
        std::string literalLowerCase;
    };
    

    template<typename T>
    class ArgumentType
    {
    public:
        using type = T;
    public:
        template<typename... Args>
        static inline CommandSyntaxException CommandParseException(ExceptionContext const& ctx, Args&&... args)
        {
            return CommandSyntaxException(ctx, "Error during parsing value of type '", GetTypeName(), "': ", std::forward<Args>(args)...);
        }

        T Parse(StringReader& reader)
        {
            return reader.ReadValue<T>();
        }

        template<typename S>
        std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            return Suggestions::Empty();
        }

        static constexpr std::string_view GetTypeName()
        {
#ifdef HAS_NAMEOF
            return NAMEOF_TYPE(T).substr(NAMEOF_TYPE(T).find(NAMEOF_SHORT_TYPE(T))); // Short type + template list
#else
            return "";
#endif
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            return {};
        }
    };
    REGISTER_ARGTYPE_TEMPL(ArgumentType, Type);

    enum class StringArgType {
        SINGLE_WORD,
        QUOTABLE_PHRASE,
        GREEDY_PHRASE
    };

    template<StringArgType strType>
    class StringArgumentType : public ArgumentType<std::string>
    {
    public:
        StringArgumentType() {};

        StringArgType GetType()
        {
            return strType;
        }

        std::string Parse(StringReader& reader) {
            if (strType == StringArgType::GREEDY_PHRASE)
            {
                std::string text(reader.GetRemaining());
                reader.SetCursor(reader.GetTotalLength());
                return text;
            }
            else if (strType == StringArgType::SINGLE_WORD)
            {
                return std::string(reader.ReadUnquotedString());
            }
            else
            {
                return reader.ReadString();
            }
        }

        static constexpr std::string_view GetTypeName()
        {
            if constexpr (strType == StringArgType::GREEDY_PHRASE)
            {
                return "words";
            }
            else if constexpr (strType == StringArgType::SINGLE_WORD)
            {
                return "word";
            }
            else
            {
                return "string";
            }
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            if constexpr (strType == StringArgType::GREEDY_PHRASE)
            {
                return { "word", "words with spaces", "\"and symbols\"" };
            }
            else if constexpr (strType == StringArgType::SINGLE_WORD)
            {
                return { "word", "words_with_underscores" };
            }
            else
            {
                return { "\"quoted phrase\"", "word", "\"\"" };
            }
        }

        static std::string EscapeIfRequired(std::string_view input) {
            for (auto c : input) {
                if (!StringReader::IsAllowedInUnquotedString(c)) {
                    return Escape(std::move(input));
                }
            }
            return std::string(input);
        }

        static std::string Escape(std::string_view input) {
            std::string result = "\"";

            for (auto c : input) {
                if (c == '\\' || c == '\"') {
                    result += '\\';
                }
                result += c;
            }

            result += '\"';
            return result;
        }
    };
    REGISTER_ARGTYPE_SPEC(StringArgumentType, Word, StringArgType::SINGLE_WORD);
    REGISTER_ARGTYPE_SPEC(StringArgumentType, String, StringArgType::QUOTABLE_PHRASE);
    REGISTER_ARGTYPE_SPEC(StringArgumentType, GreedyString, StringArgType::GREEDY_PHRASE);

    class BoolArgumentType : public ArgumentType<bool>
    {
    public:
        template<typename S>
        std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            builder.AutoSuggestLowerCase(std::initializer_list({ "true", "false" }));
            return builder.BuildFuture();
        }
        static constexpr std::string_view GetTypeName()
        {
            return "bool";
        }
        static inline std::vector<std::string_view> GetExamples()
        {
            return { "true", "false" };
        }
    };
    REGISTER_ARGTYPE(BoolArgumentType, Bool);

    class CharArgumentType : public ArgumentType<char>
    {
    public:
        char Parse(StringReader& reader)
        {
            if (reader.CanRead())
                return reader.Read();
            else throw CommandSyntaxException::BuiltInExceptions::ReaderExpectedValue(reader);
        }

        static constexpr std::string_view GetTypeName()
        {
            return "char";
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            return { "c", "@", "." };
        }
    };
    REGISTER_ARGTYPE(CharArgumentType, Char);

    template<typename T>
    class ArithmeticArgumentType : public ArgumentType<T>
    {
        static_assert(std::is_arithmetic_v<T>, "T must be a number");
    public:
        ArithmeticArgumentType(T minimum = std::numeric_limits<T>::lowest(), T maximum = std::numeric_limits<T>::max()) : minimum(minimum), maximum(maximum) {}

        T GetMinimum() {
            return minimum;
        }
        T GetMaximum() {
            return maximum;
        }

        T Parse(StringReader& reader)
        {
            int start = reader.GetCursor();
            T result = reader.ReadValue<T>();
            if (result < minimum) {
                reader.SetCursor(start);
                throw CommandSyntaxException::BuiltInExceptions::ValueTooLow(reader, result, minimum);
            }
            if (result > maximum) {
                reader.SetCursor(start);
                throw CommandSyntaxException::BuiltInExceptions::ValueTooHigh(reader, result, maximum);
            }
            return result;
        }

        static constexpr std::string_view GetTypeName()
        {
            /**/ if constexpr (std::is_floating_point_v<T>)
                return "float";
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_signed_v<T>)
                    return "int";
                else
                    return "uint";
            }
            else return ArgumentType<T>::GetTypeName();
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            /**/ if constexpr (std::is_floating_point_v<T>)
                return { "0", "1.2", ".5", "-1", "-.5", "-1234.56" };
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_signed_v<T>)
                    return { "0", "123", "-123" };
                else
                    return { "0", "123" };
            }
            else return {};
        }
    private:
        T minimum;
        T maximum;
    };
    REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Float, float);
    REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Double, double);
    REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Integer, int);
    REGISTER_ARGTYPE_SPEC(ArithmeticArgumentType, Long, long long);
    REGISTER_ARGTYPE_TEMPL(ArithmeticArgumentType, Number);

#ifdef HAS_MAGICENUM
    template<typename T>
    class EnumArgumentType : public ArgumentType<T>
    {
        static_assert(std::is_enum_v<T>, "T must be enum");
    public:
        T Parse(StringReader& reader)
        {
            int start = reader.GetCursor();
            auto str = reader.ReadString();
            auto result = magic_enum::enum_cast<T>(str);
            if (!result.has_value())
            {
                throw CommandSyntaxException::BuiltInExceptions::ReaderInvalidValue(reader, str);
            }
            return result.value();
        }

        template<typename S>
        std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            static constexpr auto names = magic_enum::enum_names<T>();
            builder.AutoSuggestLowerCase(names);
            return builder.BuildFuture();
        }

        static constexpr std::string_view GetTypeName()
        {
            return "enum";
        }

        static inline std::vector<std::string_view> GetExamples()
        {
            static constexpr auto names = magic_enum::enum_names<T>();
            return std::vector<std::string_view>(names.begin(), names.end());
        }
    };
    REGISTER_ARGTYPE_TEMPL(EnumArgumentType, Enum);
#endif

    struct TypeInfo
    {
        TypeInfo(size_t hash) : hash(hash) {}
        template<typename ArgType>
        static constexpr size_t Create() { return (((uintptr_t)(ArgType::GetTypeName().data())) + (sizeof(typename ArgType::type) << 24) + (sizeof(ArgType) << 8)); }
        inline bool operator==(TypeInfo const& other) { return hash == other.hash; }
        inline bool operator!=(TypeInfo const& other) { return hash != other.hash; }
        size_t hash = 0;
    };

    template<typename S>
    class IParsedArgument
    {
    public:
        IParsedArgument(int start, int end, TypeInfo typeInfo) : range(start, end), typeInfo(typeInfo) {}
        virtual ~IParsedArgument() = default;

        inline StringRange GetRange()    const { return range; }
        inline TypeInfo    GetTypeInfo() const { return typeInfo; }
    protected:
        StringRange range;
        TypeInfo typeInfo;
    };

    template<typename S, typename ArgType>
    class ParsedArgument : public IParsedArgument<S>
    {
    public:
        using T = typename ArgType::type;

        ParsedArgument(const int start, const int end, T result) : IParsedArgument<S>(start, end, TypeInfo(TypeInfo::Create<ArgType>())), result(std::move(result)) {}
        virtual ~ParsedArgument() = default;

        inline T& GetResult() { return result; }
        inline T const& GetResult() const { return result; }
    private:
        T result;
    };

    template<typename S>
    struct ParsedCommandNode
    {
    public:
        ParsedCommandNode(CommandNode<S>* node, StringRange range) : node(node), range(std::move(range)) {}

        inline CommandNode<S>* GetNode()  const { return node; }
        inline StringRange     GetRange() const { return range; }
    private:
        CommandNode<S>* node;
        StringRange range;
    };

    template<typename S>
    class SuggestionContext
    {
    public:
        SuggestionContext(CommandNode<S>* parent, int startPos) : parent(parent), startPos(startPos) {}

        CommandNode<S>* parent;
        int startPos;
    };

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
        context->source = std::move(source);
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
                context->child->Merge(*ctx->child);
                ctx->child = {};
            }
            else context->child = std::move(ctx->child);
        }
    }

    template<typename S, typename T>
    class RequiredArgumentBuilder;

    template<typename S>
    class IArgumentCommandNode : public CommandNode<S>
    {
    protected:
        IArgumentCommandNode(std::string_view name) : name(name) {}
        virtual ~IArgumentCommandNode() = default;
    public:
        virtual std::string const& GetName() {
            return name;
        }
        virtual CommandNodeType GetNodeType() { return CommandNodeType::ArgumentCommandNode; }
    protected:
        virtual std::string_view GetSortedKey() {
            return name;
        }
    protected:
        std::string name;
    };

    template<typename S, typename T>
    class ArgumentCommandNode : public IArgumentCommandNode<S>
    {
    private:
        static constexpr std::string_view USAGE_ARGUMENT_OPEN = "<";
        static constexpr std::string_view USAGE_ARGUMENT_CLOSE = ">";
    public:
        template<typename... Args>
        ArgumentCommandNode(std::string_view name, Args&&... args)
            : IArgumentCommandNode<S>(name)
            , type(std::forward<Args>(args)...)
        {}
        virtual ~ArgumentCommandNode() = default;
    public:
        inline SuggestionProvider<S> const& GetCustomSuggestions() const {
            return customSuggestions;
        }

        inline T const& GetType() {
            return type;
        }
    public:
        virtual std::string GetUsageText() {
            std::string ret;
            constexpr auto typeName = T::GetTypeName();
            ret.reserve(this->name.size() + USAGE_ARGUMENT_OPEN.size() + USAGE_ARGUMENT_CLOSE.size() + typeName.size() > 0 ? typeName.size() + 2 : 0);
            ret = USAGE_ARGUMENT_OPEN;
            if constexpr (typeName.size() > 0)
            {
                ret += typeName;
                ret += ": ";
            }
            ret += this->name;
            ret += USAGE_ARGUMENT_CLOSE;
            return ret;
        }
        virtual std::vector<std::string_view> GetExamples() {
            return type.GetExamples();
        }
        virtual void Parse(StringReader& reader, CommandContext<S>& contextBuilder) {
            int start = reader.GetCursor();
            using Type = typename T::type;
            Type result = type.Parse(reader);
            std::shared_ptr<ParsedArgument<S, T>> parsed = std::make_shared<ParsedArgument<S, T>>(start, reader.GetCursor(), std::move(result));

            contextBuilder.WithArgument(this->name, parsed);
            contextBuilder.WithNode(this, parsed->GetRange());
        }
        virtual std::future<Suggestions> ListSuggestions(CommandContext<S>& context, SuggestionsBuilder& builder)
        {
            if (customSuggestions == nullptr) {
                return type.template ListSuggestions<S>(context, builder);
            }
            else {
                return customSuggestions(context, builder);
            }
        }
    protected:
        virtual bool IsValidInput(std::string_view input) {
            try {
                StringReader reader = StringReader(input);
                type.Parse(reader);
                return !reader.CanRead() || reader.Peek() == ' ';
            }
            catch (CommandSyntaxException const&) {
                return false;
            }
        }
    private:
        friend class RequiredArgumentBuilder<S, T>;
        T type;
        SuggestionProvider<S> customSuggestions = nullptr;
    };

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

    template<typename S>
    class LiteralArgumentBuilder : public ArgumentBuilder<S, LiteralArgumentBuilder<S>, LiteralCommandNode<S>>
    {
    public:
        LiteralArgumentBuilder(std::shared_ptr<LiteralCommandNode<S>> node) : ArgumentBuilder<S, LiteralArgumentBuilder<S>, LiteralCommandNode<S>>(std::move(node)) {}
    };
    REGISTER_ARGTYPE_TEMPL(LiteralArgumentBuilder, Literal);

    // single builder
    template<typename S>
    inline LiteralArgumentBuilder<S> GetBuilder(std::shared_ptr<LiteralCommandNode<S>> node)
    {
        return LiteralArgumentBuilder<S>(std::move(node));
    }

    // new single builder
    template<typename S, typename... Args>
    inline LiteralArgumentBuilder<S> MakeLiteral(Args&&... args)
    {
        return LiteralArgumentBuilder<S>(std::make_shared<LiteralCommandNode<S>>(std::forward<Args>(args)...));
    }

    template<typename S, typename T>
    class RequiredArgumentBuilder : public ArgumentBuilder<S, RequiredArgumentBuilder<S, T>, ArgumentCommandNode<S, T>>
    {
    public:
        RequiredArgumentBuilder(std::shared_ptr<ArgumentCommandNode<S, T>> node) : ArgumentBuilder<S, RequiredArgumentBuilder<S, T>, ArgumentCommandNode<S, T>>(std::move(node)) {}

        RequiredArgumentBuilder<S, T>& Suggests(SuggestionProvider<S> provider)
        {
            this->node->suggestionsProvider = provider;
            return *this;
        }
    };
    REGISTER_ARGTYPE_TEMPL(RequiredArgumentBuilder, Argument);

    // single builder
    template<typename S, template<typename...> typename Spec, typename... Types>
    inline RequiredArgumentBuilder<S, Spec<S, Types...>> GetBuilder(std::shared_ptr<ArgumentCommandNode<S, Spec<S, Types...>>> node)
    {
        return RequiredArgumentBuilder<S, Spec<S, Types...>>(std::move(node));
    }

    // new single builder
    template<typename S, template<typename...> typename Spec, typename Type, typename... Args>
    inline RequiredArgumentBuilder<S, Spec<S, Type>> MakeArgument(Args&&... args)
    {
        return RequiredArgumentBuilder<S, Spec<S, Type>>(std::make_shared<ArgumentCommandNode<S, Spec<S, Type>>>(std::forward<Args>(args)...));
    }

    template<typename S>
    class CommandDispatcher;

    template<typename S>
    class BasicParseResults
    {
    public:
        BasicParseResults(CommandContext<S> context, StringReader reader, std::map<CommandNode<S>*, CommandSyntaxException> exceptions)
            : context(std::move(context))
            , reader(std::move(reader))
            , exceptions(std::move(exceptions))
        {}
        BasicParseResults(CommandContext<S> context, StringReader reader)
            : context(std::move(context))
            , reader(std::move(reader))
        {}

        BasicParseResults(CommandContext<S> context) : BasicParseResults(std::move(context), StringReader()) {}
    public:
        inline CommandContext<S> const& GetContext() const { return context; }
        inline StringReader      const& GetReader()  const { return reader; }
        inline std::map<CommandNode<S>*, CommandSyntaxException> const& GetExceptions() const { return exceptions; }

        inline bool IsBetterThan(BasicParseResults<S> const& other) const
        {
            if (!GetReader().CanRead() && other.GetReader().CanRead()) {
                return true;
            }
            if (GetReader().CanRead() && !other.GetReader().CanRead()) {
                return false;
            }
            if (GetExceptions().empty() && !other.GetExceptions().empty()) {
                return true;
            }
            if (!GetExceptions().empty() && other.GetExceptions().empty()) {
                return false;
            }
            return false;
        }

        inline void Reset(StringReader new_reader)
        {
            exceptions.clear();
            reader = std::move(new_reader);
        }
        inline void Reset(S source, CommandNode<S>* root, int start, StringReader reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, start);
        }
        inline void Reset(S source, CommandNode<S>* root, StringRange range, StringReader reader = {})
        {
            Reset(std::move(reader));
            context.Reset(std::move(source), root, std::move(range));
        }

    private:
        template<typename _S>
        friend class CommandDispatcher;

        CommandContext<S> context;
        std::map<CommandNode<S>*, CommandSyntaxException> exceptions;
        StringReader reader;
    };

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

        This is a shortcut to first Parse(StringReader, Object) and then Execute(BasicParseResults).

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
        \see Execute(BasicParseResults)
        \see Execute(StringReader, Object)
        */
        int Execute(std::string_view input, S source)
        {
            StringReader reader = StringReader(input);
            return Execute(reader, std::move(source));
        }

        /**
        Parses and executes a given command.

        This is a shortcut to first Parse(StringReader, Object) and then Execute(BasicParseResults).

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
        \see Execute(BasicParseResults)
        \see Execute(String, Object)
        */
        int Execute(StringReader& input, S source)
        {
            auto parse = Parse(input, std::move(source));
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
        int Execute(BasicParseResults<S>& parse)
        {
            if (parse.GetReader().CanRead()) {
                if (parse.GetExceptions().size() == 1) {
                    throw* parse.GetExceptions().begin();
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
                                catch (CommandSyntaxException const& ex) {
                                    consumer(context, false, 0);
                                    if (!forked) {
                                        throw ex;
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

        Parsing a command can never fail, you will always be provided with a new BasicParseResults.
        However, that does not mean that it will always parse into a valid command. You should inspect the returned results
        to check for validity. If its BasicParseResults::GetReader() StringReader::CanRead() then it did not finish
        parsing successfully. You can use that position as an indicator to the user where the command stopped being valid.
        You may inspect BasicParseResults::GetExceptions() if you know the parse failed, as it will explain why it could
        not find any valid commands. It may contain multiple exceptions, one for each "potential node" that it could have visited,
        explaining why it did not go down that node.

        When you eventually call Execute(BasicParseResults) with the result of this method, the above error checking
        will occur. You only need to inspect it yourself if you wish to handle that yourself.

        \param command a command string to parse
        \param source a custom "source" object, usually representing the originator of this command
        \return the result of parsing this command
        \see Parse(StringReader, Object)
        \see Execute(BasicParseResults)
        \see Execute(String, Object)
        */
        BasicParseResults<S> Parse(std::string_view command, S source)
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

        Parsing a command can never fail, you will always be provided with a new BasicParseResults.
        However, that does not mean that it will always parse into a valid command. You should inspect the returned results
        to check for validity. If its BasicParseResults::GetReader() StringReader::CanRead() then it did not finish
        parsing successfully. You can use that position as an indicator to the user where the command stopped being valid.
        You may inspect BasicParseResults::GetExceptions() if you know the parse failed, as it will explain why it could
        not find any valid commands. It may contain multiple exceptions, one for each "potential node" that it could have visited,
        explaining why it did not go down that node.

        When you eventually call Execute(BasicParseResults) with the result of this method, the above error checking
        will occur. You only need to inspect it yourself if you wish to handle that yourself.

        \param command a command string to parse
        \param source a custom "source" object, usually representing the originator of this command
        \return the result of parsing this command
        \see Parse(String, Object)
        \see Execute(BasicParseResults)
        \see Execute(String, Object)
        */
        BasicParseResults<S> Parse(StringReader& command, S source)
        {
            BasicParseResults<S> result(CommandContext<S>(std::move(source), root.get(), command.GetCursor()), command);
            ParseNodes(root.get(), result);
            return result;
        }

    private:
        void ParseNodes(CommandNode<S>* node, BasicParseResults<S>& result)
        {
            if (!node)
                return;

            S& source = result.context.GetSource();

            std::optional<BasicParseResults<S>> best_potential = {};
            std::optional<BasicParseResults<S>> current_result_ctx = {}; // delay initialization

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
                    current_result_ctx = BasicParseResults<S>(CommandContext<S>(source, result.GetContext().GetRootNode(), result.GetContext().GetRange()), result.GetReader());
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
                        BasicParseResults<S> child_result(CommandContext<S>(source, child->GetRedirect().get(), reader.GetCursor()), reader);
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
        std::future<Suggestions> GetCompletionSuggestions(BasicParseResults<S>& parse, bool* cancel = nullptr)
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
        std::future<Suggestions> GetCompletionSuggestions(BasicParseResults<S>& parse, int cursor, bool* cancel = nullptr)
        {
            return std::async(std::launch::async, [](BasicParseResults<S>* parse, int cursor, bool* cancel) {
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
