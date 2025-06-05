#pragma once
#include "Entity.h"

namespace cologne
{
    class ScriptableEntity
    {
    public:
        virtual ~ScriptableEntity()
        {
        }

        template<typename T>
        T &get_component()
        {
            return _entity.get_component<T>();
        }

        template<typename T>
        // ReSharper disable once CppMemberFunctionMayBeConst
        bool has_component()
        {
            return _entity.has_component<T>();
        }

    protected:
        virtual void on_create()
        {
        }

        virtual void on_destroy()
        {
        }

        virtual void on_update(float dt)
        {
        }

    private:
        Entity _entity;
        friend class Scene;
    };
}
