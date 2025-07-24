//
// Created by alecpizz on 7/21/25.
//
#include "ComponentRegistry.h"

#include "Components.h"


namespace cologne::ComponentRegistry
{
    static std::vector<unsigned int> component_ids;

    const std::vector<unsigned int> &get_component_ids()
    {
        return component_ids;
    }

#define REGISTER_COMPONENT(T) \
    component_ids.emplace_back(entt::type_hash<T>::value())


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

        entt::meta_factory<cologne::TransformComponent>()
                .data<&TransformComponent::position, entt::as_ref_t>("position"_hs)
                .custom<PropertiesMap>(PropertiesMap{{"name"_hs, "position"}})
                .data<&TransformComponent::rotation, entt::as_ref_t>("rotation"_hs)
                .custom<PropertiesMap>(PropertiesMap{{"name"_hs, "rotation"}})
                .data<&TransformComponent::scale, entt::as_ref_t>("scale"_hs)
                .custom<PropertiesMap>(PropertiesMap{{"name"_hs, "scale"}})
                .func<&entt::registry::emplace<TransformComponent>>("emplace"_hs);

        entt::meta_factory<UUID>().conv<uint64_t>().data<&UUID::_uuid>("uuid"_hs);

        entt::meta_factory<cologne::IDComponent>()
                .data<&IDComponent::id>("id"_hs)
                .custom<PropertiesMap>(PropertiesMap{{"name"_hs, "id"}})
                .func<&entt::registry::emplace<IDComponent>>("emplace"_hs);

        entt::meta_factory<cologne::WorldTransformComponent>()
                .data<&WorldTransformComponent::transform>("transform"_hs)
                .custom<PropertiesMap>(PropertiesMap{{"name"_hs, "transform"}})
                .func<&entt::registry::emplace<WorldTransformComponent>>("emplace"_hs);
    }
}
