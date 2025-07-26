//
// Created by alecpizz on 7/19/25.
//

#include "SceneSaver.h"
#include <engine/util/FileUtil.h>
#include <nlohmann/json.hpp>
#include "Components.h"
#include "Entity.h"
#include "Scene.h"
#include <fstream>
#include <engine/animation/AnimatorComponent.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/scripts/EditorCameraController.h>
#include <engine/scripts/PlayerController.h>

#include "ComponentRegistry.h"

namespace nlohmann
{
    template<>
    struct adl_serializer<glm::vec2>
    {
        static void to_json(json &j, const glm::vec2 &vec)
        {
            j = {vec.x, vec.y};
        }

        static void from_json(const json &j, glm::vec2 &vec)
        {
            j.at(0).get_to(vec.x);
            j.at(1).get_to(vec.y);
        }
    };

    template<>
    struct adl_serializer<glm::vec3>
    {
        static void to_json(json &j, const glm::vec3 &vec)
        {
            j = {vec.x, vec.y, vec.z};
        }

        static void from_json(const json &j, glm::vec3 &vec)
        {
            j.at(0).get_to(vec.x);
            j.at(1).get_to(vec.y);
            j.at(2).get_to(vec.z);
        }
    };

    template<>
    struct adl_serializer<glm::vec4>
    {
        static void to_json(json &j, const glm::vec4 &vec)
        {
            j = {vec.x, vec.y, vec.z, vec.w};
        }

        static void from_json(const json &j, glm::vec4 &vec)
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
        static void to_json(json &j, const glm::quat &quat)
        {
            j = {quat.w, quat.x, quat.y, quat.z};
        }

        static void from_json(const json &j, glm::quat &quat)
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
        static void to_json(json &j, const glm::mat4 &mat)
        {
            j = {mat[0], mat[1], mat[2], mat[3]};
        }

        static void from_json(const json &j, glm::mat4 &mat)
        {
            j.at(0).get_to(mat[0]);
            j.at(1).get_to(mat[1]);
            j.at(2).get_to(mat[2]);
            j.at(3).get_to(mat[3]);
        }
    };

    template<>
    struct adl_serializer<cologne::UUID>
    {
        static void to_json(json &j, const cologne::UUID &id)
        {
            j = id._uuid;
        }

        static void from_json(const json &j, cologne::UUID &id)
        {
            id = cologne::UUID(j.get<uint64_t>());
        }
    };
}

namespace cologne
{
    SceneSaver::SceneSaver(Scene *scene)
    {
        _scene = scene;
    }

