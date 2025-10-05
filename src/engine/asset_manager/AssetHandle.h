//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include "AssetManager.h"


namespace cologne
{
    template<typename T>
    struct AssetHandle
    {
        explicit AssetHandle(const std::string &name)
        {
            handle = name;
            if constexpr (std::is_same_v<T, AnimationClip>)
            {
                _asset = AssetManager::get_animation_by_name(name);
            }
            else if constexpr (std::is_same_v<T, Mesh>)
            {
                _asset = AssetManager::get_mesh_by_name(name);
            }
            else if constexpr (std::is_same_v<T, Texture>)
            {
                _asset = AssetManager::get_texture_by_name(name);
            }
            else if constexpr (std::is_same_v<T, Model>)
            {
                _asset = AssetManager::get_model_by_name(name);
            }
            else if constexpr (std::is_same_v<T, SkinnedModel>)
            {
                _asset = AssetManager::get_skinned_model_by_name(name);
            }
            else if constexpr (std::is_same_v<T, SkinnedMesh>)
            {
                _asset = AssetManager::get_skinned_mesh_by_name(name);
            }
            else
            {
                LOG_ERROR("UKNOWN TYPE!");
            }
        }

        AssetHandle() = default;

        T *get() const
        {
            if (!is_valid())
            {
                return nullptr;
            }
            if (!_asset)
            {
                //todo: assetmanager get asset by T
                LOG_ERROR("MISSING ASSET PTR");
            }
            return _asset;
        }

        std::string handle;

        T *operator->() const
        {
            return get();
        }

        T &operator*() const
        {
            return *get();
        }

        [[nodiscard]] bool is_valid() const
        {
            return !handle.empty();
        }

        explicit operator bool() const
        {
            return is_valid();
        }

        bool operator==(const AssetHandle & other) const
        {
            return handle == other.handle;
        }

    private:
        T* _asset;
    };
}
