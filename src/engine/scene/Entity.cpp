//
// Created by alecpizz on 5/31/25.
//

#include "Entity.h"
#include "Components.h"

namespace cologne
{
    Entity::Entity(entt::entity handle, Scene *scene) : _entity_handle(handle), _scene(scene)
    {
    }

    TransformComponent & Entity::get_transform()
    {
        return get_component<TransformComponent>();
    }

    const std::string & Entity::get_name()
    {
        return get_component<TagComponent>().tag;
    }

    const UUID & Entity::get_uuid()
    {
        return get_component<IDComponent>().id;
    }

    const bool Entity::is_active()
    {
        return get_component<ActiveComponent>().active;
    }
}
