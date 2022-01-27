#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(BoolArgumentTypeTest)
    {
        TEST_METHOD(parse)
        {
            StringReader readert("true");
            Assert::AreEqual(BoolArgumentType().Parse(readert), true);
            StringReader readerf("false");
            Assert::AreEqual(BoolArgumentType().Parse(readerf), false);
        }
    };


    TEST_CLASS(ArithmeticArgumentTypeTest)
    {
        TEST_METHOD(parse)
        {
            StringReader reader("15");
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
            StringReader reader("-5");
            try {
                Double(0, 100).Parse(reader);
                Assert::Fail();
            } catch (CommandSyntaxException const& ex) { Assert::AreEqual(ex.GetCursor(), 0); }
            try {
                Float(0, 100).Parse(reader);
                Assert::Fail();
            } catch (CommandSyntaxException const& ex) { Assert::AreEqual(ex.GetCursor(), 0); }
            try {
                Integer(0, 100).Parse(reader);
                Assert::Fail();
            } catch (CommandSyntaxException const& ex) { Assert::AreEqual(ex.GetCursor(), 0); }
            try {
                Long(0, 100).Parse(reader);
                Assert::Fail();
            } catch (CommandSyntaxException const& ex) { Assert::AreEqual(ex.GetCursor(), 0); }
        }

        TEST_METHOD(parse_tooBig)
        {
            StringReader reader("5");
            try {
                Double(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) { Assert::AreEqual(ex.GetCursor(), 0); }
            try {
                Float(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) { Assert::AreEqual(ex.GetCursor(), 0); }
            try {
                Integer(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) { Assert::AreEqual(ex.GetCursor(), 0); }
            try {
                Long(-100, 0).Parse(reader);
                Assert::Fail();
            }
            catch (CommandSyntaxException const& ex) { Assert::AreEqual(ex.GetCursor(), 0); }
        }
    };

    TEST_CLASS(StringArgumentTypeTest)
    {
        TEST_METHOD(testParseWord)
        {
            StringReader reader("hello");
            Assert::AreEqual(Word().Parse(reader), { "hello" });
        }

        TEST_METHOD(testParseString)
        {
            StringReader reader("\"hello world\"");
            Assert::AreEqual(String().Parse(reader), { "hello world" });
        }
        
        TEST_METHOD(testParseGreedyString)
        {
            StringReader reader("Hello world! This is a test.");
            Assert::AreEqual(GreedyString().Parse(reader), { "Hello world! This is a test." });
            Assert::AreEqual(reader.CanRead(), false);
        }
        
        TEST_METHOD(testEscapeIfRequired_notRequired)
        {
            Assert::AreEqual(String::EscapeIfRequired("hello"), { "hello" });
            Assert::AreEqual(String::EscapeIfRequired(""), { "" });
        }
        
        TEST_METHOD(testEscapeIfRequired_multipleWords)
        {
            Assert::AreEqual(String::EscapeIfRequired("hello world"), { "\"hello world\"" });
        }

        TEST_METHOD(testEscapeIfRequired_quote)
        {
            Assert::AreEqual(String::EscapeIfRequired("hello \"world\"!"), { "\"hello \\\"world\\\"!\"" });
        }
        
        TEST_METHOD(testEscapeIfRequired_escapes)
        {
            Assert::AreEqual(String::EscapeIfRequired("\\"), { "\"\\\\\"" });
        }

        TEST_METHOD(testEscapeIfRequired_singleQuote)
        {
            Assert::AreEqual(String::EscapeIfRequired("\""), { "\"\\\"\"" });
        }
    };
}
