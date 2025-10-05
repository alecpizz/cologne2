//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include "AssetManager.h"


namespace cologne
{
    template<typename Asset>
    struct AssetHandle
    {
        std::string handle;
        explicit AssetHandle(const std::string &name)
        {
            handle = name;
        }

        AssetHandle() = default;

        Asset *get() const
        {
            if (!is_valid())
            {
                return nullptr;
            }
            return AssetManager::get_asset_by_name<Asset>(handle);
        }


        Asset *operator->() const
        {
            return get();
        }

        Asset &operator*() const
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
    };
}