    //
    // static void serialize_entity(nlohmann::json &json, Entity entity)
    // {
    //     json["Entity"] = static_cast<uint64_t>(entity.get_uuid());
    //     json["TagComponent"]["tag"] = entity.get_name();
    //
    //     auto &tr = entity.get_transform();
    //     json["TransformComponent"]["position"] = tr.position;
    //     json["TransformComponent"]["rotation"] = tr.rotation;
    //     json["TransformComponent"]["scale"] = tr.scale;
    //
    //     auto &wc = entity.get_component<WorldTransformComponent>();
    //     json["WorldTransformComponent"] = wc.transform;
    //
    //     json["ActiveComponent"] = entity.is_active();
    //
    //     if (entity.has_component<ParentComponent>())
    //     {
    //         auto &pc = entity.get_component<ParentComponent>();
    //         for (auto child: pc.children)
    //         {
    //             json["ParentComponent"].emplace_back(static_cast<uint64_t>(child));
    //         }
    //     }
    //
    //     if (entity.has_component<ChildComponent>())
    //     {
    //         auto &cc = entity.get_component<ChildComponent>();
    //         json["ChildComponent"] = static_cast<uint64_t>(cc.parent);
    //     }
    //
    //     if (entity.has_component<ModelComponent>())
    //     {
    //         auto &mc = entity.get_component<ModelComponent>();
    //         json["ModelComponent"]["model_name"] = mc.model_name;
    //         json["ModelComponent"]["gi_only"] = mc.gi_only;
    //     }
    //
    //     if (entity.has_component<MeshComponent>())
    //     {
    //         auto &mc = entity.get_component<MeshComponent>();
    //         json["MeshComponent"] = mc.mesh_name;
    //     }
    //
    //     if (entity.has_component<SkinnedModelComponent>())
    //     {
    //         json["SkinnedModelComponent"] = entity.get_component<SkinnedModelComponent>().model_name;
    //     }
    //
    //     //TODO: how to link static collider info? use another UUID? link with mesh?
    //     if (entity.has_component<StaticColliderComponent>())
    //     {
    //         json["StaticColliderComponent"]["mesh_name"] = entity.get_component<StaticColliderComponent>().mesh_name;
    //     }
    //
    //     if (entity.has_component<CameraComponent>())
    //     {
    //         json["CameraComponent"]["fov_radians"] = entity.get_component<CameraComponent>().fov_radians;
    //         json["CameraComponent"]["primary"] = entity.get_component<CameraComponent>().primary;
    //     }
    //
    //     if (entity.has_component<PlayerComponent>())
    //     {
    //         auto &pc = entity.get_component<PlayerComponent>();
    //         json["PlayerComponent"]["camera"] = static_cast<uint64_t>(pc.camera);
    //         json["PlayerComponent"]["viewmodel"] = static_cast<uint64_t>(pc.viewmodel);
    //         json["PlayerComponent"]["gravity"] = pc.gravity;
    //         json["PlayerComponent"]["move_speed"] = pc.move_speed;;
    //         json["PlayerComponent"]["run_acceleration"] = pc.run_acceleration;
    //         json["PlayerComponent"]["run_deceleration"] = pc.run_deceleration;
    //         json["PlayerComponent"]["air_acceleration"] = pc.air_acceleration;
    //         json["PlayerComponent"]["air_deceleration"] = pc.air_deceleration;
    //         json["PlayerComponent"]["air_control"] = pc.air_control;
    //         json["PlayerComponent"]["side_strafe_acceleration"] = pc.side_strafe_acceleration;;
    //         json["PlayerComponent"]["side_strafe_speed"] = pc.side_strafe_speed;
    //         json["PlayerComponent"]["jump_speed"] = pc.jump_speed;
    //         json["PlayerComponent"]["friction"] = pc.friction;
    //         json["PlayerComponent"]["maxStepVelocity"] = pc.maxStepVelocity;
    //         json["PlayerComponent"]["minStepVelocity"] = pc.minStepVelocity;
    //         json["PlayerComponent"]["minStepInterval"] = pc.minStepInterval;
    //         json["PlayerComponent"]["maxStepInterval"] = pc.maxStepInterval;
    //     }
    //
    //     if (entity.has_component<ViewmodelComponent>())
    //     {
    //         auto &vc = entity.get_component<ViewmodelComponent>();
    //         json["ViewmodelComponent"]["position_offset"] = vc.position_offset;
    //         json["ViewmodelComponent"]["euler_offset"] = vc.euler_offset;
    //         json["ViewmodelComponent"]["sway_multiplier"] = vc.sway_multiplier;
    //         json["ViewmodelComponent"]["smoothing"] = vc.smoothing;
    //         json["ViewmodelComponent"]["amplitude"] = vc.amplitude;
    //         json["ViewmodelComponent"]["frequency"] = vc.frequency;
    //         json["ViewmodelComponent"]["vertical_velocity_multiplier"] = vc.vertical_velocity_multiplier;
    //         json["ViewmodelComponent"]["max_vertical_offset"] = vc.max_vertical_offset;
    //     }
    //
    //     if (entity.has_component<EnemyComponent>())
    //     {
    //         auto &ec = entity.get_component<EnemyComponent>();
    //         json["EnemyComponent"]["health"] = ec.health;
    //         json["EnemyComponent"]["dead"] = ec.dead;
    //     }
    //
    //     if (entity.has_component<BulletComponent>())
    //     {
    //         auto &bc = entity.get_component<BulletComponent>();
    //         json["BulletComponent"]["position"] = bc.position;
    //         json["BulletComponent"]["direction"] = bc.direction;
    //         json["BulletComponent"]["damage"] = bc.damage;
    //     }
    //
    //     if (entity.has_component<LightComponent>())
    //     {
    //         auto &lc = entity.get_component<LightComponent>();
    //         json["LightComponent"]["color"] = lc.color;
    //         json["LightComponent"]["strength"] = lc.strength;
    //         json["LightComponent"]["radius"] = lc.radius;
    //         json["LightComponent"]["type"] = lc.type;
    //         json["LightComponent"]["outer_cutoff"] = lc.outer_cutoff;
    //         json["LightComponent"]["inner_cutoff"] = lc.inner_cutoff;
    //         json["LightComponent"]["cast_shadows"] = lc.cast_shadows;
    //     }
    //
    //     if (entity.has_component<NativeScriptComponent>()) //not going to use these for much longer imo
    //     {
    //         json["NativeScriptComponent"]["type_name"] = entity.get_component<NativeScriptComponent>().type_name;
    //     }
    //
    //     if (entity.has_component<AnimatorComponent>())
    //     {
    //         auto &ac = entity.get_component<AnimatorComponent>();
    //         json["AnimatorComponent"]["model_name"] = ac.get_model_base_name();
    //         json["AnimatorComponent"]["has_ragdoll"] = ac.get_ragdoll_id() != -1;
    //         if (ac.get_current_clip())
    //         {
    //             json["AnimatorComponent"]["current_clip"] = ac.get_current_clip()->get_name();
    //         }
    //         if (ac.get_base_clip())
    //         {
    //             json["AnimatorComponent"]["base_clip"] = ac.get_base_clip()->get_name();
    //         }
    //         json["AnimatorComponent"]["current_state"] = ac.get_current_state();
    //         json["AnimatorComponent"]["current_time"] = ac.get_current_progress();
    //     }
    // }
    //
    // static void deserialize_entity(const nlohmann::json &j_entity, Entity entity)
    // {
    //     auto &j_transform = j_entity["TransformComponent"];
    //     entity.get_transform().position = j_transform["position"].get<glm::vec3>();
    //     entity.get_transform().rotation = j_transform["rotation"].get<glm::quat>();
    //     entity.get_transform().scale = j_transform["scale"].get<glm::vec3>();
    //     entity.get_component<WorldTransformComponent>().transform = j_entity["WorldTransformComponent"].get<
    //         glm::mat4>();
    //     entity.get_component<ActiveComponent>().active = j_entity["ActiveComponent"].get<bool>();
    //
    //     if (j_entity.contains("ParentComponent"))
    //     {
    //         auto &pc = entity.add_component<ParentComponent>();
    //         for (auto &child: j_entity["ParentComponent"])
    //         {
    //             pc.children.emplace_back(child.get<uint64_t>());
    //         }
    //     }
    //
    //     if (j_entity.contains("ChildComponent"))
    //     {
    //         auto &cc = entity.add_component<ChildComponent>();
    //         cc.parent = j_entity["ChildComponent"].get<uint64_t>();
    //     }
    //
    //     if (j_entity.contains("ModelComponent"))
    //     {
    //         auto &mc = entity.add_component<ModelComponent>();
    //         mc.model_name = j_entity["ParentComponent"]["model_name"].get<std::string>();
    //         mc.gi_only = j_entity["ParentComponent"]["gi_only"].get<bool>();
    //     }
    //
    //     if (j_entity.contains("MeshComponent"))
    //     {
    //         auto &mc = entity.add_component<MeshComponent>();
    //         mc.mesh_name = j_entity["MeshComponent"].get<std::string>();
    //     }
    //
    //     if (j_entity.contains("SkinnedModelComponent"))
    //     {
    //         auto &sc = entity.add_component<SkinnedModelComponent>();
    //         sc.model_name = j_entity["SkinnedModelComponent"].get<std::string>();
    //     }
    //
    //     if (j_entity.contains("StaticColliderComponent"))
    //     {
    //         auto &cc = entity.add_component<StaticColliderComponent>();
    //         cc.mesh_name = j_entity["StaticColliderComponent"]["mesh_name"].get<std::string>();
    //         auto mesh = AssetManager::get_mesh_by_name(cc.mesh_name);
    //         if (mesh)
    //         {
    //             uint32_t body_id = Physics::create_static_mesh_collider(entity, entity.get_transform(), *mesh);
    //             cc.body_id = body_id;
    //         }
    //     }
    //
    //     if (j_entity.contains("CameraComponent"))
    //     {
    //         auto &cc = entity.add_component<CameraComponent>();
    //         cc.fov_radians = j_entity["CameraComponent"]["fov_radians"].get<float>();
    //         cc.primary = j_entity["CameraComponent"]["primary"].get<bool>();
    //     }
    //
    //     if (j_entity.contains("PlayerComponent"))
    //     {
    //         auto &pc = entity.add_component<PlayerComponent>();
    //         pc.camera = j_entity["PlayerComponent"]["camera"].get<uint64_t>();
    //         pc.viewmodel = j_entity["PlayerComponent"]["viewmodel"].get<uint64_t>();
    //         pc.gravity = j_entity["PlayerComponent"]["gravity"].get<float>();
    //         pc.move_speed = j_entity["PlayerComponent"]["move_speed"].get<float>();
    //         pc.run_acceleration = j_entity["PlayerComponent"]["run_acceleration"].get<float>();
    //         pc.run_deceleration = j_entity["PlayerComponent"]["run_deceleration"].get<float>();
    //         pc.air_acceleration = j_entity["PlayerComponent"]["air_acceleration"].get<float>();
    //         pc.air_deceleration = j_entity["PlayerComponent"]["air_deceleration"].get<float>();
    //         pc.air_control = j_entity["PlayerComponent"]["air_control"].get<float>();
    //         pc.side_strafe_acceleration = j_entity["PlayerComponent"]["side_strafe_acceleration"].get<float>();
    //         pc.side_strafe_speed = j_entity["PlayerComponent"]["side_strafe_speed"].get<float>();
    //         pc.jump_speed = j_entity["PlayerComponent"]["jump_speed"].get<float>();
    //         pc.friction = j_entity["PlayerComponent"]["friction"].get<float>();
    //         pc.maxStepVelocity = j_entity["PlayerComponent"]["maxStepVelocity"].get<float>();
    //         pc.minStepVelocity = j_entity["PlayerComponent"]["minStepVelocity"].get<float>();
    //         pc.minStepInterval = j_entity["PlayerComponent"]["minStepInterval"].get<float>();
    //         pc.maxStepInterval = j_entity["PlayerComponent"]["maxStepInterval"].get<float>();
    //         PlayerCreateInfo info;
    //         info.position = entity.get_transform().position;
    //         pc.id = Physics::create_player(info);
    //     }
    //
    //     if (j_entity.contains("ViewmodelComponent"))
    //     {
    //         auto &vc = entity.add_component<ViewmodelComponent>();
    //         vc.position_offset = j_entity["ViewmodelComponent"]["position_offset"].get<glm::vec3>();
    //         vc.euler_offset = j_entity["ViewmodelComponent"]["euler_offset"].get<glm::vec3>();
    //         vc.sway_multiplier = j_entity["ViewmodelComponent"]["sway_multiplier"].get<float>();
    //         vc.smoothing = j_entity["ViewmodelComponent"]["smoothing"].get<float>();
    //         vc.amplitude = j_entity["ViewmodelComponent"]["amplitude"].get<float>();
    //         vc.frequency = j_entity["ViewmodelComponent"]["frequency"].get<float>();
    //         vc.vertical_velocity_multiplier = j_entity["ViewmodelComponent"]["vertical_velocity_multiplier"].get<
    //             float>();
    //         vc.max_vertical_offset = j_entity["ViewmodelComponent"]["max_vertical_offset"].get<float>();
    //     }
    //
    //     if (j_entity.contains("EnemyComponent"))
    //     {
    //         auto &ec = entity.add_component<EnemyComponent>();
    //         ec.health = j_entity["EnemyComponent"]["health"].get<float>();
    //         ec.dead = j_entity["EnemyComponent"]["dead"].get<bool>();
    //         ec.hurt_sound = RESOURCES_PATH "sounds/enemy_hurt.mp3";
    //     }
    //
    //     if (j_entity.contains("BulletComponent"))
    //     {
    //         auto &bc = entity.add_component<BulletComponent>();
    //         bc.position = j_entity["BulletComponent"]["position"].get<glm::vec3>();
    //         bc.direction = j_entity["BulletComponent"]["direction"].get<glm::vec3>();
    //         bc.damage = j_entity["BulletComponent"]["damage"].get<float>();
    //     }
    //
    //     if (j_entity.contains("LightComponent"))
    //     {
    //         auto &lc = entity.add_component<LightComponent>();
    //         lc.color = j_entity["LightComponent"]["color"].get<glm::vec3>();
    //         lc.strength = j_entity["LightComponent"]["strength"].get<float>();
    //         lc.radius = j_entity["LightComponent"]["radius"].get<float>();
    //         lc.type = j_entity["LightComponent"]["type"].get<int>();
    //         lc.outer_cutoff = j_entity["LightComponent"]["outer_cutoff"].get<float>();
    //         lc.inner_cutoff = j_entity["LightComponent"]["inner_cutoff"].get<float>();
    //         lc.cast_shadows = j_entity["LightComponent"]["cast_shadows"].get<bool>();
    //     }
    //
    //     if (j_entity.contains("NativeScriptComponent"))
    //     {
    //         std::string type_name = j_entity["NativeScriptComponent"]["type_name"].get<std::string>();
    //         if (type_name == "PlayerController")
    //         {
    //             entity.add_component<NativeScriptComponent>().bind<PlayerController>();
    //         }
    //         else if (type_name == "EditorCameraController")
    //         {
    //             entity.add_component<NativeScriptComponent>().bind<EditorCameraController>();
    //         }
    //     }
    //
    //     //TODO: de-serialize animator
    //     if (j_entity.contains("AnimatorComponent"))
    //     {
    //         auto &ac = entity.add_component<AnimatorComponent>(j_entity["AnimatorComponent"]["model_name"]);
    //         if (j_entity["AnimatorComponent"]["has_ragdoll"].get<bool>())
    //         {
    //             //create the ragdoll!
    //             ac.create_ragdoll(entity);
    //         }
    //         if (j_entity["AnimatorComponent"].contains("current_clip"))
    //         {
    //             auto clip = AssetManager::get_animation_by_name(j_entity["AnimatorComponent"]["current_clip"]);
    //             if (clip)
    //             {
    //                 ac.play_one_shot_animation(clip);
    //             }
    //         }
    //
    //         if (j_entity["AnimatorComponent"].contains("base_clip"))
    //         {
    //             auto clip = AssetManager::get_animation_by_name(j_entity["AnimatorComponent"]["base_clip"]);
    //             if (clip)
    //             {
    //                 ac.play_base_animation(clip);
    //             }
    //         }
    //
    //         if (j_entity["AnimatorComponent"]["current_state"] == AnimatorComponent::State::RAGDOLLING)
    //         {
    //             ac.to_kinematic();
    //         }
    //
    //         ac.set_current_progress(j_entity["AnimatorComponent"]["current_time"].get<float>());
    //     }
    // }

