#pragma once

#include <algorithm>
#include "../StringReader.hpp"

namespace brigadier
{
    class StringRange
    {
    public:
        StringRange(size_t start, size_t end) : start(start), end(end) {}

        inline size_t GetStart() const { return start; }
        inline size_t GetEnd()   const { return end;   }

        inline static StringRange At(size_t pos) { return StringRange(pos, pos); }
        inline static StringRange Between(size_t start, size_t end) { return StringRange(start, end); }
        inline static StringRange Encompassing(StringRange const& a, StringRange const& b) {
            return StringRange((std::min)(a.GetStart(), b.GetStart()), (std::max)(a.GetEnd(), b.GetEnd()));
        }

        template<typename CharT>
        inline std::basic_string_view<CharT> Get(StringReader<CharT> reader) const {
            return reader.GetString().substr(start, end - start);
        }
        template<typename CharT>
        inline std::basic_string_view<CharT> Get(std::basic_string_view<CharT> string) const {
            return string.substr(start, end - start);
        }

        inline bool      IsEmpty()   const { return start == end; }
        inline ptrdiff_t GetLength() const { return end - start;  }

        inline bool operator==(StringRange const& other) const { return (start == other.start && end == other.end); }
    private:
        size_t start = 0;
        size_t end = 0;
    };
}
