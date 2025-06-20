#pragma once
#include "Entity.h"

namespace cologne
{
    enum class RuntimeMode
    {
        GAME_ONLY = 0,
        EDITOR_ONLY = 1,
        EDITOR_AND_GAME = 2,
    };

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
        virtual RuntimeMode get_runtime_mode()
        {
            return RuntimeMode::GAME_ONLY;
        }

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
        friend class Editor;
    };
}
