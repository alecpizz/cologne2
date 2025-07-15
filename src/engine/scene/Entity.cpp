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
}
