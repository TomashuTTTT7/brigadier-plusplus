#pragma once
#include "CommonTest.hpp"

namespace brigadier
{
    TEST_CLASS(ParsedArgumentTest)
    {
        TEST_METHOD(TypeInfoTest)
        {
            std::set<size_t> hashes;
            hashes.insert(TypeInfo::Create<String>());
            hashes.insert(TypeInfo::Create<Word>());
            hashes.insert(TypeInfo::Create<GreedyString>());
            hashes.insert(TypeInfo::Create<Bool>());
            hashes.insert(TypeInfo::Create<Char>());
            hashes.insert(TypeInfo::Create<Float>());
            hashes.insert(TypeInfo::Create<Double>());
            hashes.insert(TypeInfo::Create<Integer>());
            hashes.insert(TypeInfo::Create<Long>());

            Assert::AreEqual(hashes.size(), {9});
        }
    };
}
