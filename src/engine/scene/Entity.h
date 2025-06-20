//
// Created by alecpizz on 5/31/25.
//

#pragma once
#include "Scene.h"
#include <entt/entt.hpp>


namespace cologne
{
    class Entity
    {
    public:
        Entity() = default;

        Entity(entt::entity handle, Scene *scene);

        Entity(const Entity &other) = default;

        template<typename T>
        // ReSharper disable once CppMemberFunctionMayBeConst
        bool has_component()
        {
            return _scene->_registry.all_of<T>(_entity_handle);
        }

        template<typename T, typename... Args>
        T &add_component(Args &&... args)
        {
            if (has_component<T>())
            {
                throw std::runtime_error("Entity already has component!");
            }
            return _scene->_registry.emplace<T>(_entity_handle, std::forward<Args>(args)...);
        }

        template<typename T>
        T &get_component()
        {
            if (!has_component<T>())
            {
                throw std::runtime_error("Entity does not have component!");
            }
            return _scene->_registry.get<T>(_entity_handle);
        }

        template<typename T>
        void remove_component()
        {
            if (!has_component<T>())
            {
                throw std::runtime_error("Entity does not have component!");
            }
            _scene->_registry.remove<T>(_entity_handle);
        }

        explicit operator bool() const { return _entity_handle != entt::null; }
        explicit operator uint32_t() const { return (uint32_t(_entity_handle)); }
        operator entt::entity() const { return _entity_handle; }
        bool operator==(const Entity & other) const {return _entity_handle == other._entity_handle;}

    private:
        entt::entity _entity_handle = entt::null;
        Scene *_scene;
    };
}
