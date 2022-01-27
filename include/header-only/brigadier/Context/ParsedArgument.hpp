#pragma once

#include "StringRange.hpp"

namespace brigadier
{
    struct TypeInfo
    {
        TypeInfo(size_t hash) : hash(hash) {}
        template<typename ArgType>
        static constexpr size_t Create() { return (((uintptr_t)((const char*)ArgType::GetTypeName().data())) + (sizeof(typename ArgType::type) << 24) + (sizeof(ArgType) << 8)); }
        inline bool operator==(TypeInfo const& other) { return hash == other.hash; }
        inline bool operator!=(TypeInfo const& other) { return hash != other.hash; }
        size_t hash = 0;
    };

    template<typename S>
    class IParsedArgument
    {
    public:
        IParsedArgument(int start, int end, TypeInfo typeInfo) : range(start, end), typeInfo(typeInfo) {}
        virtual ~IParsedArgument() = default;

        inline StringRange GetRange()    const { return range;    }
        inline TypeInfo    GetTypeInfo() const { return typeInfo; }
    protected:
        StringRange range;
        TypeInfo typeInfo;
    };

    template<typename S, typename ArgType>
    class ParsedArgument : public IParsedArgument<S>
    {
    public:
        using T = typename ArgType::type;

        ParsedArgument(const int start, const int end, T result) : IParsedArgument<S>(start, end, TypeInfo(TypeInfo::Create<ArgType>())), result(std::move(result)) {}
        virtual ~ParsedArgument() = default;

        inline T&       GetResult()       { return result; }
        inline T const& GetResult() const { return result; }
    private:
        T result;
    };
}
