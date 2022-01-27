#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(SuggestionTest)
    {
        TEST_METHOD(apply_insertation_start) {
            Suggestion suggestion(StringRange::At(0), "And so I said: ");
            Assert::AreEqual(suggestion.Apply("Hello world!"), { "And so I said: Hello world!" });
        }


        TEST_METHOD(apply_insertation_middle) {
            Suggestion suggestion(StringRange::At(6), "small ");
            Assert::AreEqual(suggestion.Apply("Hello world!"), { "Hello small world!" });
        }


        TEST_METHOD(apply_insertation_end) {
            Suggestion suggestion(StringRange::At(5), " world!");
            Assert::AreEqual(suggestion.Apply("Hello"), { "Hello world!" });
        }


        TEST_METHOD(apply_replacement_start) {
            Suggestion suggestion(StringRange::Between(0, 5), "Goodbye");
            Assert::AreEqual(suggestion.Apply("Hello world!"), { "Goodbye world!" });
        }


        TEST_METHOD(apply_replacement_middle) {
            Suggestion suggestion(StringRange::Between(6, 11), "Alex");
            Assert::AreEqual(suggestion.Apply("Hello world!"), { "Hello Alex!" });
        }


        TEST_METHOD(apply_replacement_end) {
            Suggestion suggestion(StringRange::Between(6, 12), "Creeper!");
            Assert::AreEqual(suggestion.Apply("Hello world!"), { "Hello Creeper!" });
        }


        TEST_METHOD(apply_replacement_everything) {
            Suggestion suggestion(StringRange::Between(0, 12), "Oh dear.");
            Assert::AreEqual(suggestion.Apply("Hello world!"), { "Oh dear." });
        }


        TEST_METHOD(expand_unchanged) {
            Suggestion suggestion(StringRange::At(1), "oo");
            suggestion.Expand("f", StringRange::At(1));
            AssertSuggestion(suggestion, Suggestion(StringRange::At(1), "oo"));
        }


        TEST_METHOD(expand_left) {
            Suggestion suggestion(StringRange::At(1), "oo");
            suggestion.Expand("f", StringRange::Between(0, 1));
            AssertSuggestion(suggestion, Suggestion(StringRange::Between(0, 1), "foo"));
        }


        TEST_METHOD(expand_right) {
            Suggestion suggestion(StringRange::At(0), "minecraft:");
            suggestion.Expand("fish", StringRange::Between(0, 4));
            AssertSuggestion(suggestion, Suggestion(StringRange::Between(0, 4), "minecraft:fish"));
        }


        TEST_METHOD(expand_both) {
            Suggestion suggestion(StringRange::At(11), "minecraft:");
            suggestion.Expand("give Steve fish_block", StringRange::Between(5, 21));
            AssertSuggestion(suggestion, Suggestion(StringRange::Between(5, 21), "Steve minecraft:fish_block"));
        }


        TEST_METHOD(expand_replacement) {
            Suggestion suggestion(StringRange::Between(6, 11), "strangers");
            suggestion.Expand("Hello world!", StringRange::Between(0, 12));
            AssertSuggestion(suggestion, Suggestion(StringRange::Between(0, 12), "Hello strangers!"));
        }
    };
}
