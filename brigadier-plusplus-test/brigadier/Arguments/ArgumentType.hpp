#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(BoolArgumentTypeTest)
    {
        TEST_METHOD(parse)
        {
            WStringReader readert(L"true");
            Assert::AreEqual(BoolArgumentType<wchar_t>().Parse(readert), true);
            WStringReader readerf(L"false");
            Assert::AreEqual(BoolArgumentType<wchar_t>().Parse(readerf), false);
        }
    };


    TEST_CLASS(ArithmeticArgumentTypeTest)
    {
        TEST_METHOD(parse)
        {
            WStringReader reader(L"15");
            Assert::AreEqual(Double<wchar_t>().Parse(reader), 15.0);
            Assert::AreEqual(reader.CanRead(), false);
            reader.SetCursor(0);
            Assert::AreEqual(Float<wchar_t>().Parse(reader), 15.f);
            Assert::AreEqual(reader.CanRead(), false);
            reader.SetCursor(0);
            Assert::AreEqual(Integer<wchar_t>().Parse(reader), 15);
            Assert::AreEqual(reader.CanRead(), false);
            reader.SetCursor(0);
            Assert::AreEqual(Long<wchar_t>().Parse(reader), 15ll);
            Assert::AreEqual(reader.CanRead(), false);
        }

        TEST_METHOD(parse_tooSmall)
        {
            WStringReader reader(L"-5");
            try {
                Double<wchar_t>(0, 100).Parse(reader);
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Float<wchar_t>(0, 100).Parse(reader);
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Integer<wchar_t>(0, 100).Parse(reader);
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Long<wchar_t>(0, 100).Parse(reader);
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
        }

        TEST_METHOD(parse_tooBig)
        {
            WStringReader reader(L"5");
            try {
                Double<wchar_t>(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Float<wchar_t>(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Integer<wchar_t>(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Long<wchar_t>(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
        }
    };

    TEST_CLASS(StringArgumentTypeTest)
    {
        TEST_METHOD(testParseWord)
        {
            WStringReader reader(L"hello");
            Assert::AreEqual(Word<wchar_t>().Parse(reader), { L"hello" });
        }

        TEST_METHOD(testParseString)
        {
            WStringReader reader(L"\"hello world\"");
            Assert::AreEqual(String<wchar_t>().Parse(reader), { L"hello world" });
        }
        
        TEST_METHOD(testParseGreedyString)
        {
            WStringReader reader(L"Hello world! This is a test.");
            Assert::AreEqual(GreedyString<wchar_t>().Parse(reader), { L"Hello world! This is a test." });
            Assert::AreEqual(reader.CanRead(), false);
        }
        
        TEST_METHOD(testEscapeIfRequired_notRequired)
        {
            Assert::AreEqual(String<wchar_t>::EscapeIfRequired(L"hello"), { L"hello" });
            Assert::AreEqual(String<wchar_t>::EscapeIfRequired(L""), { L"" });
        }
        
        TEST_METHOD(testEscapeIfRequired_multipleWords)
        {
            Assert::AreEqual(String<wchar_t>::EscapeIfRequired(L"hello world"), { L"\"hello world\"" });
        }

        TEST_METHOD(testEscapeIfRequired_quote)
        {
            Assert::AreEqual(String<wchar_t>::EscapeIfRequired(L"hello \"world\"!"), { L"\"hello \\\"world\\\"!\"" });
        }
        
        TEST_METHOD(testEscapeIfRequired_escapes)
        {
            Assert::AreEqual(String<wchar_t>::EscapeIfRequired(L"\\"), { L"\"\\\\\"" });
        }

        TEST_METHOD(testEscapeIfRequired_singleQuote)
        {
            Assert::AreEqual(String<wchar_t>::EscapeIfRequired(L"\""), { L"\"\\\"\"" });
        }
    };
}
