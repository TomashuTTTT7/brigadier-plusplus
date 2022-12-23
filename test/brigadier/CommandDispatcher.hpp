#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    CommandW<int> command = [](CommandContextW<int>& ctx) -> int { return 42; };
    CommandW<int> subcommand = [](CommandContextW<int>& ctx) -> int { return 100; };
    CommandW<int> wrongcommand = [](CommandContextW<int>& ctx) -> int { Assert::Fail(); return 0; };
    int source = 0;

    TEST_CLASS(CommandDispatcherTest)
    {
        TEST_METHOD(testCreateAndExecuteCommand) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Executes(command);

            Assert::AreEqual(subject.Execute(L"foo", source), 42);
        }

        TEST_METHOD(testCreateAndExecuteOffsetCommand) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Executes(command);

            Assert::AreEqual(subject.Execute(InputWithOffset(L"/foo", 1), source), 42);
        }

        TEST_METHOD(testCreateAndMergeCommands) {
            CommandDispatcherW<int> subject;
            subject.Register(L"base").Then<Literal>(L"foo").Executes(command);
            subject.Register(L"base").Then<Literal>(L"bar").Executes(command);

            Assert::AreEqual(subject.Execute(L"base foo", source), 42);
            Assert::AreEqual(subject.Execute(L"base bar", source), 42);
        }

        TEST_METHOD(testExecuteUnknownCommand) {
            CommandDispatcherW<int> subject;
            subject.Register(L"bar");
            subject.Register(L"baz");

            try {
                subject.Execute(L"foo", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
        }

        TEST_METHOD(testExecuteImpermissibleCommand) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Requires([](int& src) { return false; });

            try {
                subject.Execute(L"foo", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
        }

        TEST_METHOD(testExecuteEmptyCommand) {
            CommandDispatcherW<int> subject;
            subject.Register(L"");

            try {
                subject.Execute(L"", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
        }

        TEST_METHOD(testExecuteUnknownSubcommand) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Executes(command);

            try {
                subject.Execute(L"foo bar", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 4);
            }
        }

        TEST_METHOD(testExecuteIncorrectLiteral) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Executes(command).Then<Literal>(L"bar");

            try {
                subject.Execute(L"foo baz", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 4);
            }
        }

        TEST_METHOD(testExecuteAmbiguousIncorrectArgument) {
            CommandDispatcherW<int> subject;
            auto foo = subject.Register(L"foo");
            foo.Executes(command);
            foo.Then<Literal>(L"bar");
            foo.Then<Literal>(L"baz");

            try {
                subject.Execute(L"foo unknown", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 4);
            }
        }

        TEST_METHOD(testExecuteSubcommand) {
            CommandDispatcherW<int> subject;
            auto foo = subject.Register(L"foo");
            foo.Then<Literal>(L"a");
            foo.Then<Literal>(L"=").Executes(subcommand);
            foo.Then<Literal>(L"c");
            foo.Executes(command);

            Assert::AreEqual<size_t>(subject.Execute(L"foo =", source), 100);
        }

        TEST_METHOD(testParseIncompleteLiteral) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Then<Literal>(L"bar").Executes(command);

            auto Parse = subject.Parse(L"foo ", source);
            Assert::AreEqual(Parse.GetReader().GetRemaining(), { L" " });
            Assert::AreEqual(Parse.GetContext().GetNodes().size(), { 1 });
        }

        TEST_METHOD(testParseIncompleteArgument) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Then<Argument, Integer>(L"bar").Executes(command);

            auto Parse = subject.Parse(L"foo ", source);
            Assert::AreEqual(Parse.GetReader().GetRemaining(), { L" " });
            Assert::AreEqual(Parse.GetContext().GetNodes().size(), { 1 });
        }

        TEST_METHOD(testExecuteAmbiguiousParentSubcommand) {
            CommandDispatcherW<int> subject;

            auto test = subject.Register(L"test");
            test.Then<Argument, Integer>(L"incorrect").Executes(command);
            test.Then<Argument, Integer>(L"right").Then<Argument, Integer>(L"sub").Executes(subcommand);

            Assert::AreEqual(subject.Execute(L"test 1 2", source), 100);
        }

        TEST_METHOD(testExecuteAmbiguiousParentSubcommandViaRedirect) {
            CommandDispatcherW<int> subject;

            auto real = subject.Register(L"test");
            real.Then<Argument, Integer>(L"incorrect").Executes(command);
            real.Then<Argument, Integer>(L"right").Then<Argument, Integer>(L"sub").Executes(subcommand);

            subject.Register(L"redirect").Redirect(real);

            Assert::AreEqual(subject.Execute(L"redirect 1 2", source), 100);
        }

        TEST_METHOD(testExecuteRedirectedMultipleTimes) {
            CommandDispatcherW<int> subject;
            auto concreteNode = subject.Register(L"actual");
            concreteNode.Executes(command);
            auto redirectNode = subject.Register(L"redirected");
            redirectNode.Redirect(subject.GetRoot());

            std::wstring_view input = L"redirected redirected actual";

            auto parse = subject.Parse(input, source);
            Assert::AreEqual(parse.GetContext().GetRange().Get(input), { L"redirected" });
            Assert::AreEqual(parse.GetContext().GetNodes().size(), { 1 });
            Assert::AreEqual((void*)parse.GetContext().GetRootNode(), { (void*)subject.GetRoot().get() });
            AssertRange(parse.GetContext().GetNodes()[0].GetRange(), parse.GetContext().GetRange());
            Assert::AreEqual((void*)parse.GetContext().GetNodes()[0].GetNode(), (void*)redirectNode.GetCommandNode().get());

            auto child1 = parse.GetContext().GetChild();
            Assert::IsNotNull(child1);
            Assert::AreEqual(child1->GetRange().Get(input), { L"redirected" });
            Assert::AreEqual(child1->GetNodes().size(), { 1 });
            Assert::AreEqual((void*)child1->GetRootNode(), { (void*)subject.GetRoot().get() });
            AssertRange(child1->GetNodes()[0].GetRange(), child1->GetRange());
            Assert::AreEqual((void*)child1->GetNodes()[0].GetNode(), (void*)redirectNode.GetCommandNode().get());

            auto child2 = child1->GetChild();
            Assert::IsNotNull(child2);
            Assert::AreEqual(child2->GetRange().Get(input), { L"actual" });
            Assert::AreEqual(child2->GetNodes().size(), { 1 });
            Assert::AreEqual((void*)child2->GetRootNode(), { (void*)subject.GetRoot().get() });
            AssertRange(child2->GetNodes()[0].GetRange(), child2->GetRange());
            Assert::AreEqual((void*)child2->GetNodes()[0].GetNode(), (void*)concreteNode.GetCommandNode().get());

            Assert::AreEqual(subject.Execute(parse), 42);
        }

        TEST_METHOD(testExecuteRedirected) {
            CommandDispatcherW<int> subject;

            auto concreteNode = subject.Register(L"actual");
            concreteNode.Executes(command);
            auto redirectNode = subject.Register(L"redirected");
            redirectNode.Fork(subject.GetRoot(), [](CommandContextW<int>& context) -> std::vector<int> { return { 1, 2 }; });

            std::wstring_view input = L"redirected actual";
            auto parse = subject.Parse(input, source);
            Assert::AreEqual(parse.GetContext().GetRange().Get(input), { L"redirected" });
            Assert::AreEqual(parse.GetContext().GetNodes().size(), { 1 });
            Assert::AreEqual((void*)parse.GetContext().GetRootNode(), (void*)subject.GetRoot().get());
            AssertRange(parse.GetContext().GetNodes()[0].GetRange(), parse.GetContext().GetRange());
            Assert::AreEqual((void*)parse.GetContext().GetNodes()[0].GetNode(), (void*)redirectNode.GetCommandNode().get());
            Assert::AreEqual(parse.GetContext().GetSource(), source);

            auto parent = parse.GetContext().GetChild();
            Assert::IsNotNull(parent);
            Assert::AreEqual(parent->GetRange().Get(input), { L"actual" });
            Assert::AreEqual(parent->GetNodes().size(), { 1 });
            AssertRange(parent->GetNodes()[0].GetRange(), parent->GetRange());
            Assert::AreEqual((void*)parent->GetNodes()[0].GetNode(), (void*)concreteNode.GetNode().get());
            Assert::AreEqual(parent->GetSource(), source);

            Assert::AreEqual(subject.Execute(parse), 2);
        }

        TEST_METHOD(testExecuteOrphanedSubcommand) {
            CommandDispatcherW<int> subject;
            auto foo = subject.Register(L"foo").Executes(command).Then<Argument, Integer>(L"bar");

            try {
                subject.Execute(L"foo 5", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 5);
            }
        }

        TEST_METHOD(testExecute_invalidOther) {
            CommandDispatcherW<int> subject;
            subject.Register(L"w").Executes(wrongcommand);
            subject.Register(L"world").Executes(command);

            Assert::AreEqual<size_t>(subject.Execute(L"world", source), 42);
        }

        TEST_METHOD(parse_noSpaceSeparator) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Then<Argument, Integer>(L"bar").Executes(command);

            try {
                subject.Execute(L"foo$", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
        }

        TEST_METHOD(testExecuteInvalidSubcommand) {
            CommandDispatcherW<int> subject;
            subject.Register(L"foo").Then<Argument, Integer>(L"bar").Executes(command);

            try {
                subject.Execute(L"foo bar", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 4);
            }
        }

        //TEST_METHOD(testGetPath) {
        //    CommandDispatcherW<int> subject;
        //    auto bar = MakeLiteral<wchar_t, int>(L"bar");
        //    subject.Register(L"foo").Then(bar);
        //
        //    AssertArray(subject.GetPath(bar.GetCommandNode().get()), { L"foo", L"bar" });
        //}

        //TEST_METHOD(testFindNodeExists) {
        //    CommandDispatcherW<int> subject;
        //    auto bar = MakeLiteral<wchar_t, int>(L"bar");
        //    subject.Register(L"foo").Then(bar);
        //
        //    Assert::AreEqual((void*)subject.FindNode({ L"foo", L"bar" }), (void*)bar.GetCommandNode().get());
        //}

        TEST_METHOD(testFindNodeDoesntExist) {
            CommandDispatcherW<int> subject;
            Assert::IsNull(subject.FindNode({ L"foo", L"bar" }));
        }
    };

    TEST_CLASS(CommandDispatcherUsagesTest)
    {
        CommandDispatcherW<int> subject;

        TEST_METHOD_INITIALIZE(init)
        {
            auto a = subject.Register(L"a");

            auto a1 = a.Then<Literal>(L"1");
            a1.Then<Literal>(L"i").Executes(command);
            a1.Then<Literal>(L"ii").Executes(command);

            auto a2 = a.Then<Literal>(L"2");
            a2.Then<Literal>(L"i").Executes(command);
            a2.Then<Literal>(L"ii").Executes(command);

            subject.Register(L"b").Then<Literal>(L"1").Executes(command);

            subject.Register(L"c").Executes(command);

            subject.Register(L"d").Requires([](int&) -> bool { return false; }).Executes(command);

            auto& e1 = subject.Register(L"e").Executes(command).Then<Literal>(L"1").Executes(command);
            e1.Then<Literal>(L"i").Executes(command);
            e1.Then<Literal>(L"ii").Executes(command);

            auto f = subject.Register(L"f");
            auto f1 = f.Then<Literal>(L"1");
            f1.Then<Literal>(L"i").Executes(command);
            f1.Then<Literal>(L"ii").Executes(command).Requires([](int&) -> bool { return false; });
            auto f2 = f.Then<Literal>(L"2");
            f2.Then<Literal>(L"i").Executes(command).Requires([](int&) -> bool { return false; });
            f2.Then<Literal>(L"ii").Executes(command);

            subject.Register(L"g").Executes(command).Then<Literal>(L"1").Then<Literal>(L"i").Executes(command);

            auto& h = subject.Register(L"h").Executes(command);
            h.Then<Literal>(L"1").Then<Literal>(L"i").Executes(command);
            h.Then<Literal>(L"2").Then<Literal>(L"i").Then<Literal>(L"ii").Executes(command);
            h.Then<Literal>(L"3").Executes(command);

            auto& i = subject.Register(L"i").Executes(command);
            i.Then<Literal>(L"1").Executes(command);
            i.Then<Literal>(L"2").Executes(command);

            subject.Register(L"j").Redirect(subject.GetRoot());
            subject.Register(L"k").Redirect(h);
        }

        TEST_METHOD_CLEANUP(cleanup)
        {
            subject = {};
        }

        CommandNodeW<int>* get(std::wstring_view command) {
            return subject.Parse(command, source).GetContext().GetNodes().back().GetNode();
        }

        CommandNodeW<int>* get(StringReaderW command) {
            return subject.Parse(command, source).GetContext().GetNodes().back().GetNode();
        }



        TEST_METHOD(testAllUsage_noCommands) {
            subject = CommandDispatcherW<int>();
            auto results = subject.GetAllUsage(subject.GetRoot().get(), source, true);
            Assert::IsTrue(results.empty());
        }


        TEST_METHOD(testSmartUsage_noCommands) {
            subject = CommandDispatcherW<int>();
            auto results = subject.GetSmartUsage(subject.GetRoot().get(), source);
            Assert::IsTrue(results.empty());
        }


        TEST_METHOD(testAllUsage_root) {
            auto results = subject.GetAllUsage(subject.GetRoot().get(), source, true);
            AssertArray(results, {
                L"a 1 i",
                L"a 1 ii",
                L"a 2 i",
                L"a 2 ii",
                L"b 1",
                L"c",
                L"e",
                L"e 1",
                L"e 1 i",
                L"e 1 ii",
                L"f 1 i",
                L"f 2 ii",
                L"g",
                L"g 1 i",
                L"h",
                L"h 1 i",
                L"h 2 i ii",
                L"h 3",
                L"i",
                L"i 1",
                L"i 2",
                L"j ...",
                L"k -> h",
            });
        }
        

        TEST_METHOD(testSmartUsage_root) {
            auto results = subject.GetSmartUsage(subject.GetRoot().get(), source);
            AssertMap(results, {
                {get(L"a"), L"a (1|2)"},
                {get(L"b"), L"b 1"},
                {get(L"c"), L"c"},
                {get(L"e"), L"e [1]"},
                {get(L"f"), L"f (1|2)"},
                {get(L"g"), L"g [1]"},
                {get(L"h"), L"h [1|2|3]"},
                {get(L"i"), L"i [1|2]"},
                {get(L"j"), L"j ..."},
                {get(L"k"), L"k -> h"}
                });
        }


        TEST_METHOD(testSmartUsage_h) {
            auto results = subject.GetSmartUsage(get(L"h"), source);
            AssertMap(results, {
                {get(L"h 1"), L"[1] i"},
                {get(L"h 2"), L"[2] i ii"},
                {get(L"h 3"), L"[3]"}
                });
        }


        TEST_METHOD(testSmartUsage_offsetH) {
            StringReaderW offsetH(L"/|/|/h");
            offsetH.SetCursor(5);

            auto results = subject.GetSmartUsage(get(offsetH), source);
            AssertMap(results, {
                {get(L"h 1"), L"[1] i"},
                {get(L"h 2"), L"[2] i ii"},
                {get(L"h 3"), L"[3]"}
                });
        }
    };

    TEST_CLASS(CommandDispatcherSuggestionsTest)
    {
    private:
        void testSuggestions(CommandDispatcherW<int>& subject, std::wstring_view contents, size_t cursor, StringRange range, std::vector<std::wstring> suggestions = {}) {
            auto result = subject.GetCompletionSuggestions(subject.Parse(contents, source), cursor).get();
            AssertRange(result.GetRange(), range);

            std::set<SuggestionW, CompareNoCase<wchar_t>> expected;
            for (auto& suggestion : suggestions) {
                expected.emplace(range, suggestion);
            }

            AssertSet(result.GetList(), expected);
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands) {
            CommandDispatcherW<int> subject;
            subject.Register<Literal>(L"foo");
            subject.Register<Literal>(L"bar");
            subject.Register<Literal>(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(L"", source)).get();

            AssertRange(result.GetRange(), StringRange::At(0));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(0), L"bar"), SuggestionW(StringRange::At(0), L"baz"), SuggestionW(StringRange::At(0), L"foo") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_withInputOffset) {
            CommandDispatcherW<int> subject;
            subject.Register<Literal>(L"foo");
            subject.Register<Literal>(L"bar");
            subject.Register<Literal>(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(InputWithOffset(L"OOO", 3), source)).get();

            AssertRange(result.GetRange(), StringRange::At(3));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(3), L"bar"), SuggestionW(StringRange::At(3), L"baz"), SuggestionW(StringRange::At(3), L"foo") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_partial) {
            CommandDispatcherW<int> subject;
            subject.Register<Literal>(L"foo");
            subject.Register<Literal>(L"bar");
            subject.Register<Literal>(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(L"b", source)).get();

            AssertRange(result.GetRange(), StringRange::Between(0, 1));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(0, 1), L"bar"), SuggestionW(StringRange::Between(0, 1), L"baz") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_partial_withInputOffset) {
            CommandDispatcherW<int> subject;
            subject.Register<Literal>(L"foo");
            subject.Register<Literal>(L"bar");
            subject.Register<Literal>(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(InputWithOffset(L"Zb", 1), source)).get();

            AssertRange(result.GetRange(), StringRange::Between(1, 2));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(1, 2), L"bar"), SuggestionW(StringRange::Between(1, 2), L"baz") });
        }


        TEST_METHOD(getCompletionSuggestions_subCommands) {
            CommandDispatcherW<int> subject;
            auto parent = subject.Register(L"parent");
            parent.Then<Literal>(L"foo");
            parent.Then<Literal>(L"bar");
            parent.Then<Literal>(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(L"parent ", source)).get();

            AssertRange(result.GetRange(), StringRange::At(7));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(7), L"bar"), SuggestionW(StringRange::At(7), L"baz"), SuggestionW(StringRange::At(7), L"foo") });
        }


        TEST_METHOD(getCompletionSuggestions_movingCursor_subCommands) {
            CommandDispatcherW<int> subject;
            auto p1 = subject.Register(L"parent_one");
            p1.Then<Literal>(L"faz");
            p1.Then<Literal>(L"fbz");
            p1.Then<Literal>(L"gaz");

            subject.Register(L"parent_two");

            testSuggestions(subject, L"parent_one faz ", 0, StringRange::At(0), { L"parent_one", L"parent_two" });
            testSuggestions(subject, L"parent_one faz ", 1, StringRange::Between(0, 1), { L"parent_one", L"parent_two" });
            testSuggestions(subject, L"parent_one faz ", 7, StringRange::Between(0, 7), { L"parent_one", L"parent_two" });
            testSuggestions(subject, L"parent_one faz ", 8, StringRange::Between(0, 8), { L"parent_one" });
            testSuggestions(subject, L"parent_one faz ", 10, StringRange::At(0), {});
            testSuggestions(subject, L"parent_one faz ", 11, StringRange::At(11), { L"faz", L"fbz", L"gaz" });
            testSuggestions(subject, L"parent_one faz ", 12, StringRange::Between(11, 12), { L"faz", L"fbz" });
            testSuggestions(subject, L"parent_one faz ", 13, StringRange::Between(11, 13), { L"faz" });
            testSuggestions(subject, L"parent_one faz ", 14, StringRange::At(0), {});
            testSuggestions(subject, L"parent_one faz ", 15, StringRange::At(0), {});
        }
        

        TEST_METHOD(getCompletionSuggestions_subCommands_partial) {
            CommandDispatcherW<int> subject;
            auto parent = subject.Register(L"parent");
            parent.Then<Literal>(L"foo");
            parent.Then<Literal>(L"bar");
            parent.Then<Literal>(L"baz");

            auto parse = subject.Parse(L"parent b", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(7, 8));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(7, 8), L"bar"), SuggestionW(StringRange::Between(7, 8), L"baz") });
        }


        TEST_METHOD(getCompletionSuggestions_subCommands_partial_withInputOffset) {
            CommandDispatcherW<int> subject;
            auto parent = subject.Register(L"parent");
            parent.Then<Literal>(L"foo");
            parent.Then<Literal>(L"bar");
            parent.Then<Literal>(L"baz");
            

            auto parse = subject.Parse(InputWithOffset(L"junk parent b", 5), source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(12, 13));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(12, 13), L"bar"), SuggestionW(StringRange::Between(12, 13), L"baz") });
        }


        TEST_METHOD(getCompletionSuggestions_redirect) {
            CommandDispatcherW<int> subject;
            auto actual = subject.Register<Literal>(L"actual");
            actual.Then<Literal>(L"sub");
            subject.Register<Literal>(L"redirect").Redirect(actual);

            auto parse = subject.Parse(L"redirect ", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::At(9));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(9), L"sub") });
        }


        TEST_METHOD(getCompletionSuggestions_redirectPartial) {
            CommandDispatcherW<int> subject;
            auto actual = subject.Register<Literal>(L"actual");
            actual.Then<Literal>(L"sub");
            subject.Register<Literal>(L"redirect").Redirect(actual);

            auto parse = subject.Parse(L"redirect s", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(9, 10));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(9, 10), L"sub") });
        }


        TEST_METHOD(getCompletionSuggestions_movingCursor_redirect) {
            CommandDispatcherW<int> subject;
            auto actualOne = subject.Register<Literal>(L"actual_one");
            actualOne.Then<Literal>(L"faz");
            actualOne.Then<Literal>(L"fbz");
            actualOne.Then<Literal>(L"gaz");

            auto actualTwo = subject.Register<Literal>(L"actual_two");

            subject.Register<Literal>(L"redirect_one").Redirect(actualOne);
            subject.Register<Literal>(L"redirect_two").Redirect(actualOne);

            testSuggestions(subject, L"redirect_one faz ", 0, StringRange::At(0), { L"actual_one", L"actual_two", L"redirect_one", L"redirect_two" });
            testSuggestions(subject, L"redirect_one faz ", 9, StringRange::Between(0, 9), { L"redirect_one", L"redirect_two" });
            testSuggestions(subject, L"redirect_one faz ", 10, StringRange::Between(0, 10), { L"redirect_one" });
            testSuggestions(subject, L"redirect_one faz ", 12, StringRange::At(0));
            testSuggestions(subject, L"redirect_one faz ", 13, StringRange::At(13), { L"faz", L"fbz", L"gaz" });
            testSuggestions(subject, L"redirect_one faz ", 14, StringRange::Between(13, 14), { L"faz", L"fbz" });
            testSuggestions(subject, L"redirect_one faz ", 15, StringRange::Between(13, 15), { L"faz" });
            testSuggestions(subject, L"redirect_one faz ", 16, StringRange::At(0));
            testSuggestions(subject, L"redirect_one faz ", 17, StringRange::At(0));
        }


        TEST_METHOD(getCompletionSuggestions_redirectPartial_withInputOffset) {
            CommandDispatcherW<int> subject;
            auto actual = subject.Register<Literal>(L"actual");
            actual.Then<Literal>(L"sub");
            subject.Register<Literal>(L"redirect").Redirect(actual);

            auto parse = subject.Parse(InputWithOffset(L"/redirect s", 1), source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(10, 11));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(10, 11), L"sub") });
        }


        TEST_METHOD(getCompletionSuggestions_redirect_lots) {
            CommandDispatcherW<int> subject;
            auto loop = subject.Register<Literal>(L"redirect");
            loop.Then<Literal>(L"loop").Then<Argument, Integer>(L"loop").Redirect(loop);

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(L"redirect loop 1 loop 02 loop 003 ", source)).get();

            AssertRange(result.GetRange(), StringRange::At(33));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(33), L"loop") });
        }


        TEST_METHOD(getCompletionSuggestions_execute_simulation) {
            CommandDispatcherW<int> subject;
            auto execute = subject.Register<Literal>(L"execute");
            execute.Then<Literal>(L"as").Then<Argument, Word>(L"name").Redirect(execute);
            execute.Then<Literal>(L"store").Then<Argument, Word>(L"name").Redirect(execute);
            execute.Then<Literal>(L"run").Executes(command);

            auto parse = subject.Parse(L"execute as Dinnerbone as", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            Assert::AreEqual(result.IsEmpty(), true);
        }


        TEST_METHOD(getCompletionSuggestions_execute_simulation_partial) {
            CommandDispatcherW<int> subject;
            auto execute = subject.Register<Literal>(L"execute");
            auto as = execute.Then<Literal>(L"as");
            as.Then<Literal>(L"bar").Redirect(execute);
            as.Then<Literal>(L"baz").Redirect(execute);
            execute.Then<Literal>(L"store").Then<Argument, Word>(L"name").Redirect(execute);
            execute.Then<Literal>(L"run").Executes(command);

            auto parse = subject.Parse(L"execute as bar as ", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::At(18));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(18), L"bar"), SuggestionW(StringRange::At(18), L"baz") });
        }
    };

    // Benchmarks
    TEST_CLASS(ParseBenchmarks)
    {
        CommandDispatcherW<int> subject;

        TEST_METHOD_INITIALIZE(init)
        {
            auto a = subject.Register(L"a");

            auto a1 = a.Then<Literal>(L"1");
            a1.Then<Literal>(L"i").Executes(command);
            a1.Then<Literal>(L"ii").Executes(command);

            auto a2 = a.Then<Literal>(L"2");
            a2.Then<Literal>(L"i").Executes(command);
            a2.Then<Literal>(L"ii").Executes(command);

            subject.Register(L"b").Then<Literal>(L"1").Executes(command);

            subject.Register(L"c").Executes(command);

            subject.Register(L"d").Requires([](int&) -> bool { return false; }).Executes(command);

            auto& e1 = subject.Register(L"e").Executes(command).Then<Literal>(L"1").Executes(command);
            e1.Then<Literal>(L"i").Executes(command);
            e1.Then<Literal>(L"ii").Executes(command);

            auto f = subject.Register(L"f");
            auto f1 = f.Then<Literal>(L"1");
            f1.Then<Literal>(L"i").Executes(command);
            f1.Then<Literal>(L"ii").Executes(command).Requires([](int&) -> bool { return false; });
            auto f2 = f.Then<Literal>(L"2");
            f2.Then<Literal>(L"i").Executes(command).Requires([](int&) -> bool { return false; });
            f2.Then<Literal>(L"ii").Executes(command);

            subject.Register(L"g").Executes(command).Then<Literal>(L"1").Then<Literal>(L"i").Executes(command);

            auto& h = subject.Register(L"h").Executes(command);
            h.Then<Literal>(L"1").Then<Literal>(L"i").Executes(command);
            h.Then<Literal>(L"2").Then<Literal>(L"i").Then<Literal>(L"ii").Executes(command);
            h.Then<Literal>(L"3").Executes(command);

            auto& i = subject.Register(L"i").Executes(command);
            i.Then<Literal>(L"1").Executes(command);
            i.Then<Literal>(L"2").Executes(command);

            subject.Register(L"j").Redirect(subject.GetRoot());
            subject.Register(L"k").Redirect(h);
        }

        TEST_METHOD_CLEANUP(cleanup)
        {
            subject = {};
        }

        
        TEST_METHOD(parse_a1i) {
            for (int i = 0; i < 10000; i++)
                subject.Parse(L"a 1 i", source);
        }

        
        TEST_METHOD(parse_c) {
            for (int i = 0; i < 10000; i++)
                subject.Parse(L"c", source);
        }

        
        TEST_METHOD(parse_k1i) {
            for (int i = 0; i < 10000; i++)
                subject.Parse(L"k 1 i", source);
        }
    };

    TEST_CLASS(ExecuteBenchmarks)
    {
        CommandDispatcherW<int> dispatcher;
        std::optional<ParseResultsW<int>> simple;
        std::optional<ParseResultsW<int>> singleRedirect;
        std::optional<ParseResultsW<int>> forkedRedirect;

        TEST_METHOD_INITIALIZE(setup)
        {
            dispatcher.Register(L"command").Executes(command);
            dispatcher.Register(L"redirect").Redirect(dispatcher.GetRoot());
            dispatcher.Register(L"fork").Fork(dispatcher.GetRoot(), [](CommandContextW<int>& ctx) -> std::vector<int> { return { 1, 2, 3 }; });
            simple = dispatcher.Parse(L"command", source);
            singleRedirect = dispatcher.Parse(L"redirect command", source);
            forkedRedirect = dispatcher.Parse(L"fork command", source);
        }

        TEST_METHOD_CLEANUP(cleanup)
        {
            dispatcher = {};
            simple = {};
            singleRedirect = {};
            forkedRedirect = {};
        }

        TEST_METHOD(execute_simple) {
            for (int i = 0; i < 10000; i++)
                dispatcher.Execute(*simple);
        }

        TEST_METHOD(execute_single_redirect) {
            for (int i = 0; i < 10000; i++)
                dispatcher.Execute(*singleRedirect);
        }

        TEST_METHOD(execute_forked_redirect) {
            for (int i = 0; i < 10000; i++)
                dispatcher.Execute(*forkedRedirect);
        }
    };

    TEST_CLASS(SourceCopyTest)
    {
        static inline int copies = 0;
        static inline int moves  = 0;
        struct CopyGuard
        {
            __declspec(noinline) CopyGuard() {}
            __declspec(noinline) CopyGuard(CopyGuard const&) { ++copies; }
            //__declspec(noinline) CopyGuard(CopyGuard&&) { ++moves; }
        };

        TEST_METHOD_INITIALIZE(setup) { copies = 0; moves = 0; }
        TEST_METHOD_CLEANUP(cleanup)  { copies = 0; moves = 0; }

        TEST_METHOD(CopyMoveTest)
        {
            using Src = CopyGuard;

            CommandDispatcherW<Src> dispatcher;

            auto foo = dispatcher.Register<Literal>(L"foo");
            foo.Then<Argument, Integer>(L"bar").Executes([](CommandContextW<Src>& ctx) {
                printf("Bar is %d\n", ctx.GetArgument<Integer>(L"bar"));
                return 1;
            });
            foo.Executes([](CommandContextW<Src>& ctx) {
                puts("Called foo without arguments");
                return 1;
            });

            dispatcher.Execute(L"foo 1", {});

            Logger::WriteMessage((std::stringstream{} << copies << " copies, " << moves << " moves").str().c_str());
        }
    };
}
