#pragma once
#include <entt/entt.hpp>

namespace cologne::ComponentRegistry
{
    enum  Traits : uint16_t
    {
        TRANSIENT = 1 << 0,
        NO_EDITOR = 1 << 1,
        EDITOR_READ_ONLY = 1 << 2,
        EDITOR_WRITE_ONLY = 1 << 3,
        EDITOR_READ_WRITE = EDITOR_READ_ONLY | EDITOR_WRITE_ONLY
    };

    // constexpr Traits operator|(Traits a, Traits b)
    // {
    //     return static_cast<Traits>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    // }
    using PropertiesMap = std::unordered_map<entt::id_type, entt::meta_any>;
    const std::map<entt::id_type, std::string>& get_component_map();
    void register_components();
}
