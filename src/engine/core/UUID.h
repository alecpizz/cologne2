//
// Created by alecpizz on 7/21/25.
//
#pragma once

namespace cologne
{
    class UUID
    {
    public:
        UUID();
        UUID(uint64_t id);
        UUID(const UUID&) = default;
        operator uint64_t() const {return _uuid;}
    private:
        uint64_t _uuid;
    };
}


namespace std
{
    template<typename T> struct hash;

    template<>
    struct hash<cologne::UUID>
    {
        std::size_t operator()(const cologne::UUID& uuid) const
        {
            return (uint64_t)uuid;
        }
    };
}
