#pragma once

namespace cologne
{
    struct LightHandle
    {
        uint32_t id = UINT32_MAX;
        bool is_valid() const {return id != UINT32_MAX;}
        friend bool operator==(const LightHandle &, const LightHandle &) = default;
    };
}

template<>
struct std::hash<cologne::LightHandle>
{
    size_t operator()(const cologne::LightHandle& handle) const noexcept
    {
        return hash<uint32_t>()(handle.id);
    }
};
