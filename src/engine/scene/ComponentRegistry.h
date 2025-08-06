#pragma once
#include "Components.h"

namespace cologne
{
    struct TagComponent;
}


namespace cologne::ComponentRegistry
{
    using PropertiesMap = std::unordered_map<entt::id_type, entt::meta_any>;
    const std::map<entt::id_type, std::string>& get_component_map();
    void register_components();
}
