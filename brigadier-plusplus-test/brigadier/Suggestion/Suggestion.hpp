﻿#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(SuggestionTest)
    {
        TEST_METHOD(apply_insertation_start) {
            WSuggestion suggestion(StringRange::At(0), L"And so I said: ");
            Assert::AreEqual(suggestion.Apply(L"Hello world!"), { L"And so I said: Hello world!" });
        }


        TEST_METHOD(apply_insertation_middle) {
            WSuggestion suggestion(StringRange::At(6), L"small ");
            Assert::AreEqual(suggestion.Apply(L"Hello world!"), { L"Hello small world!" });
        }


        TEST_METHOD(apply_insertation_end) {
            WSuggestion suggestion(StringRange::At(5), L" world!");
            Assert::AreEqual(suggestion.Apply(L"Hello"), { L"Hello world!" });
        }


        TEST_METHOD(apply_replacement_start) {
            WSuggestion suggestion(StringRange::Between(0, 5), L"Goodbye");
            Assert::AreEqual(suggestion.Apply(L"Hello world!"), { L"Goodbye world!" });
        }


        TEST_METHOD(apply_replacement_middle) {
            WSuggestion suggestion(StringRange::Between(6, 11), L"Alex");
            Assert::AreEqual(suggestion.Apply(L"Hello world!"), { L"Hello Alex!" });
        }


        TEST_METHOD(apply_replacement_end) {
            WSuggestion suggestion(StringRange::Between(6, 12), L"Creeper!");
            Assert::AreEqual(suggestion.Apply(L"Hello world!"), { L"Hello Creeper!" });
        }


        TEST_METHOD(apply_replacement_everything) {
            WSuggestion suggestion(StringRange::Between(0, 12), L"Oh dear.");
            Assert::AreEqual(suggestion.Apply(L"Hello world!"), { L"Oh dear." });
        }


        TEST_METHOD(expand_unchanged) {
            WSuggestion suggestion(StringRange::At(1), L"oo");
            suggestion.Expand(L"f", StringRange::At(1));
            AssertSuggestion(suggestion, WSuggestion(StringRange::At(1), L"oo"));
        }


        TEST_METHOD(expand_left) {
            WSuggestion suggestion(StringRange::At(1), L"oo");
            suggestion.Expand(L"f", StringRange::Between(0, 1));
            AssertSuggestion(suggestion, WSuggestion(StringRange::Between(0, 1), L"foo"));
        }


        TEST_METHOD(expand_right) {
            WSuggestion suggestion(StringRange::At(0), L"minecraft:");
            suggestion.Expand(L"fish", StringRange::Between(0, 4));
            AssertSuggestion(suggestion, WSuggestion(StringRange::Between(0, 4), L"minecraft:fish"));
        }


        TEST_METHOD(expand_both) {
            WSuggestion suggestion(StringRange::At(11), L"minecraft:");
            suggestion.Expand(L"give Steve fish_block", StringRange::Between(5, 21));
            AssertSuggestion(suggestion, WSuggestion(StringRange::Between(5, 21), L"Steve minecraft:fish_block"));
        }


        TEST_METHOD(expand_replacement) {
            WSuggestion suggestion(StringRange::Between(6, 11), L"strangers");
            suggestion.Expand(L"Hello world!", StringRange::Between(0, 12));
            AssertSuggestion(suggestion, WSuggestion(StringRange::Between(0, 12), L"Hello strangers!"));
        }
    };
}
