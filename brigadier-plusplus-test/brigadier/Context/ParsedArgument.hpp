#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(ParsedArgumentTest)
    {
        TEST_METHOD(TypeInfoTest)
        {
            std::set<size_t> hashes;

            hashes.insert(TypeInfo::Create<String<char>>());
            hashes.insert(TypeInfo::Create<Word<char>>());
            hashes.insert(TypeInfo::Create<GreedyString<char>>());
            hashes.insert(TypeInfo::Create<Bool<char>>());
            hashes.insert(TypeInfo::Create<Char<char>>());
            hashes.insert(TypeInfo::Create<Float<char>>());
            hashes.insert(TypeInfo::Create<Double<char>>());
            hashes.insert(TypeInfo::Create<Integer<char>>());
            hashes.insert(TypeInfo::Create<Long<char>>());

            // wide strings have different pointers, thus hashes are different
            hashes.insert(TypeInfo::Create<String<wchar_t>>());
            hashes.insert(TypeInfo::Create<Word<wchar_t>>());
            hashes.insert(TypeInfo::Create<GreedyString<wchar_t>>());
            hashes.insert(TypeInfo::Create<Bool<wchar_t>>());
            hashes.insert(TypeInfo::Create<Char<wchar_t>>());
            hashes.insert(TypeInfo::Create<Float<wchar_t>>());
            hashes.insert(TypeInfo::Create<Double<wchar_t>>());
            hashes.insert(TypeInfo::Create<Integer<wchar_t>>());
            hashes.insert(TypeInfo::Create<Long<wchar_t>>());

            Assert::AreEqual(hashes.size(), {18});
        }
    };
}
