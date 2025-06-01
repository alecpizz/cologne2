//
// Created by alecpizz on 5/31/25.
//

#include "Entity.h"

namespace cologne
{
    Entity::Entity(entt::entity handle, Scene *scene) : _entity_handle(handle), _scene(scene)
    {
    }
}
