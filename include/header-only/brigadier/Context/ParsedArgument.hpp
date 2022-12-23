#pragma once

#include "StringRange.hpp"

namespace brigadier
{
    struct TypeInfo
    {
        TypeInfo(size_t hash) : hash(hash) {}
        template<typename ArgType>
        static constexpr size_t Create() { return (((uintptr_t)((void*)ArgType::GetTypeName().data())) + (sizeof(typename ArgType::type) << 24) + (sizeof(ArgType) << 8)); }
        inline bool operator==(TypeInfo const& other) { return hash == other.hash; }
        inline bool operator!=(TypeInfo const& other) { return hash != other.hash; }
        size_t hash = 0;
    };

    template<typename CharT, typename S>
    class BasicIParsedArgument
    {
    public:
        BasicIParsedArgument(size_t start, size_t end, TypeInfo typeInfo) : range(start, end), typeInfo(typeInfo) {}
        virtual ~BasicIParsedArgument() = default;

        inline StringRange GetRange()    const { return range; }
        inline TypeInfo                GetTypeInfo() const { return typeInfo; }
    protected:
        StringRange range;
        TypeInfo typeInfo;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(IParsedArgument);

    template<typename CharT, typename S, typename ArgType>
    class BasicParsedArgument : public BasicIParsedArgument<CharT, S>
    {
    public:
        using T = typename ArgType::type;

        BasicParsedArgument(size_t start, size_t end, T result) : BasicIParsedArgument<CharT, S>(start, end, TypeInfo(TypeInfo::Create<ArgType>())), result(std::move(result)) {}
        virtual ~BasicParsedArgument() = default;

        inline T&       GetResult()       { return result; }
        inline T const& GetResult() const { return result; }
    private:
        T result;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParsedArgument);
}
