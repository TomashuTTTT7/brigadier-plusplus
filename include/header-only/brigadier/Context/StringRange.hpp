#pragma once

#include <algorithm>
#include "../StringReader.hpp"

namespace brigadier
{
    template<typename CharT>
    class BasicStringRange
    {
    public:
        BasicStringRange(size_t start, size_t end) : start(start), end(end) {}

        inline int GetStart() const { return start; }
        inline int GetEnd()   const { return end;   }

        inline static BasicStringRange<CharT> At(int pos) { return BasicStringRange<CharT>(pos, pos); }
        inline static BasicStringRange<CharT> Between(int start, const int end) { return BasicStringRange<CharT>(start, end); }
        inline static BasicStringRange<CharT> Encompassing(BasicStringRange<CharT> const& a, BasicStringRange<CharT> const& b) {
            return BasicStringRange<CharT>((std::min)(a.GetStart(), b.GetStart()), (std::max)(a.GetEnd(), b.GetEnd()));
        }

        inline std::basic_string_view<CharT> Get(BasicStringReader<CharT> reader) const {
            return reader.GetString().substr(start, end - start);
        }
        inline std::basic_string_view<CharT> Get(std::basic_string_view<CharT> string) const {
            return string.substr(start, end - start);
        }

        inline bool      IsEmpty()   const { return start == end; }
        inline ptrdiff_t GetLength() const { return end - start;  }

        inline bool operator==(BasicStringRange<CharT> const& other) const { return (start == other.start && end == other.end); }
    private:
        size_t start = 0;
        size_t end = 0;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(StringRange);
}
