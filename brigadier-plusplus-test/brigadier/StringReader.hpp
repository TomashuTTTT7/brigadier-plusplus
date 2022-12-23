#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(StringReaderTest)
    {
        TEST_METHOD(CanRead) {
            WStringReader reader(L"abc");
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // L'a'
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // L'b'
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // L'c'
            Assert::AreEqual(reader.CanRead(), { false });
        }
        
        TEST_METHOD(GetRemainingLength) {
            WStringReader reader(L"abc");
            Assert::AreEqual(reader.GetRemainingLength(), { 3 });
            reader.SetCursor(1);
            Assert::AreEqual(reader.GetRemainingLength(), { 2 });
            reader.SetCursor(2);
            Assert::AreEqual(reader.GetRemainingLength(), { 1 });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRemainingLength(), { 0 });
        }

        TEST_METHOD(CanRead_length) {
            WStringReader reader(L"abc");
            Assert::AreEqual(reader.CanRead(1), { true });
            Assert::AreEqual(reader.CanRead(2), { true });
            Assert::AreEqual(reader.CanRead(3), { true });
            Assert::AreEqual(reader.CanRead(4), { false });
            Assert::AreEqual(reader.CanRead(5), { false });
        }

        TEST_METHOD(Peek) {
            WStringReader reader(L"abc");
            Assert::AreEqual(reader.Peek(), L'a');
            Assert::AreEqual<size_t>(reader.GetCursor(), 0);
            reader.SetCursor(2);
            Assert::AreEqual(reader.Peek(), L'c');
            Assert::AreEqual<size_t>(reader.GetCursor(), 2);
        }

        TEST_METHOD(Peek_length) {
            WStringReader reader(L"abc");
            Assert::AreEqual(reader.Peek(0), L'a');
            Assert::AreEqual(reader.Peek(2), L'c');
            Assert::AreEqual<size_t>(reader.GetCursor(), 0);
            reader.SetCursor(1);
            Assert::AreEqual(reader.Peek(1), L'c');
            Assert::AreEqual<size_t>(reader.GetCursor(), 1);
        }

        TEST_METHOD(Read) {
            WStringReader reader(L"abc");
            Assert::AreEqual(reader.Read(), L'a');
            Assert::AreEqual(reader.Read(), L'b');
            Assert::AreEqual(reader.Read(), L'c');
            Assert::AreEqual<size_t>(reader.GetCursor(), 3);
        }

        TEST_METHOD(Skip) {
            WStringReader reader(L"abc");
            reader.Skip();
            Assert::AreEqual<size_t>(reader.GetCursor(), 1);
        }

        TEST_METHOD(GetRemaining) {
            WStringReader reader(L"Hello!");
            Assert::AreEqual(reader.GetRemaining(), { L"Hello!" });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRemaining(), { L"lo!" });
            reader.SetCursor(6);
            Assert::AreEqual(reader.GetRemaining(), { L"" });
        }
        
        TEST_METHOD(GetRead) {
            WStringReader reader(L"Hello!");
            Assert::AreEqual(reader.GetRead(), { L"" });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRead(), { L"Hel" });
            reader.SetCursor(6);
            Assert::AreEqual(reader.GetRead(), { L"Hello!" });
        }

        TEST_METHOD(SkipWhitespace_none) {
            WStringReader reader(L"Hello!");
            reader.SkipWhitespace();
            Assert::AreEqual<size_t>(reader.GetCursor(), 0);
        }

        TEST_METHOD(SkipWhitespace_mixed) {
            WStringReader reader(L" \t \t\nHello!");
            reader.SkipWhitespace();
            Assert::AreEqual<size_t>(reader.GetCursor(), 5);
        }

        TEST_METHOD(SkipWhitespace_empty) {
            WStringReader reader(L"");
            reader.SkipWhitespace();
            Assert::AreEqual<size_t>(reader.GetCursor(), 0);
        }

        TEST_METHOD(ReadUnquotedString) {
            WStringReader reader(L"hello world");
            Assert::AreEqual(reader.ReadUnquotedString(), { L"hello" });
            Assert::AreEqual(reader.GetRead(), {L"hello"});
            Assert::AreEqual(reader.GetRemaining(), { L" world" });
        }

        TEST_METHOD(ReadUnquotedString_empty) {
            WStringReader reader(L"");
            Assert::AreEqual(reader.ReadUnquotedString(), { L"" });
            Assert::AreEqual(reader.GetRead(), { L"" });
            Assert::AreEqual(reader.GetRemaining(), { L"" });
        }

        TEST_METHOD(ReadUnquotedString_empty_withRemaining) {
            WStringReader reader(L" hello world");
            Assert::AreEqual(reader.ReadUnquotedString(), { L"" });
            Assert::AreEqual(reader.GetRead(), { L"" });
            Assert::AreEqual(reader.GetRemaining(), { L" hello world" });
        }

        TEST_METHOD(ReadQuotedString) {
            WStringReader reader(L"\"hello world\"");
            Assert::AreEqual(reader.ReadQuotedString(), { L"hello world" });
            Assert::AreEqual(reader.GetRead(), { L"\"hello world\"" });
            Assert::AreEqual(reader.GetRemaining(), { L"" });
        }

        TEST_METHOD(ReadSingleQuotedString) {
            WStringReader reader(L"'hello world'");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"'hello world'"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadMixedQuotedString_doubleInsideSingle) {
            WStringReader reader(L"'hello \"world\"'");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello \"world\""});
            Assert::AreEqual(reader.GetRead(), {L"'hello \"world\"'"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadMixedQuotedString_singleInsideDouble) {
            WStringReader reader(L"\"hello L'world'\"");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello L'world'"});
            Assert::AreEqual(reader.GetRead(), {L"\"hello L'world'\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_empty) {
            WStringReader reader(L"");
            Assert::AreEqual(reader.ReadQuotedString(), {L""});
            Assert::AreEqual(reader.GetRead(), {L""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_emptyQuoted) {
            WStringReader reader(L"\"\"");
            Assert::AreEqual(reader.ReadQuotedString(), {L""});
            Assert::AreEqual(reader.GetRead(), {L"\"\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_emptyQuoted_withRemaining) {
            WStringReader reader(L"\"\" hello world");
            Assert::AreEqual(reader.ReadQuotedString(), {L""});
            Assert::AreEqual(reader.GetRead(), {L"\"\""});
            Assert::AreEqual(reader.GetRemaining(), {L" hello world"});
        }

        TEST_METHOD(ReadQuotedString_withEscapedQuote) {
            WStringReader reader(L"\"hello \\\"world\\\"\"");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello \"world\""});
            Assert::AreEqual(reader.GetRead(), {L"\"hello \\\"world\\\"\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_withEscapedEscapes) {
            WStringReader reader(L"\"\\\\o/\"");
            Assert::AreEqual(reader.ReadQuotedString(), {L"\\o/"});
            Assert::AreEqual(reader.GetRead(), {L"\"\\\\o/\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadQuotedString_withRemaining) {
            WStringReader reader(L"\"hello world\" foo bar");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadQuotedString_withImmediateRemaining) {
            WStringReader reader(L"\"hello world\"foo bar");
            Assert::AreEqual(reader.ReadQuotedString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(ReadQuotedString_noOpen) {
            try {
                WStringReader(L"hello world\"").ReadQuotedString();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 0);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadQuotedString_noClose) {
            try {
                WStringReader(L"\"hello world").ReadQuotedString();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 12);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadQuotedString_invalidEscape) {
            try {
                WStringReader(L"\"hello\\nworld\"").ReadQuotedString();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 7);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadQuotedString_invalidQuoteEscape) {
            try {
                WStringReader(L"'hello\\\"\'world").ReadQuotedString();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 7);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadString_noQuotes) {
            WStringReader reader(L"hello world");
            Assert::AreEqual(reader.ReadString(), {L"hello"});
            Assert::AreEqual(reader.GetRead(), {L"hello"});
            Assert::AreEqual(reader.GetRemaining(), {L" world"});
        }

        TEST_METHOD(ReadString_singleQuotes) {
            WStringReader reader(L"'hello world'");
            Assert::AreEqual(reader.ReadString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"'hello world'"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadString_doubleQuotes) {
            WStringReader reader(L"\"hello world\"");
            Assert::AreEqual(reader.ReadString(), {L"hello world"});
            Assert::AreEqual(reader.GetRead(), {L"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadStringUntil_invalid) {
            try {
                WStringReader(L"hello\\, world").ReadStringUntil(L',');
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual<size_t>(ex.GetCursor(), 13);
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadStringUntil) {
            WStringReader reader(L"hello\\, world, ");
            Assert::AreEqual(reader.ReadStringUntil(L','), {L"hello, world"});
            Assert::AreEqual(reader.GetRead(), { L"hello\\, world," });
            Assert::AreEqual(reader.GetRemaining(), { L" " });
        }

        TEST_METHOD(ReadStringUntilOneOf) {
            WStringReader reader(L"hello, world");
            Assert::AreEqual(reader.ReadStringUntilOneOf(L"wo"), {L"hell"});
            Assert::AreEqual(reader.GetRead(), { L"hello" });
            Assert::AreEqual(reader.GetRemaining(), { L", world" });
        }

        TEST_METHOD(ReadUnquotedStringUntil) {
            WStringReader reader(L"hello_world");
            Assert::AreEqual(reader.ReadUnquotedStringUntil(L'_'), { L"hello" });
            Assert::AreEqual(reader.GetRead(), { L"hello_" });
            Assert::AreEqual(reader.GetRemaining(), { L"world" });
        }

        TEST_METHOD(ReadUnquotedStringUntilOneOf) {
            WStringReader reader(L"hello_world");
            Assert::AreEqual(reader.ReadUnquotedStringUntilOneOf(L"wo"), { L"hell" });
            Assert::AreEqual(reader.GetRead(), { L"hello" });
            Assert::AreEqual(reader.GetRemaining(), { L"_world" });
        }

        TEST_METHOD(ReadInt) {
            WStringReader reader(L"1234567890");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadInt_negative) {
            WStringReader reader(L"-1234567890");
            Assert::AreEqual(reader.ReadValue<int>(), {-1234567890});
            Assert::AreEqual(reader.GetRead(), {L"-1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadInt_invalid) {
            WStringReader reader(L"12.34");
            Assert::AreEqual(reader.ReadValue<int>(), 12);
            Assert::AreEqual(reader.GetRemaining(), { L".34" });
        }

        TEST_METHOD(ReadInt_none) {
            try {
                WStringReader(L"").ReadValue<int>();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadInt_withRemaining) {
            WStringReader reader(L"1234567890 foo bar");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadInt_withRemainingImmediate) {
            WStringReader reader(L"1234567890foo bar");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(ReadLong) {
            WStringReader reader(L"1234567890");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadLong_negative) {
            WStringReader reader(L"-1234567890");
            Assert::AreEqual(reader.ReadValue<long>(), {-1234567890L});
            Assert::AreEqual(reader.GetRead(), {L"-1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadLong_invalid) {
            WStringReader reader(L"12.34");
            Assert::AreEqual(reader.ReadValue<long>(), 12l);
            Assert::AreEqual(reader.GetRemaining(), { L".34" });
        }

        TEST_METHOD(ReadLong_none) {
            try {
                WStringReader(L"").ReadValue<long>();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadLong_withRemaining) {
            WStringReader reader(L"1234567890 foo bar");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadLong_withRemainingImmediate) {
            WStringReader reader(L"1234567890foo bar");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {L"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(ReadDouble) {
            WStringReader reader(L"123");
            Assert::AreEqual(reader.ReadValue<double>(), {123.0});
            Assert::AreEqual(reader.GetRead(), {L"123"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadDouble_withDecimal) {
            WStringReader reader(L"12.34");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadDouble_negative) {
            WStringReader reader(L"-123");
            Assert::AreEqual(reader.ReadValue<double>(), {-123.0});
            Assert::AreEqual(reader.GetRead(), {L"-123"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadDouble_invalid) {
            try {
                WStringReader(L"12.34.56").ReadValue<double>();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadDouble_none) {
            try {
                WStringReader(L"").ReadValue<double>();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadDouble_withRemaining) {
            WStringReader reader(L"12.34 foo bar");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadDouble_withRemainingImmediate) {
            WStringReader reader(L"12.34foo bar");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(ReadFloat) {
            WStringReader reader(L"123");
            Assert::AreEqual(reader.ReadValue<float>(), {123.0f});
            Assert::AreEqual(reader.GetRead(), {L"123"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadFloat_withDecimal) {
            WStringReader reader(L"12.34");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadFloat_negative) {
            WStringReader reader(L"-123");
            Assert::AreEqual(reader.ReadValue<float>(), {-123.0f});
            Assert::AreEqual(reader.GetRead(), {L"-123"});
            Assert::AreEqual(reader.GetRemaining(), {L""});
        }

        TEST_METHOD(ReadFloat_invalid) {
            try {
                WStringReader(L"12.34.56").ReadValue<float>();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadFloat_none) {
            try {
                WStringReader(L"").ReadValue<float>();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadFloat_withRemaining) {
            WStringReader reader(L"12.34 foo bar");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L" foo bar"});
        }

        TEST_METHOD(ReadFloat_withRemainingImmediate) {
            WStringReader reader(L"12.34foo bar");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {L"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {L"foo bar"});
        }

        TEST_METHOD(expect_correct) {
            WStringReader reader(L"abc");
            reader.Expect('a');
            Assert::AreEqual(reader.GetCursor(), {1});
        }

        TEST_METHOD(expect_incorrect) {
            WStringReader reader(L"bcd");
            try {
                reader.Expect('a');
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(expect_none) {
            WStringReader reader(L"");
            try {
                reader.Expect('a');
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadBoolean_correct) {
            WStringReader reader(L"true");
            Assert::AreEqual(reader.ReadValue<bool>(), {true});
            Assert::AreEqual(reader.GetRead(), {L"true"});
        }

        TEST_METHOD(ReadBoolean_incorrect) {
            WStringReader reader(L"tuesday");
            try {
                reader.ReadValue<bool>();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }

        TEST_METHOD(ReadBoolean_none) {
            WStringReader reader(L"");
            try {
                reader.ReadValue<bool>();
                Assert::Fail();
            }
            catch (WCommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
            catch (...) {
                Assert::Fail();
            }
        }
    };
}
