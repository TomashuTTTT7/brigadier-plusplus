#pragma once

#include "CppUnitTest.h"

#include <string_view>
#include <map>
#include <set>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace brigadier
{
    inline void AssertRange(StringRange const& a, StringRange const& b)
    {
        Assert::AreEqual(a.GetStart(), b.GetStart());
        Assert::AreEqual(a.GetEnd  (), b.GetEnd  ());
    }

    template<typename Container>
    inline void AssertArray(const Container& a, const Container& b)
    {
        Assert::AreEqual(a.size(), b.size());
        for (size_t i = 0; i < a.size(); i++) {
            Assert::AreEqual(a[i], b[i]);
        }
    }

    template<typename... Ts>
    inline void AssertMap(const std::map<Ts...>& a, const std::map<Ts...>& b)
    {
        Assert::AreEqual(a.size(), b.size());
        for (auto i1 = a.begin(), i2 = b.begin(); i1 != a.end() && i2 != b.end(); ++i1, ++i2) {
            if (i1->first != i2->first)  Assert::Fail();
            if (i1->second != i2->second) Assert::Fail();
        }
    }

    template<typename k, typename cmp = std::less<>>
    inline void AssertSet(const std::set<k, cmp>& a, const std::set<k, cmp>& b)
    {
        Assert::AreEqual(a.size(), b.size());
        for (auto i1 = a.begin(), i2 = b.begin(); i1 != a.end() && i2 != b.end(); ++i1, ++i2) {
            if (cmp()(*i1, *i2) || cmp()(*i2, *i1)) Assert::Fail();
        }
    }

    inline StringReader InputWithOffset(std::string_view input, int offset) {
        StringReader result(input);
        result.SetCursor(offset);
        return result;
    }

    inline void AssertSuggestion(Suggestion const& a, Suggestion const& b)
    {
        Assert::AreEqual(a.GetText(), b.GetText());
        Assert::AreEqual(a.GetTooltip(), b.GetTooltip());
        AssertRange(a.GetRange(), b.GetRange());
    }
}
