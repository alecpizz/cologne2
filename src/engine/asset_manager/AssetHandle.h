//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include "AssetManager.h"

namespace cologne
{
    class AnimationClip;
}

namespace cologne
{
    template<typename T>
    struct AssetHandle
    {
        //move me into the source file please
        explicit AssetHandle(const std::string& name)
        {
            handle = name;
            if constexpr(std::is_same_v<T, AnimationClip>)
            {
                _asset = AssetManager::get_animation_by_name(name);
            }
        }
        T* get() const
        {
            if (!is_valid())
            {
                return nullptr;
            }
            return _asset.get();
        }
        std::string handle;
        T* operator->() const
        {
            return get();
        }
        T& operator*() const
        {
            return *get();
        }

        bool is_valid() const
        {
            return !handle.empty();
        }

    private:
        Ref<T> _asset;
    };
}