    template<typename T>
    void save_property(nlohmann::json& j, const std::string& member_name, entt::meta_any& any)
    {
        T value = any.cast<T>();
        if (member_name.empty())
        {
            j.emplace_back(value);
        }
        else
        {
            j[member_name] = value;
        }
    }

    void save_component(entt::meta_any instance, nlohmann::json &j)
    {
        using namespace entt::literals;
        for (auto [id, data]: instance.type().data())
        {
            auto hash = data.type().info().hash();
            auto property_any = data.get(instance);
            std::string member_name;
            if (auto *mp = static_cast<const ComponentRegistry::PropertiesMap *>(data.custom()))
            {
                if (auto it = mp->find("name"_hs); it != mp->end())
                {
                    member_name = *it->second.try_cast<const char *>();
                }
            }
            if (hash == entt::type_hash<glm::vec3>::value())
            {
                save_property<glm::vec3>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<glm::vec4>::value())
            {
                save_property<glm::vec4>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<glm::quat>::value())
            {
                save_property<glm::quat>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<cologne::UUID>::value())
            {
                save_property<UUID>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<glm::mat4>::value())
            {
                save_property<glm::mat4>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<float>::value())
            {
                save_property<float>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<bool>::value())
            {
                save_property<bool>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<std::string>::value())
            {
                save_property<std::string>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<int>::value())
            {
                save_property<int>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<uint64_t>::value())
            {
                save_property<uint64_t>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<uint32_t>::value())
            {
                save_property<uint32_t>(j, member_name, property_any);
            }
            else if (data.type().is_sequence_container())
            {
                nlohmann::json array_node = nlohmann::json::array();
                if (auto view = data.get(instance).as_sequence_container(); view)
                {
                    for (auto meta_any : view)
                    {
                        save_component(meta_any.as_ref(), array_node);
                    }
                }
                j[member_name] = array_node;
            }
            else
            {
                nlohmann::json object_node;
                save_component(data.get(instance).as_ref(), object_node);
                j[member_name] = object_node;
            }
        }
    }

