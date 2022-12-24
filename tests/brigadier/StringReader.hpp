#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(StringReaderTest)
    {
        TEST_METHOD(CanRead) {
            StringReaderW reader(L"abc");
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // L'a'
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // L'b'
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // L'c'
            Assert::AreEqual(reader.CanRead(), { false });
        }
        
        TEST_METHOD(GetRemainingLength) {
            StringReaderW reader(L"abc");
            Assert::AreEqual(reader.GetRemainingLength(), { 3 });
            reader.SetCursor(1);
            Assert::AreEqual(reader.GetRemainingLength(), { 2 });
            reader.SetCursor(2);
            Assert::AreEqual(reader.GetRemainingLength(), { 1 });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRemainingLength(), { 0 });
        }

        TEST_METHOD(CanRead_length) {
            StringReaderW reader(L"abc");
            Assert::AreEqual(reader.CanRead(1), { true });
            Assert::AreEqual(reader.CanRead(2), { true });
            Assert::AreEqual(reader.CanRead(3), { true });
            Assert::AreEqual(reader.CanRead(4), { false });
            Assert::AreEqual(reader.CanRead(5), { false });
        }

        TEST_METHOD(Peek) {
            StringReaderW reader(L"abc");
            Assert::AreEqual(reader.Peek(), L'a');
            Assert::AreEqual<size_t>(reader.GetCursor(), 0);
            reader.SetCursor(2);
            Assert::AreEqual(reader.Peek(), L'c');
            Assert::AreEqual<size_t>(reader.GetCursor(), 2);
        }

        TEST_METHOD(Peek_length) {
            StringReaderW reader(L"abc");
            Assert::AreEqual(reader.Peek(0), L'a');
            Assert::AreEqual(reader.Peek(2), L'c');
            Assert::AreEqual<size_t>(reader.GetCursor(), 0);
            reader.SetCursor(1);
            Assert::AreEqual(reader.Peek(1), L'c');
            Assert::AreEqual<size_t>(reader.GetCursor(), 1);
        }

        TEST_METHOD(Read) {
            StringReaderW reader(L"abc");
            Assert::AreEqual(reader.Read(), L'a');
            Assert::AreEqual(reader.Read(), L'b');
            Assert::AreEqual(reader.Read(), L'c');
            Assert::AreEqual<size_t>(reader.GetCursor(), 3);
        }

        TEST_METHOD(Skip) {
            StringReaderW reader(L"abc");
            reader.Skip();
            Assert::AreEqual<size_t>(reader.GetCursor(), 1);
        }

        TEST_METHOD(GetRemaining) {
            StringReaderW reader(L"Hello!");
            Assert::AreEqual(reader.GetRemaining(), { L"Hello!" });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRemaining(), { L"lo!" });
            reader.SetCursor(6);
            Assert::AreEqual(reader.GetRemaining(), { L"" });
        }
        
        TEST_METHOD(GetRead) {
            StringReaderW reader(L"Hello!");
            Assert::AreEqual(reader.GetRead(), { L"" });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRead(), { L"Hel" });
            reader.SetCursor(6);
            Assert::AreEqual(reader.GetRead(), { L"Hello!" });
        }

        TEST_METHOD(SkipWhitespace_none) {
            StringReaderW reader(L"Hello!");
            reader.SkipWhitespace();
            Assert::AreEqual<size_t>(reader.GetCursor(), 0);
        }

        TEST_METHOD(SkipWhitespace_mixed) {
            StringReaderW reader(L" \t \t\nHello!");
            reader.SkipWhitespace();
            Assert::AreEqual<size_t>(reader.GetCursor(), 5);
        }

        TEST_METHOD(SkipWhitespace_empty) {
            StringReaderW reader(L"");
            reader.SkipWhitespace();
            Assert::AreEqual<size_t>(reader.GetCursor(), 0);
        }

        TEST_METHOD(ReadUnquotedString) {
            StringReaderW reader(L"hello world");
            Assert::AreEqual(reader.ReadUnquotedString(), { L"hello" });
            Assert::AreEqual(reader.GetRead(), {L"hello"});
            Assert::AreEqual(reader.GetRemaining(), { L" world" });
        }

        TEST_METHOD(ReadUnquotedString_empty) {
            StringReaderW reader(L"");
            Assert::AreEqual(reader.ReadUnquotedString(), { L"" });
            Assert::AreEqual(reader.GetRead(), { L"" });
            Assert::AreEqual(reader.GetRemaining(), { L"" });
        }

        TEST_METHOD(ReadUnquotedString_empty_withRemaining) {
            StringReaderW reader(L" hello world");
            Assert::AreEqual(reader.ReadUnquotedString(), { L"" });
            Assert::AreEqual(reader.GetRead(), { L"" });
            Assert::AreEqual(reader.GetRemaining(), { L" hello world" });
        }

        TEST_METHOD(ReadQuotedString) {
            StringReaderW reader(L"\"hello world\"");
            Assert::AreEqual(reader.ReadQuotedString(), { L"hello world" });
            Assert::AreEqual(reader.GetRead(), { L"\"hello world\"" });
            Assert::AreEqual(reader.GetRemaining(), { L"" });
        }

        TEST_METHOD(ReadSingleQuotedString) {
            StringReaderW reader(L"'hello world'");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"'hello world'"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadMixedQuotedString_doubleInsideSingle) {
            StringReaderW reader(L"'hello \"world\"'");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello \"world\""});
            Assert::AreEqual(reader.GetRead(), {L"'hello \"world\"'"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadMixedQuotedString_singleInsideDouble) {
            StringReaderW reader(L"\"hello L'world'\"");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello L'world'"});
            Assert::AreEqual(reader.GetRead(), {L"\"hello L'world'\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_empty) {
            StringReaderW reader(L"");
            Assert::AreEqual(reader.ReadQuotedString(), {L""});
            Assert::AreEqual(reader.GetRead(), {L""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_emptyQuoted) {
            StringReaderW reader(L"\"\"");
            Assert::AreEqual(reader.ReadQuotedString(), {L""});
            Assert::AreEqual(reader.GetRead(), {L"\"\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_emptyQuoted_withRemaining) {
            StringReaderW reader(L"\"\" hello world");
            Assert::AreEqual(reader.ReadQuotedString(), {L""});
            Assert::AreEqual(reader.GetRead(), {L"\"\""});
            Assert::AreEqual(reader.GetRemaining(), {L" hello world"});
        }

        TEST_METHOD(ReadQuotedString_withEscapedQuote) {
            StringReaderW reader(L"\"hello \\\"world\\\"\"");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello \"world\""});
            Assert::AreEqual(reader.GetRead(), {L"\"hello \\\"world\\\"\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_withEscapedEscapes) {
            StringReaderW reader(L"\"\\\\o/\"");
            Assert::AreEqual(reader.ReadQuotedString(), {L"\\o/"});
            Assert::AreEqual(reader.GetRead(), {L"\"\\\\o/\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_withRemaining) {
            StringReaderW reader(L"\"hello world\" foo bar");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadQuotedString_withImmediateRemaining) {
            StringReaderW reader(L"\"hello world\"foo bar");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(ReadQuotedString_noOpen) {
            try {
                StringReaderW(L"hello world\"").ReadQuotedString();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadQuotedString_noClose) {
            try {
                StringReaderW(L"\"hello world").ReadQuotedString();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 12);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadQuotedString_invalidEscape) {
            try {
                StringReaderW(L"\"hello\\nworld\"").ReadQuotedString();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 7);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadQuotedString_invalidQuoteEscape) {
            try {
                StringReaderW(L"'hello\\\"\'world").ReadQuotedString();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 7);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadString_noQuotes) {
            StringReaderW reader(L"hello world");
            Assert::AreEqual(reader.ReadString(), {L"hello"});
            Assert::AreEqual(reader.GetRead(), {L"hello"});
            Assert::AreEqual(reader.GetRemaining(), {L" world"});
        }

        TEST_METHOD(ReadString_singleQuotes) {
            StringReaderW reader(L"'hello world'");
            Assert::AreEqual(reader.ReadString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"'hello world'"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadString_doubleQuotes) {
            StringReaderW reader(L"\"hello world\"");
            Assert::AreEqual(reader.ReadString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadStringUntil_invalid) {
            try {
                StringReaderW(L"hello\\, world").ReadStringUntil(L',');
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 13);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadStringUntil) {
            StringReaderW reader(L"hello\\, world, ");
            Assert::AreEqual(reader.ReadStringUntil(L','), {L"hello, world"});
            Assert::AreEqual(reader.GetRead(), { L"hello\\, world," });
            Assert::AreEqual(reader.GetRemaining(), { L" " });
        }

        TEST_METHOD(ReadStringUntilOneOf) {
            StringReaderW reader(L"hello, world");
            Assert::AreEqual(reader.ReadStringUntilOneOf(L"wo"), {L"hell"});
            Assert::AreEqual(reader.GetRead(), { L"hello" });
            Assert::AreEqual(reader.GetRemaining(), { L", world" });
        }

        TEST_METHOD(ReadUnquotedStringUntil) {
            StringReaderW reader(L"hello_world");
            Assert::AreEqual(reader.ReadUnquotedStringUntil(L'_'), { L"hello" });
            Assert::AreEqual(reader.GetRead(), { L"hello_" });
            Assert::AreEqual(reader.GetRemaining(), { L"world" });
        }

        TEST_METHOD(ReadUnquotedStringUntilOneOf) {
            StringReaderW reader(L"hello_world");
            Assert::AreEqual(reader.ReadUnquotedStringUntilOneOf(L"wo"), { L"hell" });
            Assert::AreEqual(reader.GetRead(), { L"hello" });
            Assert::AreEqual(reader.GetRemaining(), { L"_world" });
        }

        TEST_METHOD(ReadInt) {
            StringReaderW reader(L"1234567890");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadInt_negative) {
            StringReaderW reader(L"-1234567890");
            Assert::AreEqual(reader.ReadValue<int>(), {-1234567890});
            Assert::AreEqual(reader.GetRead(), {L"-1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadInt_invalid) {
            StringReaderW reader(L"12.34");
            Assert::AreEqual(reader.ReadValue<int>(), 12);
            Assert::AreEqual(reader.GetRemaining(), { L".34" });
        }

        TEST_METHOD(ReadInt_none) {
            try {
                StringReaderW(L"").ReadValue<int>();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadInt_withRemaining) {
            StringReaderW reader(L"1234567890 foo bar");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadInt_withRemainingImmediate) {
            StringReaderW reader(L"1234567890foo bar");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(ReadLong) {
            StringReaderW reader(L"1234567890");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadLong_negative) {
            StringReaderW reader(L"-1234567890");
            Assert::AreEqual(reader.ReadValue<long>(), {-1234567890L});
            Assert::AreEqual(reader.GetRead(), {L"-1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadLong_invalid) {
            StringReaderW reader(L"12.34");
            Assert::AreEqual(reader.ReadValue<long>(), 12l);
            Assert::AreEqual(reader.GetRemaining(), { L".34" });
        }

        TEST_METHOD(ReadLong_none) {
            try {
                StringReaderW(L"").ReadValue<long>();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadLong_withRemaining) {
            StringReaderW reader(L"1234567890 foo bar");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadLong_withRemainingImmediate) {
            StringReaderW reader(L"1234567890foo bar");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(ReadDouble) {
            StringReaderW reader(L"123");
            Assert::AreEqual(reader.ReadValue<double>(), {123.0});
            Assert::AreEqual(reader.GetRead(), {L"123"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadDouble_withDecimal) {
            StringReaderW reader(L"12.34");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadDouble_negative) {
            StringReaderW reader(L"-123");
            Assert::AreEqual(reader.ReadValue<double>(), {-123.0});
            Assert::AreEqual(reader.GetRead(), {L"-123"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadDouble_invalid) {
            try {
                StringReaderW(L"12.34.56").ReadValue<double>();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadDouble_none) {
            try {
                StringReaderW(L"").ReadValue<double>();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadDouble_withRemaining) {
            StringReaderW reader(L"12.34 foo bar");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadDouble_withRemainingImmediate) {
            StringReaderW reader(L"12.34foo bar");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(ReadFloat) {
            StringReaderW reader(L"123");
            Assert::AreEqual(reader.ReadValue<float>(), {123.0f});
            Assert::AreEqual(reader.GetRead(), {L"123"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadFloat_withDecimal) {
            StringReaderW reader(L"12.34");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadFloat_negative) {
            StringReaderW reader(L"-123");
            Assert::AreEqual(reader.ReadValue<float>(), {-123.0f});
            Assert::AreEqual(reader.GetRead(), {L"-123"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadFloat_invalid) {
            try {
                StringReaderW(L"12.34.56").ReadValue<float>();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadFloat_none) {
            try {
                StringReaderW(L"").ReadValue<float>();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadFloat_withRemaining) {
            StringReaderW reader(L"12.34 foo bar");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadFloat_withRemainingImmediate) {
            StringReaderW reader(L"12.34foo bar");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(expect_correct) {
            StringReaderW reader(L"abc");
            reader.Expect('a');
            Assert::AreEqual(reader.GetCursor(), {1});
        }

        TEST_METHOD(expect_incorrect) {
            StringReaderW reader(L"bcd");
            try {
                reader.Expect('a');
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(expect_none) {
            StringReaderW reader(L"");
            try {
                reader.Expect('a');
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadBoolean_correct) {
            StringReaderW reader(L"true");
            Assert::AreEqual(reader.ReadValue<bool>(), {true});
            Assert::AreEqual(reader.GetRead(), {L"true"});
        }

        TEST_METHOD(ReadBoolean_incorrect) {
            StringReaderW reader(L"tuesday");
            try {
                reader.ReadValue<bool>();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadBoolean_none) {
            StringReaderW reader(L"");
            try {
                reader.ReadValue<bool>();
                Assert::Fail();
            }
            catch (CommandSyntaxExceptionW const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }
    };
}
