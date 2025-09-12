//
// Created by alecpizz on 7/19/25.
//
#pragma once
#include <nlohmann/json_fwd.hpp>

#include "Entity.h"

namespace cologne
{
    class Scene;
}

namespace cologne
    {
    class SceneSaver
    {
    public:
        SceneSaver(Scene* scene);
        void serialize(const std::string& path);
        void deserialize(const std::string& path);
        void serialize_runtime(const std::string& path);
        void deserialize_runtime(const std::string& path);
        void serialize_entity(nlohmann::json& j, Entity entity);
        Entity deserialize_entity(nlohmann::json& j, std::unordered_map<UUID, UUID>& old_to_new_uuid_map);
    private:
        Scene* _scene;
    };
}
