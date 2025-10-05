//
// Created by alecpizz on 9/20/25.
//
#include <engine/asset_manager/AssetHandle.h>
#include <engine/core/Color.h>
#include <engine/core/UUID.h>
#include <engine/scene/Serialization.h>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <nlohmann/detail/meta/std_fs.hpp>

namespace nlohmann
{
    template<>
    struct adl_serializer<glm::vec2>
    {
        static void to_json(json &j, const glm::vec2 &vec)
        {
            j = {vec.x, vec.y};
        }

        static void from_json(const json &j, glm::vec2 &vec)
        {
            j.at(0).get_to(vec.x);
            j.at(1).get_to(vec.y);
        }
    };

    template<>
    struct adl_serializer<glm::vec3>
    {
        static void to_json(json &j, const glm::vec3 &vec)
        {
            j = {vec.x, vec.y, vec.z};
        }

        static void from_json(const json &j, glm::vec3 &vec)
        {
            j.at(0).get_to(vec.x);
            j.at(1).get_to(vec.y);
            j.at(2).get_to(vec.z);
        }
    };

    template<>
    struct adl_serializer<glm::vec4>
    {
        static void to_json(json &j, const glm::vec4 &vec)
        {
            j = {vec.x, vec.y, vec.z, vec.w};
        }

        static void from_json(const json &j, glm::vec4 &vec)
        {
            j.at(0).get_to(vec.x);
            j.at(1).get_to(vec.y);
            j.at(2).get_to(vec.z);
            j.at(3).get_to(vec.w);
        }
    };

    template<>
    struct adl_serializer<glm::quat>
    {
        static void to_json(json &j, const glm::quat &quat)
        {
            j = {quat.w, quat.x, quat.y, quat.z};
        }

        static void from_json(const json &j, glm::quat &quat)
        {
            j.at(0).get_to(quat.w);
            j.at(1).get_to(quat.x);
            j.at(2).get_to(quat.y);
            j.at(3).get_to(quat.z);
        }
    };


    template<>
    struct adl_serializer<glm::mat4>
    {
        static void to_json(json &j, const glm::mat4 &mat)
        {
            j = {mat[0], mat[1], mat[2], mat[3]};
        }

        static void from_json(const json &j, glm::mat4 &mat)
        {
            j.at(0).get_to(mat[0]);
            j.at(1).get_to(mat[1]);
            j.at(2).get_to(mat[2]);
            j.at(3).get_to(mat[3]);
        }
    };

    template<>
    struct adl_serializer<cologne::Color>
    {
        static void to_json(json &j, const cologne::Color &color)
        {
            j = {color.color.r, color.color.g, color.color.b, color.color.a};
        }

        static void from_json(const json &j, cologne::Color &color)
        {
            j.at(0).get_to(color.color.r);
            j.at(1).get_to(color.color.g);
            j.at(2).get_to(color.color.b);
            j.at(3).get_to(color.color.a);
        }
    };

    template<>
    struct adl_serializer<cologne::UUID>
    {
        static void to_json(json &j, const cologne::UUID &id)
        {
            j = id._uuid;
        }

        static void from_json(const json &j, cologne::UUID &id)
        {
            id = cologne::UUID(j.get<uint64_t>());
        }
    };

    template<typename T>
    struct adl_serializer<cologne::AssetHandle<T> >
    {
        static void to_json(json &j, const cologne::AssetHandle<T> &handle)
        {
            j = handle.handle;
        }

        static void from_json(const json &j, cologne::AssetHandle<T> &handle)
        {
            handle = cologne::AssetHandle<T>(j.get<std::string>());
        }
    };
}

namespace cologne::Serialization
{
    template<typename T>
    void serialize(entt::meta_any any, nlohmann::json &j, const std::string &member_name)
    {
        T value = any.cast<T>();
        if (member_name.empty())
        {
            j.emplace_back(value);
        }
        else
        {
            if (j.is_array())
            {
                j.emplace_back(value);
            }
            else
            {
                j[member_name] = value;
            }
        }
    }

    template<typename T>
    void serialize_asset_handle(entt::meta_any any, nlohmann::json &j, const std::string &member_name)
    {
        AssetHandle<T> value = any.cast<AssetHandle<T> >();
        if (member_name.empty())
        {
            j.emplace_back(value.handle);
        }
        else
        {
            if (j.is_array())
            {
                j.emplace_back(value.handle);
            }
            else
            {
                j[member_name] = value.handle;
            }
        }
    }

    template<typename T>
    void deserialize(const nlohmann::json &j, entt::meta_data &meta_data, entt::meta_any &instance)
    {
        meta_data.set(instance, j.get<T>());
    }

    template<typename T>
    void deserialize_asset_handle(const nlohmann::json &j, entt::meta_data &meta_data, entt::meta_any &instance)
    {
        meta_data.set(instance, AssetHandle<T>(j.get<std::string>()));
    }

    void init()
    {
        using namespace entt::literals;
#define MAKE_SERIALIZERS(T) \
        entt::meta_factory<T>() \
      .func<[](T & val, nlohmann::json & j, const std::string& member_name) \
      { \
          serialize<T>(val, j, member_name); \
      }>("serialize"_hs) \
      .func<deserialize<T>>("deserialize"_hs)
#define MAKE_ASSET_HANDLE_SERIALIZERS(T) \
        entt::meta_factory<AssetHandle<T>>()\
        .func<[](AssetHandle<T> & val, nlohmann::json & j, const std::string& member_name)\
        {\
            serialize_asset_handle<T>(val, j, member_name); \
        }>("serialize"_hs) \
        .func<deserialize_asset_handle<T>>("deserialize"_hs);

        MAKE_ASSET_HANDLE_SERIALIZERS(AnimationClip);
        MAKE_ASSET_HANDLE_SERIALIZERS(Texture);
        MAKE_ASSET_HANDLE_SERIALIZERS(SkinnedModel);
        MAKE_ASSET_HANDLE_SERIALIZERS(SkinnedMesh);
        MAKE_ASSET_HANDLE_SERIALIZERS(Material);
        MAKE_ASSET_HANDLE_SERIALIZERS(Mesh);
        MAKE_ASSET_HANDLE_SERIALIZERS(Model);
        MAKE_SERIALIZERS(glm::vec3);
        MAKE_SERIALIZERS(glm::vec4);
        MAKE_SERIALIZERS(Color);
        MAKE_SERIALIZERS(glm::vec2);
        MAKE_SERIALIZERS(glm::quat);
        MAKE_SERIALIZERS(UUID);
        MAKE_SERIALIZERS(glm::mat4);
        MAKE_SERIALIZERS(float);
        MAKE_SERIALIZERS(double);
        MAKE_SERIALIZERS(bool);
        MAKE_SERIALIZERS(std::string);
        MAKE_SERIALIZERS(int);
        MAKE_SERIALIZERS(uint64_t);
        MAKE_SERIALIZERS(uint32_t);
        MAKE_SERIALIZERS(uint16_t);
        MAKE_SERIALIZERS(uint8_t);
    }
}
