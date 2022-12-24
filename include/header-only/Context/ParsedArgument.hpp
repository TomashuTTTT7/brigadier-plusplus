#pragma once

#include "StringRange.hpp"

namespace brigadier
{
    struct TypeInfo
    {
        constexpr TypeInfo(void* typeName, size_t sizes) : typeName(typeName), sizes(sizes) {}
        template<typename CharT, typename ArgType>
        static constexpr TypeInfo Create() { return TypeInfo((void*)ArgType::template GetTypeName<CharT>().data(), (sizeof(typename ArgType::type) << 16) + sizeof(ArgType)); }
        template<typename CharT, template<typename> typename ArgType>
        inline static constexpr TypeInfo Create() { return Create<CharT, ArgType<CharT>>(); }
        inline bool operator==(TypeInfo const& other) { return (typeName == other.typeName) && (sizes == other.sizes); }
        inline bool operator!=(TypeInfo const& other) { return !(*this == other); }
        void* typeName = nullptr;
        size_t sizes = 0;
    };
    inline bool operator<(TypeInfo const& lhs, TypeInfo const& rhs) { return (lhs.typeName == rhs.typeName) ? lhs.sizes < rhs.sizes : lhs.typeName < rhs.typeName; }

    template<typename CharT, typename S>
    class IParsedArgument
    {
    public:
        IParsedArgument(size_t start, size_t end, TypeInfo typeInfo) : range(start, end), typeInfo(typeInfo) {}
        virtual ~IParsedArgument() = default;

        inline StringRange GetRange()    const { return range; }
        inline TypeInfo                GetTypeInfo() const { return typeInfo; }
    protected:
        StringRange range;
        TypeInfo typeInfo;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(IParsedArgument);

    template<typename CharT, typename S, typename ArgType>
    class ParsedArgument : public IParsedArgument<CharT, S>
    {
    public:
        using T = typename ArgType::type;

        ParsedArgument(size_t start, size_t end, T result) : IParsedArgument<CharT, S>(start, end, TypeInfo(TypeInfo::Create<CharT, ArgType>())), result(std::move(result)) {}
        virtual ~ParsedArgument() = default;

        inline T&       GetResult()       { return result; }
        inline T const& GetResult() const { return result; }
    private:
        T result;
    };
    BRIGADIER_SPECIALIZE_BASIC_TEMPL(ParsedArgument);
}
