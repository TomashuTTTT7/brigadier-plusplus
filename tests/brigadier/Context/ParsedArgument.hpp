#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(ParsedArgumentTest)
    {
        TEST_METHOD(TypeInfoTest)
        {
            std::set<TypeInfo> hashes;

            hashes.insert(TypeInfo::Create<char, String>());
            hashes.insert(TypeInfo::Create<char, Word>());
            hashes.insert(TypeInfo::Create<char, GreedyString>());
            hashes.insert(TypeInfo::Create<char, Bool>());
            hashes.insert(TypeInfo::Create<char, Char>());
            hashes.insert(TypeInfo::Create<char, Float>());
            hashes.insert(TypeInfo::Create<char, Double>());
            hashes.insert(TypeInfo::Create<char, Integer>());
            hashes.insert(TypeInfo::Create<char, Long>());

            // wide strings have different pointers, thus hashes are different
            hashes.insert(TypeInfo::Create<wchar_t, String>());
            hashes.insert(TypeInfo::Create<wchar_t, Word>());
            hashes.insert(TypeInfo::Create<wchar_t, GreedyString>());
            hashes.insert(TypeInfo::Create<wchar_t, Bool>());
            hashes.insert(TypeInfo::Create<wchar_t, Char>());
            hashes.insert(TypeInfo::Create<wchar_t, Float>());
            hashes.insert(TypeInfo::Create<wchar_t, Double>());
            hashes.insert(TypeInfo::Create<wchar_t, Integer>());
            hashes.insert(TypeInfo::Create<wchar_t, Long>());

            Assert::AreEqual(hashes.size(), {18});
        }
    };
}
