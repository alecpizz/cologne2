//
// Created by alecpizz on 7/21/25.
//
#include "ComponentRegistry.h"


#include <engine/asset_manager/AssetManager.h>
#include <nlohmann/json.hpp>
#include "Components.h"


namespace cologne::ComponentRegistry
{
    template<typename T>
    void copy(entt::sparse_set &base, entt::registry &to)
    {
        auto &src = static_cast<entt::storage_for_t<T> &>(base);
        to.insert<T>(src.entt::sparse_set::begin(), src.entt::sparse_set::end(), src.begin());
    }

    using namespace entt::literals;
#define REGISTER_COMPONENT(T, name, tr) \
component_type_map[entt::type_hash<T>::value()] = name; \
entt::meta_factory<T>() \
.type(entt::hashed_string(name))\
.traits(tr) \
.func<[](entt::registry* registry, entt::entity entity, T& value) { \
        registry->emplace_or_replace<T>(entity, std::move(value)); }>("emplace"_hs) \
.func<[](entt::registry* registry, entt::entity entity) { \
registry->remove<T>(entity); }>("remove"_hs) \
.func<&copy<T>>("copy"_hs)

#define REGISTER_PROPERTY(Type, member, ...) \
.data<&Type::member, entt::as_ref_t>(#member##_hs) \
.custom<PropertiesMap>(PropertiesMap{{"name"_hs, #member} __VA_OPT__(, __VA_ARGS__)})


#define REFLECT_ENUM(T) \
        entt::meta_factory<T>()
#define ENUMERATOR(E, Member, ...) \
        .data<E::Member>(#Member##_hs) \
        .custom<PropertiesMap>(PropertiesMap{{"name"_hs, #Member} __VA_OPT__(, __VA_ARGS__)})

    static std::map<entt::id_type, std::string> component_type_map;



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

        REGISTER_COMPONENT(TransformComponent, "TransformComponent", NO_EDITOR)
                REGISTER_PROPERTY(TransformComponent, position)
                REGISTER_PROPERTY(TransformComponent, rotation)
                REGISTER_PROPERTY(TransformComponent, scale);

        entt::meta_factory<UUID>().conv<uint64_t>()
                .type("UUID"_hs)
                REGISTER_PROPERTY(UUID, _uuid);

        REGISTER_COMPONENT(IDComponent, "IDComponent", NO_EDITOR)
                REGISTER_PROPERTY(IDComponent, id);

        REGISTER_COMPONENT(WorldTransformComponent, "WorldTransformComponent", NO_EDITOR)
                REGISTER_PROPERTY(WorldTransformComponent, transform);

        REGISTER_COMPONENT(ChildComponent, "ChildComponent", NO_EDITOR)
                REGISTER_PROPERTY(ChildComponent, parent);

        REGISTER_COMPONENT(ActiveComponent, "ActiveComponent", NO_EDITOR)
                REGISTER_PROPERTY(ActiveComponent, active);

        REGISTER_COMPONENT(ModelComponent, "ModelComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(ModelComponent, model_name)
                REGISTER_PROPERTY(ModelComponent, gi_only);
        //
        REGISTER_COMPONENT(MeshComponent, "MeshComponent", EDITOR_READ_WRITE)
        //     .traits(Traits::TRANSIENT)
        //     .type("MeshComponent"_hs)
                REGISTER_PROPERTY(MeshComponent, mesh_name);


        REGISTER_COMPONENT(SkinnedModelComponent, "SkinnedModelComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(SkinnedModelComponent, model_name);

        REGISTER_COMPONENT(CameraComponent, "CameraComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(CameraComponent, fov_radians)
                REGISTER_PROPERTY(CameraComponent, primary)
                REGISTER_PROPERTY(CameraComponent, ortho_zoom)
                REGISTER_PROPERTY(CameraComponent, orthographic);

        REGISTER_COMPONENT(TagComponent, "TagComponent", NO_EDITOR)
                REGISTER_PROPERTY(TagComponent, tag);

        REGISTER_COMPONENT(StaticColliderComponent, "StaticColliderComponent", EDITOR_READ_ONLY)
                REGISTER_PROPERTY(StaticColliderComponent, mesh_name);

        REGISTER_COMPONENT(RigidbodyComponent, "RigidbodyComponent", EDITOR_READ_ONLY);

        REGISTER_COMPONENT(ConvexMeshColliderComponent, "ConvexMeshColliderComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(ConvexMeshColliderComponent, mesh_name);

        REGISTER_COMPONENT(PlayerComponent, "PlayerComponent", EDITOR_READ_WRITE)
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

        REGISTER_COMPONENT(ViewmodelComponent, "ViewmodelComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(ViewmodelComponent, position_offset)
                REGISTER_PROPERTY(ViewmodelComponent, euler_offset)
                REGISTER_PROPERTY(ViewmodelComponent, sway_multiplier)
                REGISTER_PROPERTY(ViewmodelComponent, smoothing)
                REGISTER_PROPERTY(ViewmodelComponent, amplitude)
                REGISTER_PROPERTY(ViewmodelComponent, frequency)
                REGISTER_PROPERTY(ViewmodelComponent, vertical_velocity_multiplier)
                REGISTER_PROPERTY(ViewmodelComponent, max_vertical_offset);

        REGISTER_COMPONENT(EnemyComponent, "EnemyComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(EnemyComponent, health)
                REGISTER_PROPERTY(EnemyComponent, dead)
                REGISTER_PROPERTY(EnemyComponent, hurt_sound);

        REGISTER_COMPONENT(BulletComponent, "BulletComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(BulletComponent, position)
                REGISTER_PROPERTY(BulletComponent, direction)
                REGISTER_PROPERTY(BulletComponent, damage);

        REGISTER_COMPONENT(AnimComponent, "AnimComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(AnimComponent, base_clip_name)
                REGISTER_PROPERTY(AnimComponent, one_shot_name);
            REGISTER_COMPONENT(RagdollComponent, "RagdollComponent", EDITOR_READ_ONLY);

        REGISTER_COMPONENT(HideInEditorComponent, "HideInEditorComponent", NO_EDITOR);
        REGISTER_COMPONENT(EditorCameraComponent, "EditorCameraComponent", NO_EDITOR);
        REGISTER_COMPONENT(LightComponent, "LightComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(LightComponent, color)
                REGISTER_PROPERTY(LightComponent, strength)
                REGISTER_PROPERTY(LightComponent, radius)
                REGISTER_PROPERTY(LightComponent, type)
                REGISTER_PROPERTY(LightComponent, outer_cutoff)
                REGISTER_PROPERTY(LightComponent, inner_cutoff)
                REGISTER_PROPERTY(LightComponent, always_update_shadows)
                REGISTER_PROPERTY(LightComponent, cast_shadows);

        REGISTER_COMPONENT(InteractorComponent, "InteractorComponent", EDITOR_READ_WRITE)
                REGISTER_PROPERTY(InteractorComponent, update_every_frame);

        REFLECT_ENUM(LightComponent::LightType)
                ENUMERATOR(LightComponent::LightType, Directional)
                ENUMERATOR(LightComponent::LightType, Point)
                ENUMERATOR(LightComponent::LightType, Spot);

        REGISTER_COMPONENT(ParentComponent, "ParentComponent", NO_EDITOR)
                REGISTER_PROPERTY(ParentComponent, children);
    }
}
