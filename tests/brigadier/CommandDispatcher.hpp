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
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Executes(command);

            Assert::AreEqual(subject.Execute(L"foo", source), 42);
        }

        TEST_METHOD(testCreateAndExecuteOffsetCommand) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Executes(command);

            Assert::AreEqual(subject.Execute(InputWithOffset(L"/foo", 1), source), 42);
        }

        TEST_METHOD(testCreateAndMergeCommands) {
            RootCommandNodeW<int> subject;
            subject.Then(L"base").Then(L"foo").Executes(command);
            subject.Then(L"base").Then(L"bar").Executes(command);

            Assert::AreEqual(subject.Execute(L"base foo", source), 42);
            Assert::AreEqual(subject.Execute(L"base bar", source), 42);
        }

        TEST_METHOD(testExecuteUnknownCommand) {
            RootCommandNodeW<int> subject;
            subject.Then(L"bar");
            subject.Then(L"baz");

            try {
                subject.Execute(L"foo", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(testExecuteImpermissibleCommand) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Requires([](int& src) { return false; });

            try {
                subject.Execute(L"foo", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(testExecuteEmptyCommand) {
            RootCommandNodeW<int> subject;
            subject.Then(L"");

            try {
                subject.Execute(L"", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(testExecuteUnknownSubcommand) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Executes(command);

            try {
                subject.Execute(L"foo bar", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 4);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(testExecuteIncorrectLiteral) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Executes(command).Then(L"bar");

            try {
                subject.Execute(L"foo baz", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 4);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(testExecuteAmbiguousIncorrectArgument) {
            RootCommandNodeW<int> subject;
            auto& foo = subject.Then(L"foo");
            foo.Executes(command);
            foo.Then(L"bar");
            foo.Then(L"baz");

            try {
                subject.Execute(L"foo unknown", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 4);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(testExecuteSubcommand) {
            RootCommandNodeW<int> subject;
            auto& foo = subject.Then(L"foo");
            foo.Then(L"a");
            foo.Then(L"=").Executes(subcommand);
            foo.Then(L"c");
            foo.Executes(command);

            Assert::AreEqual<size_t>(subject.Execute(L"foo =", source), 100);
        }

        TEST_METHOD(testParseIncompleteLiteral) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Then(L"bar").Executes(command);

            auto Parse = subject.Parse(L"foo ", source);
            Assert::AreEqual(Parse.GetReader().GetRemaining(), { L" " });
            Assert::AreEqual(Parse.GetContext().GetNodes().size(), { 1 });
        }

        TEST_METHOD(testParseIncompleteArgument) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Then<Integer>(L"bar").Executes(command);

            auto Parse = subject.Parse(L"foo ", source);
            Assert::AreEqual(Parse.GetReader().GetRemaining(), { L" " });
            Assert::AreEqual(Parse.GetContext().GetNodes().size(), { 1 });
        }

        TEST_METHOD(testExecuteAmbiguiousParentSubcommand) {
            RootCommandNodeW<int> subject;

            auto& test = subject.Then(L"test");
            test.Then<Integer>(L"incorrect").Executes(command);
            test.Then<Integer>(L"right").Then<Integer>(L"sub").Executes(subcommand);

            Assert::AreEqual(subject.Execute(L"test 1 2", source), 100);
        }

        TEST_METHOD(testExecuteAmbiguiousParentSubcommandViaRedirect) {
            RootCommandNodeW<int> subject;

            auto& real = subject.Then(L"test");
            real.Then<Integer>(L"incorrect").Executes(command);
            real.Then<Integer>(L"right").Then<Integer>(L"sub").Executes(subcommand);

            subject.Then(L"redirect").Redirect(real);

            Assert::AreEqual(subject.Execute(L"redirect 1 2", source), 100);
        }

        TEST_METHOD(testExecuteRedirectedMultipleTimes) {
            RootCommandNodeW<int> subject;
            auto& concreteNode = subject.Then(L"actual");
            concreteNode.Executes(command);
            auto& redirectNode = subject.Then(L"redirected");
            redirectNode.Redirect(subject);

            std::wstring_view input = L"redirected redirected actual";

            auto parse = subject.Parse(input, source);
            Assert::AreEqual(parse.GetContext().GetRange().Get(input), { L"redirected" });
            Assert::AreEqual(parse.GetContext().GetNodes().size(), { 1 });
            Assert::AreEqual((void*)parse.GetContext().GetRootNode(), { (void*)&subject });
            AssertRange(parse.GetContext().GetNodes()[0].GetRange(), parse.GetContext().GetRange());
            Assert::AreEqual((void*)parse.GetContext().GetNodes()[0].GetNode(), (void*)&redirectNode);

            auto child1 = parse.GetContext().GetChild();
            Assert::IsNotNull(child1);
            Assert::AreEqual(child1->GetRange().Get(input), { L"redirected" });
            Assert::AreEqual(child1->GetNodes().size(), { 1 });
            Assert::AreEqual((void*)child1->GetRootNode(), { (void*)&subject });
            AssertRange(child1->GetNodes()[0].GetRange(), child1->GetRange());
            Assert::AreEqual((void*)child1->GetNodes()[0].GetNode(), (void*)&redirectNode);

            auto child2 = child1->GetChild();
            Assert::IsNotNull(child2);
            Assert::AreEqual(child2->GetRange().Get(input), { L"actual" });
            Assert::AreEqual(child2->GetNodes().size(), { 1 });
            Assert::AreEqual((void*)child2->GetRootNode(), { (void*)&subject });
            AssertRange(child2->GetNodes()[0].GetRange(), child2->GetRange());
            Assert::AreEqual((void*)child2->GetNodes()[0].GetNode(), (void*)&concreteNode);

            Assert::AreEqual(subject.Execute(parse), 42);
        }

        TEST_METHOD(testExecuteRedirected) {
            RootCommandNodeW<int> subject;

            auto& concreteNode = subject.Then(L"actual");
            concreteNode.Executes(command);
            auto& redirectNode = subject.Then(L"redirected");
            redirectNode.Fork(subject, [](CommandContextW<int>& context) -> std::vector<int> { return { 1, 2 }; });

            std::wstring_view input = L"redirected actual";
            auto parse = subject.Parse(input, source);
            Assert::AreEqual(parse.GetContext().GetRange().Get(input), { L"redirected" });
            Assert::AreEqual(parse.GetContext().GetNodes().size(), { 1 });
            Assert::AreEqual((void*)parse.GetContext().GetRootNode(), (void*)&subject);
            AssertRange(parse.GetContext().GetNodes()[0].GetRange(), parse.GetContext().GetRange());
            Assert::AreEqual((void*)parse.GetContext().GetNodes()[0].GetNode(), (void*)&redirectNode);
            Assert::AreEqual(parse.GetContext().GetSource(), source);

            auto parent = parse.GetContext().GetChild();
            Assert::IsNotNull(parent);
            Assert::AreEqual(parent->GetRange().Get(input), { L"actual" });
            Assert::AreEqual(parent->GetNodes().size(), { 1 });
            AssertRange(parent->GetNodes()[0].GetRange(), parent->GetRange());
            Assert::AreEqual((void*)parent->GetNodes()[0].GetNode(), (void*)&concreteNode);
            Assert::AreEqual(parent->GetSource(), source);

            Assert::AreEqual(subject.Execute(parse), 2);
        }

        TEST_METHOD(testExecuteOrphanedSubcommand) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Executes(command).Then<Integer>(L"bar");

            try {
                subject.Execute(L"foo 5", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 5);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(testExecute_invalidOther) {
            RootCommandNodeW<int> subject;
            subject.Then(L"w").Executes(wrongcommand);
            subject.Then(L"world").Executes(command);

            Assert::AreEqual<size_t>(subject.Execute(L"world", source), 42);
        }

        TEST_METHOD(parse_noSpaceSeparator) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Then<Integer>(L"bar").Executes(command);

            try {
                subject.Execute(L"foo$", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(testExecuteInvalidSubcommand) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo").Then<Integer>(L"bar").Executes(command);

            try {
                subject.Execute(L"foo bar", source);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 4);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        //TEST_METHOD(testGetPath) {
        //    RootCommandNodeW<int> subject;
        //    auto bar = MakeLiteral<wchar_t, int>(L"bar");
        //    subject.Then(L"foo").Then(bar);
        //
        //    AssertArray(subject.GetPath(bar.GetCommandNode().get()), { L"foo", L"bar" });
        //}

        //TEST_METHOD(testFindNodeExists) {
        //    RootCommandNodeW<int> subject;
        //    auto bar = MakeLiteral<wchar_t, int>(L"bar");
        //    subject.Then(L"foo").Then(bar);
        //
        //    Assert::AreEqual((void*)subject.FindNode({ L"foo", L"bar" }), (void*)bar.GetCommandNode().get());
        //}

        TEST_METHOD(testFindNodeDoesntExist) {
            RootCommandNodeW<int> subject;
            Assert::IsNull(subject.FindNode({ L"foo", L"bar" }));
        }
    };

    TEST_CLASS(CommandDispatcherUsagesTest)
    {
        RootCommandNodeW<int> subject;

        TEST_METHOD_INITIALIZE(init)
        {
            auto& a = subject.Then(L"a");

            auto& a1 = a.Then(L"1");
            a1.Then(L"i").Executes(command);
            a1.Then(L"ii").Executes(command);

            auto& a2 = a.Then(L"2");
            a2.Then(L"i").Executes(command);
            a2.Then(L"ii").Executes(command);

            subject.Then(L"b").Then(L"1").Executes(command);

            subject.Then(L"c").Executes(command);

            subject.Then(L"d").Requires([](int&) -> bool { return false; }).Executes(command);

            auto& e1 = subject.Then(L"e").Executes(command).Then(L"1").Executes(command);
            e1.Then(L"i").Executes(command);
            e1.Then(L"ii").Executes(command);

            auto& f = subject.Then(L"f");
            auto& f1 = f.Then(L"1");
            f1.Then(L"i").Executes(command);
            f1.Then(L"ii").Executes(command).Requires([](int&) -> bool { return false; });
            auto& f2 = f.Then(L"2");
            f2.Then(L"i").Executes(command).Requires([](int&) -> bool { return false; });
            f2.Then(L"ii").Executes(command);

            subject.Then(L"g").Executes(command).Then(L"1").Then(L"i").Executes(command);

            auto& h = subject.Then(L"h").Executes(command);
            h.Then(L"1").Then(L"i").Executes(command);
            h.Then(L"2").Then(L"i").Then(L"ii").Executes(command);
            h.Then(L"3").Executes(command);

            auto& i = subject.Then(L"i").Executes(command);
            i.Then(L"1").Executes(command);
            i.Then(L"2").Executes(command);

            subject.Then(L"j").Redirect(subject);
            subject.Then(L"k").Redirect(h);
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
            subject = RootCommandNodeW<int>();
            auto results = subject.GetAllUsage(&subject, source, true);
            Assert::IsTrue(results.empty());
        }


        TEST_METHOD(testSmartUsage_noCommands) {
            subject = RootCommandNodeW<int>();
            auto results = subject.GetSmartUsage(&subject, source);
            Assert::IsTrue(results.empty());
        }


        TEST_METHOD(testAllUsage_root) {
            auto results = subject.GetAllUsage(&subject, source, true);
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
            auto results = subject.GetSmartUsage(&subject, source);
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
        void testSuggestions(RootCommandNodeW<int>& subject, std::wstring_view contents, size_t cursor, StringRange range, std::vector<std::wstring> suggestions = {}) {
            auto result = subject.GetCompletionSuggestions(subject.Parse(contents, source), cursor).get();
            AssertRange(result.GetRange(), range);

            std::set<SuggestionW, CompareNoCase<wchar_t>> expected;
            for (auto& suggestion : suggestions) {
                expected.emplace(range, suggestion);
            }

            AssertSet(result.GetList(), expected);
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo");
            subject.Then(L"bar");
            subject.Then(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(L"", source)).get();

            AssertRange(result.GetRange(), StringRange::At(0));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(0), L"bar"), SuggestionW(StringRange::At(0), L"baz"), SuggestionW(StringRange::At(0), L"foo") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_withInputOffset) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo");
            subject.Then(L"bar");
            subject.Then(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(InputWithOffset(L"OOO", 3), source)).get();

            AssertRange(result.GetRange(), StringRange::At(3));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(3), L"bar"), SuggestionW(StringRange::At(3), L"baz"), SuggestionW(StringRange::At(3), L"foo") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_partial) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo");
            subject.Then(L"bar");
            subject.Then(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(L"b", source)).get();

            AssertRange(result.GetRange(), StringRange::Between(0, 1));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(0, 1), L"bar"), SuggestionW(StringRange::Between(0, 1), L"baz") });
        }


        TEST_METHOD(getCompletionSuggestions_rootCommands_partial_withInputOffset) {
            RootCommandNodeW<int> subject;
            subject.Then(L"foo");
            subject.Then(L"bar");
            subject.Then(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(InputWithOffset(L"Zb", 1), source)).get();

            AssertRange(result.GetRange(), StringRange::Between(1, 2));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(1, 2), L"bar"), SuggestionW(StringRange::Between(1, 2), L"baz") });
        }


        TEST_METHOD(getCompletionSuggestions_subCommands) {
            RootCommandNodeW<int> subject;
            auto& parent = subject.Then(L"parent");
            parent.Then(L"foo");
            parent.Then(L"bar");
            parent.Then(L"baz");

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(L"parent ", source)).get();

            AssertRange(result.GetRange(), StringRange::At(7));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(7), L"bar"), SuggestionW(StringRange::At(7), L"baz"), SuggestionW(StringRange::At(7), L"foo") });
        }


        TEST_METHOD(getCompletionSuggestions_movingCursor_subCommands) {
            RootCommandNodeW<int> subject;
            auto& p1 = subject.Then(L"parent_one");
            p1.Then(L"faz");
            p1.Then(L"fbz");
            p1.Then(L"gaz");

            subject.Then(L"parent_two");

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
            RootCommandNodeW<int> subject;
            auto& parent = subject.Then(L"parent");
            parent.Then(L"foo");
            parent.Then(L"bar");
            parent.Then(L"baz");

            auto parse = subject.Parse(L"parent b", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(7, 8));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(7, 8), L"bar"), SuggestionW(StringRange::Between(7, 8), L"baz") });
        }


        TEST_METHOD(getCompletionSuggestions_subCommands_partial_withInputOffset) {
            RootCommandNodeW<int> subject;
            auto& parent = subject.Then(L"parent");
            parent.Then(L"foo");
            parent.Then(L"bar");
            parent.Then(L"baz");
            
            auto parse = subject.Parse(InputWithOffset(L"junk parent b", 5), source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(12, 13));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(12, 13), L"bar"), SuggestionW(StringRange::Between(12, 13), L"baz") });
        }


        TEST_METHOD(getCompletionSuggestions_redirect) {
            RootCommandNodeW<int> subject;
            auto& actual = subject.Then(L"actual");
            actual.Then(L"sub");
            subject.Then(L"redirect").Redirect(actual);

            auto parse = subject.Parse(L"redirect ", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::At(9));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(9), L"sub") });
        }


        TEST_METHOD(getCompletionSuggestions_redirectPartial) {
            RootCommandNodeW<int> subject;
            auto& actual = subject.Then(L"actual");
            actual.Then(L"sub");
            subject.Then(L"redirect").Redirect(actual);

            auto parse = subject.Parse(L"redirect s", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(9, 10));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(9, 10), L"sub") });
        }


        TEST_METHOD(getCompletionSuggestions_movingCursor_redirect) {
            RootCommandNodeW<int> subject;
            auto& actualOne = subject.Then(L"actual_one");
            actualOne.Then(L"faz");
            actualOne.Then(L"fbz");
            actualOne.Then(L"gaz");

            subject.Then(L"actual_two");

            subject.Then(L"redirect_one").Redirect(actualOne);
            subject.Then(L"redirect_two").Redirect(actualOne);

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
            RootCommandNodeW<int> subject;
            auto& actual = subject.Then(L"actual");
            actual.Then(L"sub");
            subject.Then(L"redirect").Redirect(actual);

            auto parse = subject.Parse(InputWithOffset(L"/redirect s", 1), source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::Between(10, 11));
            AssertSet(result.GetList(), { SuggestionW(StringRange::Between(10, 11), L"sub") });
        }


        TEST_METHOD(getCompletionSuggestions_redirect_lots) {
            RootCommandNodeW<int> subject;
            auto& loop = subject.Then(L"redirect");
            loop.Then(L"loop").Then<Integer>(L"loop").Redirect(loop);

            SuggestionsW result = subject.GetCompletionSuggestions(subject.Parse(L"redirect loop 1 loop 02 loop 003 ", source)).get();

            AssertRange(result.GetRange(), StringRange::At(33));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(33), L"loop") });
        }


        TEST_METHOD(getCompletionSuggestions_execute_simulation) {
            RootCommandNodeW<int> subject;
            auto& execute = subject.Then(L"execute");
            execute.Then(L"as").Then<Word>(L"name").Redirect(execute);
            execute.Then(L"store").Then<Word>(L"name").Redirect(execute);
            execute.Then(L"run").Executes(command);

            auto parse = subject.Parse(L"execute as Dinnerbone as", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            Assert::AreEqual(result.IsEmpty(), true);
        }


        TEST_METHOD(getCompletionSuggestions_execute_simulation_partial) {
            RootCommandNodeW<int> subject;
            auto& execute = subject.Then(L"execute");
            auto& as = execute.Then(L"as");
            as.Then(L"bar").Redirect(execute);
            as.Then(L"baz").Redirect(execute);
            execute.Then(L"store").Then<Word>(L"name").Redirect(execute);
            execute.Then(L"run").Executes(command);

            auto parse = subject.Parse(L"execute as bar as ", source);
            SuggestionsW result = subject.GetCompletionSuggestions(parse).get();

            AssertRange(result.GetRange(), StringRange::At(18));
            AssertSet(result.GetList(), { SuggestionW(StringRange::At(18), L"bar"), SuggestionW(StringRange::At(18), L"baz") });
        }
    };

    // Benchmarks
    TEST_CLASS(ParseBenchmarks)
    {
        RootCommandNodeW<int> subject;

        TEST_METHOD_INITIALIZE(init)
        {
            auto& a = subject.Then(L"a");

            auto& a1 = a.Then(L"1");
            a1.Then(L"i").Executes(command);
            a1.Then(L"ii").Executes(command);

            auto& a2 = a.Then(L"2");
            a2.Then(L"i").Executes(command);
            a2.Then(L"ii").Executes(command);

            subject.Then(L"b").Then(L"1").Executes(command);

            subject.Then(L"c").Executes(command);

            subject.Then(L"d").Requires([](int&) -> bool { return false; }).Executes(command);

            auto& e1 = subject.Then(L"e").Executes(command).Then(L"1").Executes(command);
            e1.Then(L"i").Executes(command);
            e1.Then(L"ii").Executes(command);

            auto& f = subject.Then(L"f");
            auto& f1 = f.Then(L"1");
            f1.Then(L"i").Executes(command);
            f1.Then(L"ii").Executes(command).Requires([](int&) -> bool { return false; });
            auto& f2 = f.Then(L"2");
            f2.Then(L"i").Executes(command).Requires([](int&) -> bool { return false; });
            f2.Then(L"ii").Executes(command);

            subject.Then(L"g").Executes(command).Then(L"1").Then(L"i").Executes(command);

            auto& h = subject.Then(L"h").Executes(command);
            h.Then(L"1").Then(L"i").Executes(command);
            h.Then(L"2").Then(L"i").Then(L"ii").Executes(command);
            h.Then(L"3").Executes(command);

            auto& i = subject.Then(L"i").Executes(command);
            i.Then(L"1").Executes(command);
            i.Then(L"2").Executes(command);

            subject.Then(L"j").Redirect(subject);
            subject.Then(L"k").Redirect(h);
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
        RootCommandNodeW<int> dispatcher;
        std::optional<ParseResultsW<int>> simple;
        std::optional<ParseResultsW<int>> singleRedirect;
        std::optional<ParseResultsW<int>> forkedRedirect;

        TEST_METHOD_INITIALIZE(setup)
        {
            dispatcher.Then(L"command").Executes(command);
            dispatcher.Then(L"redirect").Redirect(dispatcher);
            dispatcher.Then(L"fork").Fork(dispatcher, [](CommandContextW<int>& ctx) -> std::vector<int> { return { 1, 2, 3 }; });
            simple.emplace(dispatcher.Parse(L"command", source));
            singleRedirect.emplace(dispatcher.Parse(L"redirect command", source));
            forkedRedirect.emplace(dispatcher.Parse(L"fork command", source));
        }

        TEST_METHOD_CLEANUP(cleanup)
        {
            dispatcher = {};
            simple = std::nullopt;
            singleRedirect = std::nullopt;
            forkedRedirect = std::nullopt;
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
        static inline int creations = 0;
        static inline int destructions = 0;
        struct CopyGuard
        {
            __declspec(noinline) CopyGuard() { ++creations; }
            __declspec(noinline) CopyGuard(CopyGuard const&) { ++copies; }
            __declspec(noinline) CopyGuard(CopyGuard&&) { ++moves; }
            __declspec(noinline) CopyGuard& operator=(CopyGuard&&) = default;
            __declspec(noinline) ~CopyGuard() { ++destructions; }
        };

        TEST_METHOD_INITIALIZE(setup) { copies = 0; moves = 0; creations = 0; destructions = 0; }
        TEST_METHOD_CLEANUP(cleanup)  { copies = 0; moves = 0; creations = 0; destructions = 0; }

        TEST_METHOD(CopyMoveTest)
        {
            using Src = CopyGuard;

            RootCommandNodeW<Src> dispatcher;

            auto& foo = dispatcher.Then(L"foo");
            foo.Then<Integer>(L"bar").Executes([](CommandContextW<Src>& ctx) {
                printf("Bar is %d\n", ctx.GetArgument<Integer>(L"bar"));
                return 1;
            });
            foo.Executes([](CommandContextW<Src>& ctx) {
                puts("Called foo without arguments");
                return 1;
            });

            dispatcher.Execute(L"foo 1", {});

            Logger::WriteMessage((std::stringstream{} << creations << " creations\n" << copies << " copies\n" << moves << " moves\n" << destructions << " destructions").str().c_str());
        }
    };
}