    void load_property(const nlohmann::json& j, entt::meta_data& meta_data, entt::meta_any instance)
    {
        auto hash = meta_data.type().info().hash();
        if (hash == entt::type_hash<glm::vec3>::value())
        {
            if (!meta_data.set(instance, j.get<glm::vec3>()))
            {
                LOG_ERROR("couldn't set meta data");
            }
        }
        else if (hash == entt::type_hash<glm::vec4>::value())
        {
            meta_data.set(instance, j.get<glm::vec4>());
        }
        else if (hash == entt::type_hash<glm::quat>::value())
        {
            meta_data.set(instance, j.get<glm::quat>());
        }
        else if (hash == entt::type_hash<UUID>::value())
        {
            meta_data.set(instance, j.get<UUID>());
        }
        else if (hash == entt::type_hash<glm::mat4>::value())
        {
            meta_data.set(instance, j.get<glm::mat4>());
        }
        else if (hash == entt::type_hash<float>::value())
        {
            meta_data.set(instance, j.get<float>());
        }
        else if (hash == entt::type_hash<bool>::value())
        {
            meta_data.set(instance, j.get<bool>());
        }
        else if (hash == entt::type_hash<std::string>::value())
        {
            meta_data.set(instance, j.get<std::string>());
        }
        else if (hash == entt::type_hash<int>::value())
        {
            meta_data.set(instance, j.get<int>());
        }
        else if (hash == entt::type_hash<uint64_t>::value())
        {
            meta_data.set(instance, j.get<uint64_t>());
        }
        else if (meta_data.type().is_sequence_container())
        {
            if (j.is_array())
            {
                auto container_view = meta_data.get(instance).as_ref().as_sequence_container();
                container_view.resize(j.size());
                for (size_t i = 0; i < j.size(); i++)
                {
                    for (auto [id, data] : container_view.value_type().data())
                    {
                        load_property(j[i], data, container_view[i].as_ref());
                    }
                }
            }
        }
        else
        {
            LOG_INFO("TRYING TO RECURSIVELY LOAD UNKNOWN TYPE %s!", std::string(meta_data.type().info().name()).c_str());
            load_property(j, meta_data, meta_data.get(instance).as_ref());
        }
    }

