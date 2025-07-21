//
// Created by alecpizz on 7/19/25.
//

#include "SceneSaver.h"
#include <engine/util/FileUtil.h>

#include "Components.h"
#include "Entity.h"
#include "Scene.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace cologne
{
    SceneSaver::SceneSaver(Scene *scene)
    {
        _scene = scene;
    }

    static void serialize_entity(nlohmann::json& json, Entity entity)
    {
        json["Entity"] = static_cast<uint32_t>(entity);
        json["TagComponent"]["Tag"] = entity.get_name();
    }

    void SceneSaver::serialize(const std::string &path)
    {
        if (!_scene)
        {
            LOG_ERROR("No scene to serialize!");
            return;
        }

        using json = nlohmann::json;

        json js;
        js["scene_name"] = "untitled_scene";
        for (auto e : _scene->_registry.view<entt::entity>())
        {
            Entity entity {e, _scene};
            json e_json;
            serialize_entity(e_json, entity);
            js["entities"].push_back(e_json);
        }
        FileUtil::create_directory_recursive(path);
        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("error opening file for serialization!");
            return;
        }
        file << js.dump(4);
        file.close();
    }

    void SceneSaver::deserialize(const std::string &path)
    {
    }

    void SceneSaver::serialize_runtime(const std::string &path)
    {
    }

    void SceneSaver::deserialize_runtime(const std::string &path)
    {
    }
}
