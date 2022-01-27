#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(StringReaderTest)
    {
        TEST_METHOD(CanRead) {
            StringReader reader("abc");
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // 'a'
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // 'b'
            Assert::AreEqual(reader.CanRead(), { true });
            reader.Skip(); // 'c'
            Assert::AreEqual(reader.CanRead(), { false });
        }
        
        TEST_METHOD(GetRemainingLength) {
            StringReader reader("abc");
            Assert::AreEqual(reader.GetRemainingLength(), { 3 });
            reader.SetCursor(1);
            Assert::AreEqual(reader.GetRemainingLength(), { 2 });
            reader.SetCursor(2);
            Assert::AreEqual(reader.GetRemainingLength(), { 1 });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRemainingLength(), { 0 });
        }

        TEST_METHOD(CanRead_length) {
            StringReader reader("abc");
            Assert::AreEqual(reader.CanRead(1), { true });
            Assert::AreEqual(reader.CanRead(2), { true });
            Assert::AreEqual(reader.CanRead(3), { true });
            Assert::AreEqual(reader.CanRead(4), { false });
            Assert::AreEqual(reader.CanRead(5), { false });
        }

        TEST_METHOD(Peek) {
            StringReader reader("abc");
            Assert::AreEqual(reader.Peek(), 'a');
            Assert::AreEqual(reader.GetCursor(), 0);
            reader.SetCursor(2);
            Assert::AreEqual(reader.Peek(), 'c');
            Assert::AreEqual(reader.GetCursor(), 2);
        }

        TEST_METHOD(Peek_length) {
            StringReader reader("abc");
            Assert::AreEqual(reader.Peek(0), 'a');
            Assert::AreEqual(reader.Peek(2), 'c');
            Assert::AreEqual(reader.GetCursor(), 0);
            reader.SetCursor(1);
            Assert::AreEqual(reader.Peek(1), 'c');
            Assert::AreEqual(reader.GetCursor(), 1);
        }

        TEST_METHOD(Read) {
            StringReader reader("abc");
            Assert::AreEqual(reader.Read(), 'a');
            Assert::AreEqual(reader.Read(), 'b');
            Assert::AreEqual(reader.Read(), 'c');
            Assert::AreEqual(reader.GetCursor(), 3);
        }

        TEST_METHOD(Skip) {
            StringReader reader("abc");
            reader.Skip();
            Assert::AreEqual(reader.GetCursor(), 1);
        }

        TEST_METHOD(GetRemaining) {
            StringReader reader("Hello!");
            Assert::AreEqual(reader.GetRemaining(), { "Hello!" });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRemaining(), { "lo!" });
            reader.SetCursor(6);
            Assert::AreEqual(reader.GetRemaining(), { "" });
        }
        
        TEST_METHOD(GetRead) {
            StringReader reader("Hello!");
            Assert::AreEqual(reader.GetRead(), { "" });
            reader.SetCursor(3);
            Assert::AreEqual(reader.GetRead(), { "Hel" });
            reader.SetCursor(6);
            Assert::AreEqual(reader.GetRead(), { "Hello!" });
        }

        TEST_METHOD(SkipWhitespace_none) {
            StringReader reader("Hello!");
            reader.SkipWhitespace();
            Assert::AreEqual(reader.GetCursor(), 0);
        }

        TEST_METHOD(SkipWhitespace_mixed) {
            StringReader reader(" \t \t\nHello!");
            reader.SkipWhitespace();
            Assert::AreEqual(reader.GetCursor(), 5);
        }

        TEST_METHOD(SkipWhitespace_empty) {
            StringReader reader("");
            reader.SkipWhitespace();
            Assert::AreEqual(reader.GetCursor(), 0);
        }

        TEST_METHOD(ReadUnquotedString) {
            StringReader reader("hello world");
            Assert::AreEqual(reader.ReadUnquotedString(), { "hello" });
            Assert::AreEqual(reader.GetRead(), {"hello"});
            Assert::AreEqual(reader.GetRemaining(), { " world" });
        }

        TEST_METHOD(ReadUnquotedString_empty) {
            StringReader reader("");
            Assert::AreEqual(reader.ReadUnquotedString(), { "" });
            Assert::AreEqual(reader.GetRead(), { "" });
            Assert::AreEqual(reader.GetRemaining(), { "" });
        }

        TEST_METHOD(ReadUnquotedString_empty_withRemaining) {
            StringReader reader(" hello world");
            Assert::AreEqual(reader.ReadUnquotedString(), { "" });
            Assert::AreEqual(reader.GetRead(), { "" });
            Assert::AreEqual(reader.GetRemaining(), { " hello world" });
        }

        TEST_METHOD(ReadQuotedString) {
            StringReader reader("\"hello world\"");
            Assert::AreEqual(reader.ReadQuotedString(), { "hello world" });
            Assert::AreEqual(reader.GetRead(), { "\"hello world\"" });
            Assert::AreEqual(reader.GetRemaining(), { "" });
        }

        TEST_METHOD(ReadSingleQuotedString) {
            StringReader reader("'hello world'");
            Assert::AreEqual(reader.ReadQuotedString(), {"hello world"});
            Assert::AreEqual(reader.GetRead(), {"'hello world'"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadMixedQuotedString_doubleInsideSingle) {
            StringReader reader("'hello \"world\"'");
            Assert::AreEqual(reader.ReadQuotedString(), {"hello \"world\""});
            Assert::AreEqual(reader.GetRead(), {"'hello \"world\"'"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadMixedQuotedString_singleInsideDouble) {
            StringReader reader("\"hello 'world'\"");
            Assert::AreEqual(reader.ReadQuotedString(), {"hello 'world'"});
            Assert::AreEqual(reader.GetRead(), {"\"hello 'world'\""});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadQuotedString_empty) {
            StringReader reader("");
            Assert::AreEqual(reader.ReadQuotedString(), {""});
            Assert::AreEqual(reader.GetRead(), {""});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadQuotedString_emptyQuoted) {
            StringReader reader("\"\"");
            Assert::AreEqual(reader.ReadQuotedString(), {""});
            Assert::AreEqual(reader.GetRead(), {"\"\""});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadQuotedString_emptyQuoted_withRemaining) {
            StringReader reader("\"\" hello world");
            Assert::AreEqual(reader.ReadQuotedString(), {""});
            Assert::AreEqual(reader.GetRead(), {"\"\""});
            Assert::AreEqual(reader.GetRemaining(), {" hello world"});
        }

        TEST_METHOD(ReadQuotedString_withEscapedQuote) {
            StringReader reader("\"hello \\\"world\\\"\"");
            Assert::AreEqual(reader.ReadQuotedString(), {"hello \"world\""});
            Assert::AreEqual(reader.GetRead(), {"\"hello \\\"world\\\"\""});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadQuotedString_withEscapedEscapes) {
            StringReader reader("\"\\\\o/\"");
            Assert::AreEqual(reader.ReadQuotedString(), {"\\o/"});
            Assert::AreEqual(reader.GetRead(), {"\"\\\\o/\""});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadQuotedString_withRemaining) {
            StringReader reader("\"hello world\" foo bar");
            Assert::AreEqual(reader.ReadQuotedString(), {"hello world"});
            Assert::AreEqual(reader.GetRead(), {"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {" foo bar"});
        }

        TEST_METHOD(ReadQuotedString_withImmediateRemaining) {
            StringReader reader("\"hello world\"foo bar");
            Assert::AreEqual(reader.ReadQuotedString(), {"hello world"});
            Assert::AreEqual(reader.GetRead(), {"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {"foo bar"});
        }

        TEST_METHOD(ReadQuotedString_noOpen) {
            try {
                StringReader("hello world\"").ReadQuotedString();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadQuotedString_noClose) {
            try {
                StringReader("\"hello world").ReadQuotedString();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {12});
            }
        }

        TEST_METHOD(ReadQuotedString_invalidEscape) {
            try {
                StringReader("\"hello\\nworld\"").ReadQuotedString();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {7});
            }
        }

        TEST_METHOD(ReadQuotedString_invalidQuoteEscape) {
            try {
                StringReader("'hello\\\"\'world").ReadQuotedString();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {7});
            }
        }

        TEST_METHOD(ReadString_noQuotes) {
            StringReader reader("hello world");
            Assert::AreEqual(reader.ReadString(), {"hello"});
            Assert::AreEqual(reader.GetRead(), {"hello"});
            Assert::AreEqual(reader.GetRemaining(), {" world"});
        }

        TEST_METHOD(ReadString_singleQuotes) {
            StringReader reader("'hello world'");
            Assert::AreEqual(reader.ReadString(), {"hello world"});
            Assert::AreEqual(reader.GetRead(), {"'hello world'"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadString_doubleQuotes) {
            StringReader reader("\"hello world\"");
            Assert::AreEqual(reader.ReadString(), {"hello world"});
            Assert::AreEqual(reader.GetRead(), {"\"hello world\""});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadInt) {
            StringReader reader("1234567890");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadInt_negative) {
            StringReader reader("-1234567890");
            Assert::AreEqual(reader.ReadValue<int>(), {-1234567890});
            Assert::AreEqual(reader.GetRead(), {"-1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadInt_invalid) {
            try {
                StringReader("12.34").ReadValue<int>();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadInt_none) {
            try {
                StringReader("").ReadValue<int>();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadInt_withRemaining) {
            StringReader reader("1234567890 foo bar");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {" foo bar"});
        }

        TEST_METHOD(ReadInt_withRemainingImmediate) {
            StringReader reader("1234567890foo bar");
            Assert::AreEqual(reader.ReadValue<int>(), {1234567890});
            Assert::AreEqual(reader.GetRead(), {"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {"foo bar"});
        }

        TEST_METHOD(ReadLong) {
            StringReader reader("1234567890");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadLong_negative) {
            StringReader reader("-1234567890");
            Assert::AreEqual(reader.ReadValue<long>(), {-1234567890L});
            Assert::AreEqual(reader.GetRead(), {"-1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadLong_invalid) {
            try {
                StringReader("12.34").ReadValue<long>();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadLong_none) {
            try {
                StringReader("").ReadValue<long>();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadLong_withRemaining) {
            StringReader reader("1234567890 foo bar");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {" foo bar"});
        }

        TEST_METHOD(ReadLong_withRemainingImmediate) {
            StringReader reader("1234567890foo bar");
            Assert::AreEqual(reader.ReadValue<long>(), {1234567890L});
            Assert::AreEqual(reader.GetRead(), {"1234567890"});
            Assert::AreEqual(reader.GetRemaining(), {"foo bar"});
        }

        TEST_METHOD(ReadDouble) {
            StringReader reader("123");
            Assert::AreEqual(reader.ReadValue<double>(), {123.0});
            Assert::AreEqual(reader.GetRead(), {"123"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadDouble_withDecimal) {
            StringReader reader("12.34");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadDouble_negative) {
            StringReader reader("-123");
            Assert::AreEqual(reader.ReadValue<double>(), {-123.0});
            Assert::AreEqual(reader.GetRead(), {"-123"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadDouble_invalid) {
            try {
                StringReader("12.34.56").ReadValue<double>();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadDouble_none) {
            try {
                StringReader("").ReadValue<double>();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadDouble_withRemaining) {
            StringReader reader("12.34 foo bar");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {" foo bar"});
        }

        TEST_METHOD(ReadDouble_withRemainingImmediate) {
            StringReader reader("12.34foo bar");
            Assert::AreEqual(reader.ReadValue<double>(), {12.34});
            Assert::AreEqual(reader.GetRead(), {"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {"foo bar"});
        }

        TEST_METHOD(ReadFloat) {
            StringReader reader("123");
            Assert::AreEqual(reader.ReadValue<float>(), {123.0f});
            Assert::AreEqual(reader.GetRead(), {"123"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadFloat_withDecimal) {
            StringReader reader("12.34");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadFloat_negative) {
            StringReader reader("-123");
            Assert::AreEqual(reader.ReadValue<float>(), {-123.0f});
            Assert::AreEqual(reader.GetRead(), {"-123"});
            Assert::AreEqual(reader.GetRemaining(), {""});
        }

        TEST_METHOD(ReadFloat_invalid) {
            try {
                StringReader("12.34.56").ReadValue<float>();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadFloat_none) {
            try {
                StringReader("").ReadValue<float>();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadFloat_withRemaining) {
            StringReader reader("12.34 foo bar");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {" foo bar"});
        }

        TEST_METHOD(ReadFloat_withRemainingImmediate) {
            StringReader reader("12.34foo bar");
            Assert::AreEqual(reader.ReadValue<float>(), {12.34f});
            Assert::AreEqual(reader.GetRead(), {"12.34"});
            Assert::AreEqual(reader.GetRemaining(), {"foo bar"});
        }

        TEST_METHOD(expect_correct) {
            StringReader reader("abc");
            reader.Expect('a');
            Assert::AreEqual(reader.GetCursor(), {1});
        }

        TEST_METHOD(expect_incorrect) {
            StringReader reader("bcd");
            try {
                reader.Expect('a');
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(expect_none) {
            StringReader reader("");
            try {
                reader.Expect('a');
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadBoolean_correct) {
            StringReader reader("true");
            Assert::AreEqual(reader.ReadValue<bool>(), {true});
            Assert::AreEqual(reader.GetRead(), {"true"});
        }

        TEST_METHOD(ReadBoolean_incorrect) {
            StringReader reader("tuesday");
            try {
                reader.ReadValue<bool>();
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }

        TEST_METHOD(ReadBoolean_none) {
            StringReader reader("");
            try {
                reader.ReadValue<bool>();
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) {
                Assert::AreEqual(ex.GetCursor(), {0});
            }
        }
    };
}