    void load_component(const nlohmann::json &j, entt::meta_any &instance)
    {
        using namespace entt::literals;
        for (auto [id, data]: instance.type().data())
        {
            if (auto *mp = static_cast<const ComponentRegistry::PropertiesMap *>(data.custom()))
            {
                if (auto it = mp->find("name"_hs); it != mp->end())
                {
                    std::string member_name = *it->second.try_cast<const char *>();
                    if (j.contains(member_name))
                    {
                        load_property(j.at(member_name), data, instance.as_ref());
                    }
                }
            }
        }
    }

    void emplace_component(entt::registry& registry, entt::entity entity, entt::meta_any& any)
    {
        using namespace entt::literals;
        if (auto emplace_func = any.type().func("emplace"_hs); emplace_func)
        {
            emplace_func.invoke({}, &registry, entity, any.as_ref());
        }
        else
        {
            LOG_WARN("NO EMPLACE FUNCTION FOR THE COMPONENT, WILL BE SKIPPED!");
        }
    }

    void SceneSaver::serialize(const std::string &path)
    {
        using namespace entt::literals;
        if (!_scene)
        {
            LOG_ERROR("No scene to serialize!");
            return;
        }

        nlohmann::json j_scene;
        j_scene["scene_name"] = "untitled_scene";
        nlohmann::json j_entities;
        _scene->_registry.view<entt::entity>().each([&](auto entity)
        {
            nlohmann::json j_entity;
            nlohmann::json j_components = nlohmann::json::object();
            for (auto [id, set]: _scene->_registry.storage())
            {
                if (!set.contains(entity))
                {
                    continue;
                }
                if (auto meta = entt::resolve(id))
                {
                    nlohmann::json j_component_data;
                    entt::meta_any instance = meta.from_void(set.value(entity));
                    save_component(instance, j_component_data);
                    j_components[std::string(set.type().name())] = j_component_data;
                }
            }
            j_entity["components"] = j_components;
            j_entities.push_back(j_entity);
        });

        j_scene["entities"] = j_entities;

        FileUtil::create_directory_recursive(path);
        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("error opening file for serialization!");
            return;
        }
        file << j_scene.dump(4);
        file.close();
    }

    void copy_component_meta_data(entt::meta_any& src, entt::meta_any& dest)
    {
        for (auto [i, data] : src.type().data())
        {
            data.set(dest, data.get(src));
        }
    }

    void SceneSaver::deserialize(const std::string &path)
    {
        if (!FileUtil::file_exists(path))
        {
            LOG_ERROR("No file at %s", path.c_str());
            return;
        }

        std::ifstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("Error opening file at %s", path.c_str());
            return;
        }
        using json = nlohmann::json;
        json data = json::parse(file);
        std::string scene_name = data["scene_name"];
        json entities = data["entities"];
        for (const auto &entity: entities)
        {
            entt::entity e = _scene->_registry.create();
            for (auto &[type_name, j_component_data]: entity.at("components").items())
            {
                if (auto meta_type = entt::resolve(entt::hashed_string(type_name.c_str())))
                {
                    auto new_component = meta_type.construct();
                    load_component(j_component_data, new_component);
                    emplace_component(_scene->_registry, e, new_component);
                }
            }
        }
        file.close();
    }

    void SceneSaver::serialize_runtime(const std::string &path)
    {
    }

    void SceneSaver::deserialize_runtime(const std::string &path)
    {
    }
}
