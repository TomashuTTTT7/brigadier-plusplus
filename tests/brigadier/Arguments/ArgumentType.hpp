#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(BoolArgumentTypeTest)
    {
        TEST_METHOD(parse)
        {
            StringReaderW readert(L"true");
            Assert::AreEqual(BoolArgumentType().Parse(readert), true);
            StringReaderW readerf(L"false");
            Assert::AreEqual(BoolArgumentType().Parse(readerf), false);
        }
    };


    TEST_CLASS(ArithmeticArgumentTypeTest)
    {
        TEST_METHOD(parse)
        {
            StringReaderW reader(L"15");
            Assert::AreEqual(Double().Parse(reader), 15.0);
            Assert::AreEqual(reader.CanRead(), false);
            reader.SetCursor(0);
            Assert::AreEqual(Float().Parse(reader), 15.f);
            Assert::AreEqual(reader.CanRead(), false);
            reader.SetCursor(0);
            Assert::AreEqual(Integer().Parse(reader), 15);
            Assert::AreEqual(reader.CanRead(), false);
            reader.SetCursor(0);
            Assert::AreEqual(Long().Parse(reader), 15ll);
            Assert::AreEqual(reader.CanRead(), false);
        }

        TEST_METHOD(parse_tooSmall)
        {
            StringReaderW reader(L"-5");
            try {
                Double(0, 100).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Float(0, 100).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Integer(0, 100).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Long(0, 100).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
        }

        TEST_METHOD(parse_tooBig)
        {
            StringReaderW reader(L"5");
            try {
                Double(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Float(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Integer(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
            try {
                Long(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) { Assert::AreEqual<size_t>(ex.GetCursor(), 0); }
            catch (...) { Assert::Fail(); }
        }
    };

    TEST_CLASS(StringArgumentTypeTest)
    {
        TEST_METHOD(testParseWord)
        {
            StringReaderW reader(L"hello");
            Assert::AreEqual(Word<wchar_t>().Parse(reader), { L"hello" });
        }

        TEST_METHOD(testParseString)
        {
            StringReaderW reader(L"\"hello world\"");
            Assert::AreEqual(String<wchar_t>().Parse(reader), { L"hello world" });
        }
        
        TEST_METHOD(testParseGreedyString)
        {
            StringReaderW reader(L"Hello world! This is a test.");
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
