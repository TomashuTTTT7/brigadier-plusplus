#pragma once

#include <algorithm>
#include "../StringReader.hpp"

namespace brigadier
{
    class StringRange
    {
    public:
        StringRange(const int start, const int end) : start(start), end(end) {}

        inline int GetStart() const { return start; }
        inline int GetEnd()   const { return end;   }

        inline static StringRange At(int pos) { return StringRange(pos, pos); }
        inline static StringRange Between(int start, const int end) { return StringRange(start, end); }
        inline static StringRange Encompassing(StringRange const& a, StringRange const& b) {
            return StringRange((std::min)(a.GetStart(), b.GetStart()), (std::max)(a.GetEnd(), b.GetEnd()));
        }

        inline std::string_view Get(StringReader reader) const {
            return reader.GetString().substr(start, end - start);
        }
        inline std::string_view Get(std::string_view string) const {
            return string.substr(start, end - start);
        }

        inline bool IsEmpty()   const { return start == end; }
        inline int  GetLength() const { return end - start;  }

        inline bool operator==(StringRange const& other) const { return (start == other.start && end == other.end); }
    private:
        int start = 0;
        int end = 0;
    };
}
