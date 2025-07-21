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

namespace nlohmann
{
    template<>
    struct adl_serializer<glm::vec2>
    {
        static void to_json(json& j, const glm::vec2& vec)
        {
            j = {vec.x, vec.y};
        }

        static void from_json(const json& j, glm::vec2& vec)
        {
            j.at(0).get_to(vec.x);
            j.at(1).get_to(vec.y);
        }
    };

    template<>
    struct adl_serializer<glm::vec3>
    {
        static void to_json(json& j, const glm::vec3& vec)
        {
            j = {vec.x, vec.y, vec.z};
        }

        static void from_json(const json& j, glm::vec3& vec)
        {
            j.at(0).get_to(vec.x);
            j.at(1).get_to(vec.y);
            j.at(2).get_to(vec.z);
        }
    };

    template<>
    struct adl_serializer<glm::vec4>
    {
        static void to_json(json& j, const glm::vec4& vec)
        {
            j = {vec.x, vec.y, vec.z, vec.w};
        }

        static void from_json(const json& j, glm::vec4& vec)
        {
            j.at(0).get_to(vec.x);
            j.at(1).get_to(vec.y);
            j.at(2).get_to(vec.z);
            j.at(3).get_to(vec.w);
        }
    };

    template<>
    struct adl_serializer<glm::quat>
    {
        static void to_json(json& j, const glm::quat& quat)
        {
            j = {quat.w, quat.x, quat.y, quat.z};
        }

        static void from_json(const json& j, glm::quat& quat)
        {
            j.at(0).get_to(quat.w);
            j.at(1).get_to(quat.x);
            j.at(2).get_to(quat.y);
            j.at(3).get_to(quat.z);
        }
    };

    template<>
    struct adl_serializer<glm::mat4>
    {
        static void to_json(json& j, const glm::mat4& mat)
        {
            j = {mat[0], mat[1], mat[2], mat[3]};
        }

        static void from_json(const json& j, glm::mat4& mat)
        {
            j.at(0).get_to(mat[0]);
            j.at(1).get_to(mat[1]);
            j.at(2).get_to(mat[2]);
            j.at(3).get_to(mat[3]);
        }
    };
}


namespace cologne
{
    SceneSaver::SceneSaver(Scene *scene)
    {
        _scene = scene;
    }

    static void serialize_entity(nlohmann::json& json, Entity entity)
    {
        json["Entity"] = static_cast<uint64_t>(entity.get_uuid());
        json["TagComponent"]["tag"] = entity.get_name();

        auto& tr = entity.get_transform();
        json["TransformComponent"]["position"] = tr.position;
        json["TransformComponent"]["rotation"] = tr.rotation;
        json["TransformComponent"]["scale"] = tr.scale;

        auto& wc = entity.get_component<WorldTransformComponent>();
        json["WorldTransformComponent"] = wc.transform;

        json["ActiveComponent"] = entity.is_active();

        if (entity.has_component<ParentComponent>())
        {
            auto& pc = entity.get_component<ParentComponent>();
            for (auto child : pc.children)
            {
                json["ParentComponent"].emplace_back(static_cast<uint64_t>(child));
            }
        }

        if (entity.has_component<ModelComponent>())
        {
            auto& mc = entity.get_component<ModelComponent>();
            json["ModelComponent"]["model_name"] = mc.model_name;
            json["ModelComponent"]["gi_only"] = mc.gi_only;
        }

        if (entity.has_component<MeshComponent>())
        {
            auto& mc = entity.get_component<MeshComponent>();
            json["MeshComponent"] = mc.mesh_name;
        }

        if (entity.has_component<SkinnedModelComponent>())
        {
            json["SkinnedModelComponent"] = entity.get_component<SkinnedModelComponent>().model_name;
        }

        //TODO: how to link static collider info? use another UUID? link with mesh?
        if (entity.has_component<StaticColliderComponent>())
        {
        }

        if (entity.has_component<CameraComponent>())
        {
            json["CameraComponent"]["fov_radians"] = entity.get_component<CameraComponent>().fov_radians;
            json["CameraComponent"]["primary"] = entity.get_component<CameraComponent>().primary;
        }

        //TODO: player component serialization
        if (entity.has_component<PlayerComponent>())
        {

        }

        //TODO: viewmodel serialization
        if (entity.has_component<ViewmodelComponent>())
        {

        }

        if (entity.has_component<EnemyComponent>())
        {
            auto& ec = entity.get_component<EnemyComponent>();
            json["EnemyComponent"]["health"] = ec.health;
            json["EnemyComponent"]["dead"] = ec.dead;
        }

        if (entity.has_component<BulletComponent>())
        {
            auto& bc = entity.get_component<BulletComponent>();
            json["BulletComponent"]["position"] = bc.position;
            json["BulletComponent"]["direction"] = bc.direction;
            json["BulletComponent"]["damage"] = bc.damage;
        }

        if (entity.has_component<LightComponent>())
        {
            auto& lc = entity.get_component<LightComponent>();
            json["LightComponent"]["color"] = lc.color;
            json["LightComponent"]["strength"] = lc.strength;
            json["LightComponent"]["radius"] = lc.radius;
            json["LightComponent"]["type"] = lc.type;
            json["LightComponent"]["outer_cutoff"] = lc.outer_cutoff;
            json["LightComponent"]["inner_cutoff"] = lc.inner_cutoff;
            json["LightComponent"]["cast_shadows"] = lc.cast_shadows;
        }

        //TODO: native script serialization
        if (entity.has_component<NativeScriptComponent>())
        {

        }
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
