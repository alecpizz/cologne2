//
// Created by alecpizz on 7/21/25.
//
#include "ComponentRegistry.h"

#include "Components.h"
#include "SceneSaver.h"


namespace cologne::ComponentRegistry
{
#define REGISTER_COMPONENT(T) \
entt::meta_factory<T>() \
.func<[](entt::registry* registry, entt::entity entity, T& value) { \
        registry->emplace_or_replace<T>(entity, std::move(value)); }>("emplace"_hs)
#define REGISTER_PROPERTY(Type, member, ...) \
.data<&Type::member, entt::as_ref_t>(#member##_hs) \
.custom<PropertiesMap>(PropertiesMap{{"name"_hs, #member} __VA_OPT__(, __VA_ARGS__)})

    void register_components()
    {
        using namespace entt::literals;
        entt::meta_reset();
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

        REGISTER_COMPONENT(TransformComponent)
                REGISTER_PROPERTY(TransformComponent, position)
                REGISTER_PROPERTY(TransformComponent, rotation)
                REGISTER_PROPERTY(TransformComponent, scale);

        entt::meta_factory<UUID>().conv<uint64_t>().data<&UUID::_uuid>("uuid"_hs);
        REGISTER_COMPONENT(IDComponent)
                REGISTER_PROPERTY(IDComponent, id);

        REGISTER_COMPONENT(WorldTransformComponent)
                REGISTER_PROPERTY(WorldTransformComponent, transform);

        REGISTER_COMPONENT(ChildComponent)
                REGISTER_PROPERTY(ChildComponent, parent);

        REGISTER_COMPONENT(ActiveComponent)
                REGISTER_PROPERTY(ActiveComponent, active);

        REGISTER_COMPONENT(ModelComponent)
                REGISTER_PROPERTY(ModelComponent, model_name)
                REGISTER_PROPERTY(ModelComponent, gi_only);

        REGISTER_COMPONENT(MeshComponent)
                REGISTER_PROPERTY(MeshComponent, mesh_name);

        REGISTER_COMPONENT(SkinnedModelComponent)
                REGISTER_PROPERTY(SkinnedModelComponent, model_name);

        REGISTER_COMPONENT(CameraComponent)
                REGISTER_PROPERTY(CameraComponent, fov_radians)
                REGISTER_PROPERTY(CameraComponent, primary);

        REGISTER_COMPONENT(TagComponent)
                REGISTER_PROPERTY(TagComponent, tag);

        REGISTER_COMPONENT(StaticColliderComponent)
                REGISTER_PROPERTY(StaticColliderComponent, mesh_name);

        REGISTER_COMPONENT(PlayerComponent)
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

        REGISTER_COMPONENT(ViewmodelComponent)
                REGISTER_PROPERTY(ViewmodelComponent, position_offset)
                REGISTER_PROPERTY(ViewmodelComponent, euler_offset)
                REGISTER_PROPERTY(ViewmodelComponent, sway_multiplier)
                REGISTER_PROPERTY(ViewmodelComponent, smoothing)
                REGISTER_PROPERTY(ViewmodelComponent, amplitude)
                REGISTER_PROPERTY(ViewmodelComponent, frequency)
                REGISTER_PROPERTY(ViewmodelComponent, vertical_velocity_multiplier)
                REGISTER_PROPERTY(ViewmodelComponent, max_vertical_offset);

        REGISTER_COMPONENT(EnemyComponent)
                REGISTER_PROPERTY(EnemyComponent, health)
                REGISTER_PROPERTY(EnemyComponent, dead)
                REGISTER_PROPERTY(EnemyComponent, hurt_sound);

        REGISTER_COMPONENT(BulletComponent)
                REGISTER_PROPERTY(BulletComponent, position)
                REGISTER_PROPERTY(BulletComponent, direction)
                REGISTER_PROPERTY(BulletComponent, damage);

        REGISTER_COMPONENT(LightComponent)
                REGISTER_PROPERTY(LightComponent, color)
                REGISTER_PROPERTY(LightComponent, strength)
                REGISTER_PROPERTY(LightComponent, radius)
                REGISTER_PROPERTY(LightComponent, type)
                REGISTER_PROPERTY(LightComponent, outer_cutoff)
                REGISTER_PROPERTY(LightComponent, inner_cutoff)
                REGISTER_PROPERTY(LightComponent, cast_shadows);

        REGISTER_COMPONENT(ParentComponent)
                REGISTER_PROPERTY(ParentComponent, children);
        REGISTER_COMPONENT(NativeScriptComponent)
                REGISTER_PROPERTY(NativeScriptComponent, type_name);
    }
}
