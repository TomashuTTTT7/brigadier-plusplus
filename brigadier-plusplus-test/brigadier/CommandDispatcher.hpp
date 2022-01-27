#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    Command<int> command = [](CommandContext<int>& ctx) -> int { return 42; };
    Command<int> subcommand = [](CommandContext<int>& ctx) -> int { return 100; };
    Command<int> wrongcommand = [](CommandContext<int>& ctx) -> int { Assert::Fail(); return 0; };
    int source = 0;

    TEST_CLASS(CommandDispatcherTest)
    {
        TEST_METHOD(testCreateAndExecuteCommand) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Executes(command);

            Assert::AreEqual(subject.Execute("foo", source), 42);
        }

        TEST_METHOD(testCreateAndExecuteOffsetCommand) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Executes(command);

            Assert::AreEqual(subject.Execute(InputWithOffset("/foo", 1), source), 42);
        }

        TEST_METHOD(testCreateAndMergeCommands) {
            CommandDispatcher<int> subject;
            subject.Register("base").Then<Literal>("foo").Executes(command);
            subject.Register("base").Then<Literal>("bar").Executes(command);

            Assert::AreEqual(subject.Execute("base foo", source), 42);
            Assert::AreEqual(subject.Execute("base bar", source), 42);
        }

        TEST_METHOD(testExecuteUnknownCommand) {
            CommandDispatcher<int> subject;
            subject.Register("bar");
            subject.Register("baz");

            try {
                subject.Execute("foo", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 0);
            }
        }

        TEST_METHOD(testExecuteImpermissibleCommand) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Requires([](int& src) { return false; });

            try {
                subject.Execute("foo", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 0);
            }
        }

        TEST_METHOD(testExecuteEmptyCommand) {
            CommandDispatcher<int> subject;
            subject.Register("");

            try {
                subject.Execute("", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 0);
            }
        }

        TEST_METHOD(testExecuteUnknownSubcommand) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Executes(command);

            try {
                subject.Execute("foo bar", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 4);
            }
        }

        TEST_METHOD(testExecuteIncorrectLiteral) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Executes(command).Then<Literal>("bar");

            try {
                subject.Execute("foo baz", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 4);
            }
        }

        TEST_METHOD(testExecuteAmbiguousIncorrectArgument) {
            CommandDispatcher<int> subject;
            auto foo = subject.Register("foo");
            foo.Executes(command);
            foo.Then<Literal>("bar");
            foo.Then<Literal>("baz");

            try {
                subject.Execute("foo unknown", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 4);
            }
        }

        TEST_METHOD(testExecuteSubcommand) {
            CommandDispatcher<int> subject;
            auto foo = subject.Register("foo");
            foo.Then<Literal>("a");
            foo.Then<Literal>("=").Executes(subcommand);
            foo.Then<Literal>("c");
            foo.Executes(command);

            Assert::AreEqual(subject.Execute("foo =", source), 100);
        }

        TEST_METHOD(testParseIncompleteLiteral) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Then<Literal>("bar").Executes(command);

            auto Parse = subject.Parse("foo ", source);
            Assert::AreEqual(Parse.GetReader().GetRemaining(), { " " });
            Assert::AreEqual(Parse.GetContext().GetNodes().size(), { 1 });
        }

        TEST_METHOD(testParseIncompleteArgument) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Then<Argument, Integer>("bar").Executes(command);

            auto Parse = subject.Parse("foo ", source);
            Assert::AreEqual(Parse.GetReader().GetRemaining(), { " " });
            Assert::AreEqual(Parse.GetContext().GetNodes().size(), { 1 });
        }

        TEST_METHOD(testExecuteAmbiguiousParentSubcommand) {
            CommandDispatcher<int> subject;

            auto test = subject.Register("test");
            test.Then<Argument, Integer>("incorrect").Executes(command);
            test.Then<Argument, Integer>("right").Then<Argument, Integer>("sub").Executes(subcommand);

            Assert::AreEqual(subject.Execute("test 1 2", source), 100);
        }

        TEST_METHOD(testExecuteAmbiguiousParentSubcommandViaRedirect) {
            CommandDispatcher<int> subject;

            auto real = subject.Register("test");
            real.Then<Argument, Integer>("incorrect").Executes(command);
            real.Then<Argument, Integer>("right").Then<Argument, Integer>("sub").Executes(subcommand);

            subject.Register("redirect").Redirect(real);

            Assert::AreEqual(subject.Execute("redirect 1 2", source), 100);
        }

        TEST_METHOD(testExecuteRedirectedMultipleTimes) {
            CommandDispatcher<int> subject;
            auto concreteNode = subject.Register("actual");
            concreteNode.Executes(command);
            auto redirectNode = subject.Register("redirected");
            redirectNode.Redirect(subject.GetRoot());

            std::string_view input = "redirected redirected actual";

            auto parse = subject.Parse(input, source);
            Assert::AreEqual(parse.GetContext().GetRange().Get(input), { "redirected" });
            Assert::AreEqual(parse.GetContext().GetNodes().size(), { 1 });
            Assert::AreEqual((void*)parse.GetContext().GetRootNode(), { (void*)subject.GetRoot().get() });
            AssertRange(parse.GetContext().GetNodes()[0].GetRange(), parse.GetContext().GetRange());
            Assert::AreEqual((void*)parse.GetContext().GetNodes()[0].GetNode(), (void*)redirectNode.GetCommandNode().get());

            auto child1 = parse.GetContext().GetChild();
            Assert::IsNotNull(child1);
            Assert::AreEqual(child1->GetRange().Get(input), { "redirected" });
            Assert::AreEqual(child1->GetNodes().size(), { 1 });
            Assert::AreEqual((void*)child1->GetRootNode(), { (void*)subject.GetRoot().get() });
            AssertRange(child1->GetNodes()[0].GetRange(), child1->GetRange());
            Assert::AreEqual((void*)child1->GetNodes()[0].GetNode(), (void*)redirectNode.GetCommandNode().get());

            auto child2 = child1->GetChild();
            Assert::IsNotNull(child2);
            Assert::AreEqual(child2->GetRange().Get(input), { "actual" });
            Assert::AreEqual(child2->GetNodes().size(), { 1 });
            Assert::AreEqual((void*)child2->GetRootNode(), { (void*)subject.GetRoot().get() });
            AssertRange(child2->GetNodes()[0].GetRange(), child2->GetRange());
            Assert::AreEqual((void*)child2->GetNodes()[0].GetNode(), (void*)concreteNode.GetCommandNode().get());

            Assert::AreEqual(subject.Execute(parse), 42);
        }

        TEST_METHOD(testExecuteRedirected) {
            CommandDispatcher<int> subject;

            auto concreteNode = subject.Register("actual");
            concreteNode.Executes(command);
            auto redirectNode = subject.Register("redirected");
            redirectNode.Fork(subject.GetRoot(), [](CommandContext<int>& context) -> std::vector<int> { return { 1, 2 }; });

            std::string_view input = "redirected actual";
            auto parse = subject.Parse(input, source);
            Assert::AreEqual(parse.GetContext().GetRange().Get(input), { "redirected" });
            Assert::AreEqual(parse.GetContext().GetNodes().size(), { 1 });
            Assert::AreEqual((void*)parse.GetContext().GetRootNode(), (void*)subject.GetRoot().get());
            AssertRange(parse.GetContext().GetNodes()[0].GetRange(), parse.GetContext().GetRange());
            Assert::AreEqual((void*)parse.GetContext().GetNodes()[0].GetNode(), (void*)redirectNode.GetCommandNode().get());
            Assert::AreEqual(parse.GetContext().GetSource(), source);

            auto parent = parse.GetContext().GetChild();
            Assert::IsNotNull(parent);
            Assert::AreEqual(parent->GetRange().Get(input), { "actual" });
            Assert::AreEqual(parent->GetNodes().size(), { 1 });
            AssertRange(parent->GetNodes()[0].GetRange(), parent->GetRange());
            Assert::AreEqual((void*)parent->GetNodes()[0].GetNode(), (void*)concreteNode.GetNode().get());
            Assert::AreEqual(parent->GetSource(), source);

            Assert::AreEqual(subject.Execute(parse), 2);
        }

        TEST_METHOD(testExecuteOrphanedSubcommand) {
            CommandDispatcher<int> subject;
            auto foo = subject.Register("foo").Executes(command).Then<Argument, Integer>("bar");

            try {
                subject.Execute("foo 5", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 5);
            }
        }

        TEST_METHOD(testExecute_invalidOther) {
            CommandDispatcher<int> subject;
            subject.Register("w").Executes(wrongcommand);
            subject.Register("world").Executes(command);

            Assert::AreEqual(subject.Execute("world", source), 42);
        }

        TEST_METHOD(parse_noSpaceSeparator) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Then<Argument, Integer>("bar").Executes(command);

            try {
                subject.Execute("foo$", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 0);
            }
        }

        TEST_METHOD(testExecuteInvalidSubcommand) {
            CommandDispatcher<int> subject;
            subject.Register("foo").Then<Argument, Integer>("bar").Executes(command);

            try {
                subject.Execute("foo bar", source);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), 4);
            }
        }

        TEST_METHOD(testGetPath) {
            CommandDispatcher<int> subject;
            auto bar = MakeLiteral<int>("bar");
            subject.Register("foo").Then(bar);

            AssertArray(subject.GetPath(bar.GetCommandNode().get()), { "foo", "bar" });
        }

        TEST_METHOD(testFindNodeExists) {
            CommandDispatcher<int> subject;
            auto bar = MakeLiteral<int>("bar");
            subject.Register("foo").Then(bar);

            Assert::AreEqual((void*)subject.FindNode({ "foo", "bar" }), (void*)bar.GetCommandNode().get());
        }

        TEST_METHOD(testFindNodeDoesntExist) {
            CommandDispatcher<int> subject;
            Assert::IsNull(subject.FindNode({ "foo", "bar" }));
        }
    };

    TEST_CLASS(CommandDispatcherUsagesTest)
    {
        CommandDispatcher<int> subject;

        TEST_METHOD_INITIALIZE(init)
        {
            auto a = subject.Register("a");

            auto a1 = a.Then<Literal>("1");
            a1.Then<Literal>("i").Executes(command);
            a1.Then<Literal>("ii").Executes(command);

            auto a2 = a.Then<Literal>("2");
            a2.Then<Literal>("i").Executes(command);
            a2.Then<Literal>("ii").Executes(command);

            subject.Register("b").Then<Literal>("1").Executes(command);

            subject.Register("c").Executes(command);

            subject.Register("d").Requires([](int&) -> bool { return false; }).Executes(command);

            auto& e1 = subject.Register("e").Executes(command).Then<Literal>("1").Executes(command);
            e1.Then<Literal>("i").Executes(command);
            e1.Then<Literal>("ii").Executes(command);

            auto f = subject.Register("f");
            auto f1 = f.Then<Literal>("1");
            f1.Then<Literal>("i").Executes(command);
            f1.Then<Literal>("ii").Executes(command).Requires([](int&) -> bool { return false; });
            auto f2 = f.Then<Literal>("2");
            f2.Then<Literal>("i").Executes(command).Requires([](int&) -> bool { return false; });
            f2.Then<Literal>("ii").Executes(command);

            subject.Register("g").Executes(command).Then<Literal>("1").Then<Literal>("i").Executes(command);

            auto& h = subject.Register("h").Executes(command);
            h.Then<Literal>("1").Then<Literal>("i").Executes(command);
            h.Then<Literal>("2").Then<Literal>("i").Then<Literal>("ii").Executes(command);
            h.Then<Literal>("3").Executes(command);

            auto& i = subject.Register("i").Executes(command);
            i.Then<Literal>("1").Executes(command);
            i.Then<Literal>("2").Executes(command);

            subject.Register("j").Redirect(subject.GetRoot());
            subject.Register("k").Redirect(h);
        }

        TEST_METHOD_CLEANUP(cleanup)
        {
            subject = {};
        }

        CommandNode<int>* get(std::string_view command) {
            return subject.Parse(command, source).GetContext().GetNodes().back().GetNode();
        }

        CommandNode<int>* get(StringReader command) {
            return subject.Parse(command, source).GetContext().GetNodes().back().GetNode();
        }



        TEST_METHOD(testAllUsage_noCommands) {
            subject = CommandDispatcher<int>();
            auto results = subject.GetAllUsage(subject.GetRoot().get(), source, true);
            Assert::IsTrue(results.empty());
        }


        TEST_METHOD(testSmartUsage_noCommands) {
            subject = CommandDispatcher<int>();
            auto results = subject.GetSmartUsage(subject.GetRoot().get(), source);
            Assert::IsTrue(results.empty());
        }


        TEST_METHOD(testAllUsage_root) {
            auto results = subject.GetAllUsage(subject.GetRoot().get(), source, true);
            AssertArray(results, {
                "a 1 i",
                "a 1 ii",
                "a 2 i",
                "a 2 ii",
                "b 1",
                "c",
                "e",
                "e 1",
                "e 1 i",
                "e 1 ii",
                "f 1 i",
                "f 2 ii",
                "g",
                "g 1 i",
                "h",
                "h 1 i",
                "h 2 i ii",
                "h 3",
                "i",
                "i 1",
                "i 2",
                "j ...",
                "k -> h",
                });
        }
        

        TEST_METHOD(testSmartUsage_root) {
            auto results = subject.GetSmartUsage(subject.GetRoot().get(), source);
            AssertMap(results, {
                {get("a"), "a (1|2)"},
                {get("b"), "b 1"},
                {get("c"), "c"},
                {get("e"), "e [1]"},
                {get("f"), "f (1|2)"},
                {get("g"), "g [1]"},
                {get("h"), "h [1|2|3]"},
                {get("i"), "i [1|2]"},
                {get("j"), "j ..."},
                {get("k"), "k -> h"}
                });
        }


        TEST_METHOD(testSmartUsage_h) {
            auto results = subject.GetSmartUsage(get("h"), source);
            AssertMap(results, {
                {get("h 1"), "[1] i"},
                {get("h 2"), "[2] i ii"},
                {get("h 3"), "[3]"}
                });
        }


        TEST_METHOD(testSmartUsage_offsetH) {
            StringReader offsetH("/|/|/h");
            offsetH.SetCursor(5);

            auto results = subject.GetSmartUsage(get(offsetH), source);
            AssertMap(results, {
                {get("h 1"), "[1] i"},
                {get("h 2"), "[2] i ii"},
                {get("h 3"), "[3]"}
                });
        }
    };

    TEST_CLASS(CommandDispatcherSuggestionsTest)
    {
    private:
        void testSuggestions(CommandDispatcher<int>& subject, std::string_view contents, int cursor, StringRange range, std::vector<std::string> suggestions = {}) {
            auto result = subject.GetCompletionSuggestions(subject.Parse(contents, source), cursor).get();
            AssertRange(result.GetRange(), range);

            std::set<Suggestion, CompareNoCase> expected;
            for (auto& suggestion : suggestions) {
                expected.emplace(range, suggestion);
            }

            AssertSet(result.GetList(), expected);
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands) {
            CommandDispatcher<int> subject;
            subject.Register<Literal>("foo");
            subject.Register<Literal>("bar");
            subject.Register<Literal>("baz");

            Suggestions result = subject.GetCompletionSuggestions(subject.Parse("", source)).get();

            AssertRange(result.GetRange(), StringRange::At(0));
            AssertSet(result.GetList(), { Suggestion(StringRange::At(0), "bar"), Suggestion(StringRange::At(0), "baz"), Suggestion(StringRange::At(0), "foo") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_withInputOffset) {
            CommandDispatcher<int> subject;
            subject.Register<Literal>("foo");
            subject.Register<Literal>("bar");
            subject.Register<Literal>("baz");

            Suggestions result = subject.GetCompletionSuggestions(subject.Parse(InputWithOffset("OOO", 3), source)).get();

            AssertRange(result.GetRange(), StringRange::At(3));
            AssertSet(result.GetList(), { Suggestion(StringRange::At(3), "bar"), Suggestion(StringRange::At(3), "baz"), Suggestion(StringRange::At(3), "foo") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_partial) {
            CommandDispatcher<int> subject;
            subject.Register<Literal>("foo");
            subject.Register<Literal>("bar");
            subject.Register<Literal>("baz");

            Suggestions result = subject.GetCompletionSuggestions(subject.Parse("b", source)).get();

            AssertRange(result.GetRange(), StringRange::Between(0, 1));
            AssertSet(result.GetList(), { Suggestion(StringRange::Between(0, 1), "bar"), Suggestion(StringRange::Between(0, 1), "baz") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_partial_withInputOffset) {
            CommandDispatcher<int> subject;
            subject.Register<Literal>("foo");
            subject.Register<Literal>("bar");
            subject.Register<Literal>("baz");

            Suggestions result = subject.GetCompletionSuggestions(subject.Parse(InputWithOffset("Zb", 1), source)).get();

            AssertRange(result.GetRange(), StringRange::Between(1, 2));
            AssertSet(result.GetList(), { Suggestion(StringRange::Between(1, 2), "bar"), Suggestion(StringRange::Between(1, 2), "baz") });
        }


        TEST_METHOD(getCompletionSuggestions_subCommands) {
            CommandDispatcher<int> subject;
            auto parent = subject.Register("parent");
            parent.Then<Literal>("foo");
            parent.Then<Literal>("bar");
            parent.Then<Literal>("baz");

            Suggestions result = subject.GetCompletionSuggestions(subject.Parse("parent ", source)).get();

            AssertRange(result.GetRange(), StringRange::At(7));
            AssertSet(result.GetList(), { Suggestion(StringRange::At(7), "bar"), Suggestion(StringRange::At(7), "baz"), Suggestion(StringRange::At(7), "foo") });
        }


        TEST_METHOD(getCompletionSuggestions_movingCursor_subCommands) {
            CommandDispatcher<int> subject;
            auto p1 = subject.Register("parent_one");
            p1.Then<Literal>("faz");
            p1.Then<Literal>("fbz");
            p1.Then<Literal>("gaz");

            subject.Register("parent_two");

            testSuggestions(subject, "parent_one faz ", 0, StringRange::At(0), { "parent_one", "parent_two" });
            testSuggestions(subject, "parent_one faz ", 1, StringRange::Between(0, 1), { "parent_one", "parent_two" });
            testSuggestions(subject, "parent_one faz ", 7, StringRange::Between(0, 7), { "parent_one", "parent_two" });
            testSuggestions(subject, "parent_one faz ", 8, StringRange::Between(0, 8), { "parent_one" });
            testSuggestions(subject, "parent_one faz ", 10, StringRange::At(0), {});
            testSuggestions(subject, "parent_one faz ", 11, StringRange::At(11), { "faz", "fbz", "gaz" });
            testSuggestions(subject, "parent_one faz ", 12, StringRange::Between(11, 12), { "faz", "fbz" });
            testSuggestions(subject, "parent_one faz ", 13, StringRange::Between(11, 13), { "faz" });
            testSuggestions(subject, "parent_one faz ", 14, StringRange::At(0), {});
            testSuggestions(subject, "parent_one faz ", 15, StringRange::At(0), {});
        }
        

        TEST_METHOD(getCompletionSuggestions_subCommands_partial) {
            CommandDispatcher<int> subject;
            auto parent = subject.Register("parent");
            parent.Then<Literal>("foo");
            parent.Then<Literal>("bar");
            parent.Then<Literal>("baz");

            auto parse = subject.Parse("parent b", source);
            Suggestions result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(7, 8));
            AssertSet(result.GetList(), { Suggestion(StringRange::Between(7, 8), "bar"), Suggestion(StringRange::Between(7, 8), "baz") });
        }


        TEST_METHOD(getCompletionSuggestions_subCommands_partial_withInputOffset) {
            CommandDispatcher<int> subject;
            auto parent = subject.Register("parent");
            parent.Then<Literal>("foo");
            parent.Then<Literal>("bar");
            parent.Then<Literal>("baz");
            

            auto parse = subject.Parse(InputWithOffset("junk parent b", 5), source);
            Suggestions result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(12, 13));
            AssertSet(result.GetList(), { Suggestion(StringRange::Between(12, 13), "bar"), Suggestion(StringRange::Between(12, 13), "baz") });
        }


        TEST_METHOD(getCompletionSuggestions_redirect) {
            CommandDispatcher<int> subject;
            auto actual = subject.Register<Literal>("actual");
            actual.Then<Literal>("sub");
            subject.Register<Literal>("redirect").Redirect(actual);

            auto parse = subject.Parse("redirect ", source);
            Suggestions result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::At(9));
            AssertSet(result.GetList(), { Suggestion(StringRange::At(9), "sub") });
        }


        TEST_METHOD(getCompletionSuggestions_redirectPartial) {
            CommandDispatcher<int> subject;
            auto actual = subject.Register<Literal>("actual");
            actual.Then<Literal>("sub");
            subject.Register<Literal>("redirect").Redirect(actual);

            auto parse = subject.Parse("redirect s", source);
            Suggestions result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(9, 10));
            AssertSet(result.GetList(), { Suggestion(StringRange::Between(9, 10), "sub") });
        }


        TEST_METHOD(getCompletionSuggestions_movingCursor_redirect) {
            CommandDispatcher<int> subject;
            auto actualOne = subject.Register<Literal>("actual_one");
            actualOne.Then<Literal>("faz");
            actualOne.Then<Literal>("fbz");
            actualOne.Then<Literal>("gaz");

            auto actualTwo = subject.Register<Literal>("actual_two");

            subject.Register<Literal>("redirect_one").Redirect(actualOne);
            subject.Register<Literal>("redirect_two").Redirect(actualOne);

            testSuggestions(subject, "redirect_one faz ", 0, StringRange::At(0), { "actual_one", "actual_two", "redirect_one", "redirect_two" });
            testSuggestions(subject, "redirect_one faz ", 9, StringRange::Between(0, 9), { "redirect_one", "redirect_two" });
            testSuggestions(subject, "redirect_one faz ", 10, StringRange::Between(0, 10), { "redirect_one" });
            testSuggestions(subject, "redirect_one faz ", 12, StringRange::At(0));
            testSuggestions(subject, "redirect_one faz ", 13, StringRange::At(13), { "faz", "fbz", "gaz" });
            testSuggestions(subject, "redirect_one faz ", 14, StringRange::Between(13, 14), { "faz", "fbz" });
            testSuggestions(subject, "redirect_one faz ", 15, StringRange::Between(13, 15), { "faz" });
            testSuggestions(subject, "redirect_one faz ", 16, StringRange::At(0));
            testSuggestions(subject, "redirect_one faz ", 17, StringRange::At(0));
        }


        TEST_METHOD(getCompletionSuggestions_redirectPartial_withInputOffset) {
            CommandDispatcher<int> subject;
            auto actual = subject.Register<Literal>("actual");
            actual.Then<Literal>("sub");
            subject.Register<Literal>("redirect").Redirect(actual);

            auto parse = subject.Parse(InputWithOffset("/redirect s", 1), source);
            Suggestions result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(10, 11));
            AssertSet(result.GetList(), { Suggestion(StringRange::Between(10, 11), "sub") });
        }


        TEST_METHOD(getCompletionSuggestions_redirect_lots) {
            CommandDispatcher<int> subject;
            auto loop = subject.Register<Literal>("redirect");
            loop.Then<Literal>("loop").Then<Argument, Integer>("loop").Redirect(loop);

            Suggestions result = subject.GetCompletionSuggestions(subject.Parse("redirect loop 1 loop 02 loop 003 ", source)).get();

            AssertRange(result.GetRange(), StringRange::At(33));
            AssertSet(result.GetList(), { Suggestion(StringRange::At(33), "loop") });
        }


        TEST_METHOD(getCompletionSuggestions_execute_simulation) {
            CommandDispatcher<int> subject;
            auto execute = subject.Register<Literal>("execute");
            execute.Then<Literal>("as").Then<Argument, Word>("name").Redirect(execute);
            execute.Then<Literal>("store").Then<Argument, Word>("name").Redirect(execute);
            execute.Then<Literal>("run").Executes(command);

            auto parse = subject.Parse("execute as Dinnerbone as", source);
            Suggestions result = subject.GetCompletionSuggestions(parse).get();

            Assert::AreEqual(result.IsEmpty(), true);
        }


        TEST_METHOD(getCompletionSuggestions_execute_simulation_partial) {
            CommandDispatcher<int> subject;
            auto execute = subject.Register<Literal>("execute");
            auto as = execute.Then<Literal>("as");
            as.Then<Literal>("bar").Redirect(execute);
            as.Then<Literal>("baz").Redirect(execute);
            execute.Then<Literal>("store").Then<Argument, Word>("name").Redirect(execute);
            execute.Then<Literal>("run").Executes(command);

            auto parse = subject.Parse("execute as bar as ", source);
            Suggestions result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::At(18));
            AssertSet(result.GetList(), { Suggestion(StringRange::At(18), "bar"), Suggestion(StringRange::At(18), "baz") });
        }
    };

    // Benchmarks
    TEST_CLASS(ParseBenchmarks)
    {
        CommandDispatcher<int> subject;

        TEST_METHOD_INITIALIZE(init)
        {
            auto a = subject.Register("a");

            auto a1 = a.Then<Literal>("1");
            a1.Then<Literal>("i").Executes(command);
            a1.Then<Literal>("ii").Executes(command);

            auto a2 = a.Then<Literal>("2");
            a2.Then<Literal>("i").Executes(command);
            a2.Then<Literal>("ii").Executes(command);

            subject.Register("b").Then<Literal>("1").Executes(command);

            subject.Register("c").Executes(command);

            subject.Register("d").Requires([](int&) -> bool { return false; }).Executes(command);

            auto& e1 = subject.Register("e").Executes(command).Then<Literal>("1").Executes(command);
            e1.Then<Literal>("i").Executes(command);
            e1.Then<Literal>("ii").Executes(command);

            auto f = subject.Register("f");
            auto f1 = f.Then<Literal>("1");
            f1.Then<Literal>("i").Executes(command);
            f1.Then<Literal>("ii").Executes(command).Requires([](int&) -> bool { return false; });
            auto f2 = f.Then<Literal>("2");
            f2.Then<Literal>("i").Executes(command).Requires([](int&) -> bool { return false; });
            f2.Then<Literal>("ii").Executes(command);

            subject.Register("g").Executes(command).Then<Literal>("1").Then<Literal>("i").Executes(command);

            auto& h = subject.Register("h").Executes(command);
            h.Then<Literal>("1").Then<Literal>("i").Executes(command);
            h.Then<Literal>("2").Then<Literal>("i").Then<Literal>("ii").Executes(command);
            h.Then<Literal>("3").Executes(command);

            auto& i = subject.Register("i").Executes(command);
            i.Then<Literal>("1").Executes(command);
            i.Then<Literal>("2").Executes(command);

            subject.Register("j").Redirect(subject.GetRoot());
            subject.Register("k").Redirect(h);
        }

        TEST_METHOD_CLEANUP(cleanup)
        {
            subject = {};
        }

        
        TEST_METHOD(parse_a1i) {
            for (int i = 0; i < 1000000; i++)
                subject.Parse("a 1 i", source);
        }

        
        TEST_METHOD(parse_c) {
            for (int i = 0; i < 1000000; i++)
                subject.Parse("c", source);
        }

        
        TEST_METHOD(parse_k1i) {
            for (int i = 0; i < 1000000; i++)
                subject.Parse("k 1 i", source);
        }
    };

    TEST_CLASS(ExecuteBenchmarks)
    {
        CommandDispatcher<int> dispatcher;
        std::optional<ParseResults<int>> simple;
        std::optional<ParseResults<int>> singleRedirect;
        std::optional<ParseResults<int>> forkedRedirect;

        TEST_METHOD_INITIALIZE(setup)
        {
            dispatcher.Register("command").Executes(command);
            dispatcher.Register("redirect").Redirect(dispatcher.GetRoot());
            dispatcher.Register("fork").Fork(dispatcher.GetRoot(), [](CommandContext<int>& ctx) -> std::vector<int> { return { 1, 2, 3 }; });
            simple = dispatcher.Parse("command", source);
            singleRedirect = dispatcher.Parse("redirect command", source);
            forkedRedirect = dispatcher.Parse("fork command", source);
        }

        TEST_METHOD_CLEANUP(cleanup)
        {
            dispatcher = {};
            simple = {};
            singleRedirect = {};
            forkedRedirect = {};
        }

        TEST_METHOD(execute_simple) {
            for (int i = 0; i < 1000000; i++)
                dispatcher.Execute(*simple);
        }

        TEST_METHOD(execute_single_redirect) {
            for (int i = 0; i < 1000000; i++)
                dispatcher.Execute(*singleRedirect);
        }

        TEST_METHOD(execute_forked_redirect) {
            for (int i = 0; i < 1000000; i++)
                dispatcher.Execute(*forkedRedirect);
        }
    };
}
