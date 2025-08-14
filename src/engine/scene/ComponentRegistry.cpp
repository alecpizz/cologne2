//
// Created by alecpizz on 7/21/25.
//
#include "ComponentRegistry.h"

#include <engine/animation/AnimatorComponent.h>
#include <engine/asset_manager/AssetManager.h>
#include <nlohmann/json.hpp>
#include "Components.h"


namespace cologne::ComponentRegistry
{
    using namespace entt::literals;
#define REGISTER_COMPONENT(T, name) \
component_type_map[entt::type_hash<T>::value()] = name; \
entt::meta_factory<T>() \
.type(entt::hashed_string(name))\
.func<[](entt::registry* registry, entt::entity entity, T& value) { \
        registry->emplace_or_replace<T>(entity, std::move(value)); }>("emplace"_hs)
#define REGISTER_PROPERTY(Type, member, ...) \
.data<&Type::member, entt::as_ref_t>(#member##_hs) \
.custom<PropertiesMap>(PropertiesMap{{"name"_hs, #member} __VA_OPT__(, __VA_ARGS__)})

#define REFLECT_ENUM(T) \
        entt::meta_factory<T>()
#define ENUMERATOR(E, Member, ...) \
        .data<E::Member>(#Member##_hs) \
        .custom<PropertiesMap>(PropertiesMap{{"name"_hs, #Member} __VA_OPT__(, __VA_ARGS__)})

    static std::map<entt::id_type, std::string> component_type_map;

    void serialize_animator(const AnimatorComponent &comp, nlohmann::json &j)
    {
        j["model_name"] = comp.get_model_base_name();
        j["has_ragdoll"] = comp.get_ragdoll_id() != -1;
        if (comp.get_current_clip())
        {
            j["current_clip"] = comp.get_current_clip()->get_name();
        }
        if (comp.get_base_clip())
        {
            j["base_clip"] = comp.get_base_clip()->get_name();
        }
        j["current_state"] = comp.get_current_state();
        j["current_time"] = comp.get_current_progress();
    }

    void deserialize_animator(AnimatorComponent &animator_component, const nlohmann::json &j)
    {
        AnimatorComponent temp(j["model_name"]);
        if (j["has_ragdoll"].get<bool>())
        {
            temp.set_has_ragdoll(true);
        }
        if (j.contains("current_clip"))
        {
            auto clip = AssetManager::get_animation_by_name(j["current_clip"]);
            if (clip)
            {
                temp.play_one_shot_animation(clip);
            }
        }
        if (j.contains("base_clip"))
        {
            auto clip = AssetManager::get_animation_by_name(j["base_clip"]);
            if (clip)
            {
                temp.play_base_animation(clip);
            }
        }
        if (j["current_state"] == AnimatorComponent::State::RAGDOLLING)
        {
            temp.to_ragdoll();
        }
        temp.set_current_progress(j["current_time"].get<float>());
        animator_component = temp;
    }

    const std::map<entt::id_type, std::string> &get_component_map()
    {
        return component_type_map;
    }

    void register_components()
    {
        using namespace entt::literals;
        entt::meta_factory<glm::vec3>()
                .data<&glm::vec3::x>("x"_hs)
                .data<&glm::vec3::y>("y"_hs)
                .data<&glm::vec3::z>("z"_hs);
        entt::meta_factory<glm::quat>()
                .data<&glm::quat::w>("w"_hs)
                .data<&glm::quat::x>("x"_hs)
                .data<&glm::quat::y>("y"_hs)
                .data<&glm::quat::z>("z"_hs);
        entt::meta_factory<glm::vec4>()
                .data<&glm::vec4::w>("w"_hs)
                .data<&glm::vec4::x>("x"_hs)
                .data<&glm::vec4::y>("y"_hs)
                .data<&glm::vec4::z>("z"_hs);

        entt::meta_factory<glm::mat4>()
                .data<[](glm::mat4 &m) { return m[0]; }>("c0"_hs)
                .data<[](glm::mat4 &m) { return m[1]; }>("c1"_hs)
                .data<[](glm::mat4 &m) { return m[2]; }>("c2"_hs)
                .data<[](glm::mat4 &m) { return m[3]; }>("c3"_hs);

        REGISTER_COMPONENT(TransformComponent, "TransformComponent")
                REGISTER_PROPERTY(TransformComponent, position)
                REGISTER_PROPERTY(TransformComponent, rotation)
                REGISTER_PROPERTY(TransformComponent, scale);

        entt::meta_factory<UUID>().conv<uint64_t>()
                .type("UUID"_hs)
                REGISTER_PROPERTY(UUID, _uuid);
        REGISTER_COMPONENT(IDComponent, "IDComponent")
                REGISTER_PROPERTY(IDComponent, id);

        REGISTER_COMPONENT(WorldTransformComponent, "WorldTransformComponent")
                REGISTER_PROPERTY(WorldTransformComponent, transform);

        REGISTER_COMPONENT(ChildComponent, "ChildComponent")
                REGISTER_PROPERTY(ChildComponent, parent);

        REGISTER_COMPONENT(ActiveComponent, "ActiveComponent")
                REGISTER_PROPERTY(ActiveComponent, active);

        REGISTER_COMPONENT(ModelComponent, "ModelComponent")
                REGISTER_PROPERTY(ModelComponent, model_name)
                REGISTER_PROPERTY(ModelComponent, gi_only);

        REGISTER_COMPONENT(MeshComponent, "MeshComponent")
                REGISTER_PROPERTY(MeshComponent, mesh_name);

        REGISTER_COMPONENT(SkinnedModelComponent, "SkinnedModelComponent")
                REGISTER_PROPERTY(SkinnedModelComponent, model_name);

        REGISTER_COMPONENT(CameraComponent, "CameraComponent")
                REGISTER_PROPERTY(CameraComponent, fov_radians)
                REGISTER_PROPERTY(CameraComponent, primary)
                REGISTER_PROPERTY(CameraComponent, ortho_zoom)
                REGISTER_PROPERTY(CameraComponent, orthographic);

        REGISTER_COMPONENT(TagComponent, "TagComponent")
                REGISTER_PROPERTY(TagComponent, tag);

        REGISTER_COMPONENT(StaticColliderComponent, "StaticColliderComponent")
                REGISTER_PROPERTY(StaticColliderComponent, mesh_name);

        REGISTER_COMPONENT(RigidbodyComponent, "RigidbodyComponent");

        REGISTER_COMPONENT(ConvexMeshColliderComponent, "ConvexMeshColliderComponent")
                REGISTER_PROPERTY(ConvexMeshColliderComponent, mesh_name);

        REGISTER_COMPONENT(PlayerComponent, "PlayerComponent")
                REGISTER_PROPERTY(PlayerComponent, camera)
                REGISTER_PROPERTY(PlayerComponent, viewmodel)
                REGISTER_PROPERTY(PlayerComponent, gravity)
                REGISTER_PROPERTY(PlayerComponent, move_speed)
                REGISTER_PROPERTY(PlayerComponent, run_acceleration)
                REGISTER_PROPERTY(PlayerComponent, run_deceleration)
                REGISTER_PROPERTY(PlayerComponent, air_acceleration)
                REGISTER_PROPERTY(PlayerComponent, air_deceleration)
                REGISTER_PROPERTY(PlayerComponent, air_control)
                REGISTER_PROPERTY(PlayerComponent, side_strafe_acceleration)
                REGISTER_PROPERTY(PlayerComponent, side_strafe_speed)
                REGISTER_PROPERTY(PlayerComponent, jump_speed)
                REGISTER_PROPERTY(PlayerComponent, friction)
                REGISTER_PROPERTY(PlayerComponent, maxStepVelocity)
                REGISTER_PROPERTY(PlayerComponent, minStepVelocity)
                REGISTER_PROPERTY(PlayerComponent, maxStepInterval)
                REGISTER_PROPERTY(PlayerComponent, minStepInterval);

        REGISTER_COMPONENT(ViewmodelComponent, "ViewmodelComponent")
                REGISTER_PROPERTY(ViewmodelComponent, position_offset)
                REGISTER_PROPERTY(ViewmodelComponent, euler_offset)
                REGISTER_PROPERTY(ViewmodelComponent, sway_multiplier)
                REGISTER_PROPERTY(ViewmodelComponent, smoothing)
                REGISTER_PROPERTY(ViewmodelComponent, amplitude)
                REGISTER_PROPERTY(ViewmodelComponent, frequency)
                REGISTER_PROPERTY(ViewmodelComponent, vertical_velocity_multiplier)
                REGISTER_PROPERTY(ViewmodelComponent, max_vertical_offset);

        REGISTER_COMPONENT(EnemyComponent, "EnemyComponent")
                REGISTER_PROPERTY(EnemyComponent, health)
                REGISTER_PROPERTY(EnemyComponent, dead)
                REGISTER_PROPERTY(EnemyComponent, hurt_sound);

        REGISTER_COMPONENT(BulletComponent, "BulletComponent")
                REGISTER_PROPERTY(BulletComponent, position)
                REGISTER_PROPERTY(BulletComponent, direction)
                REGISTER_PROPERTY(BulletComponent, damage);

        REGISTER_COMPONENT(HideInEditorComponent, "HideInEditorComponent");
        REGISTER_COMPONENT(EditorCameraComponent, "EditorCameraComponent");
        REGISTER_COMPONENT(LightComponent, "LightComponent")
                REGISTER_PROPERTY(LightComponent, color)
                REGISTER_PROPERTY(LightComponent, strength)
                REGISTER_PROPERTY(LightComponent, radius)
                REGISTER_PROPERTY(LightComponent, type)
                REGISTER_PROPERTY(LightComponent, outer_cutoff)
                REGISTER_PROPERTY(LightComponent, inner_cutoff)
                REGISTER_PROPERTY(LightComponent, always_update_shadows)
                REGISTER_PROPERTY(LightComponent, cast_shadows);

        REFLECT_ENUM(LightComponent::LightType)
                ENUMERATOR(LightComponent::LightType, Directional)
                ENUMERATOR(LightComponent::LightType, Point)
                ENUMERATOR(LightComponent::LightType, Spot);

        REGISTER_COMPONENT(AnimatorComponent, "AnimatorComponent")
                .func<[](AnimatorComponent &comp, nlohmann::json &j)
                {
                    serialize_animator(comp, j);
                }>("serialize"_hs)
                .func<[](AnimatorComponent &comp, const nlohmann::json &j)
                {
                    deserialize_animator(comp, j);
                }>("deserialize"_hs);
        REGISTER_COMPONENT(ParentComponent, "ParentComponent")
                REGISTER_PROPERTY(ParentComponent, children);
        REGISTER_COMPONENT(NativeScriptComponent, "NativeScriptComponent")
                REGISTER_PROPERTY(NativeScriptComponent, type_name);
    }
}
