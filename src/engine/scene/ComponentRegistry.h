#pragma once
#include "Components.h"

namespace cologne
{
    struct TagComponent;
}


namespace cologne::ComponentRegistry
{
    using PropertiesMap = std::unordered_map<entt::id_type, entt::meta_any>;
    const std::vector<unsigned>& get_component_ids();
    void register_components();
}
