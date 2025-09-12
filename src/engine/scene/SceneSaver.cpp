//
// Created by alecpizz on 7/19/25.
//

#include "SceneSaver.h"
#include <engine/util/FileUtil.h>
#include <nlohmann/json.hpp>
#include "Components.h"
#include "Scene.h"
#include <fstream>
#include <engine/asset_manager/AssetManager.h>
#include "ComponentRegistry.h"

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
}

namespace cologne
{
    SceneSaver::SceneSaver(Scene *scene)
    {
        _scene = scene;
    }


    template<typename T>
    void save_property(nlohmann::json &j, const std::string &member_name, entt::meta_any &any)
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

    void save_component(entt::meta_any instance, nlohmann::json &j)
    {
        using namespace entt::literals;
        for (auto [id, data]: instance.type().data())
        {
            auto hash = data.type().info().hash();
            auto property_any = data.get(instance);
            std::string member_name;
            if (auto *mp = static_cast<const ComponentRegistry::PropertiesMap *>(data.custom()))
            {
                if (auto it = mp->find("name"_hs); it != mp->end())
                {
                    member_name = *it->second.try_cast<const char *>();
                }
            }
            if (hash == entt::type_hash<glm::vec3>::value())
            {
                save_property<glm::vec3>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<glm::vec4>::value())
            {
                save_property<glm::vec4>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<glm::quat>::value())
            {
                save_property<glm::quat>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<cologne::UUID>::value())
            {
                save_property<UUID>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<glm::mat4>::value())
            {
                save_property<glm::mat4>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<float>::value())
            {
                save_property<float>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<bool>::value())
            {
                save_property<bool>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<std::string>::value())
            {
                save_property<std::string>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<int>::value())
            {
                save_property<int>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<uint64_t>::value())
            {
                save_property<uint64_t>(j, member_name, property_any);
            }
            else if (hash == entt::type_hash<uint32_t>::value())
            {
                save_property<uint32_t>(j, member_name, property_any);
            }
            else if (data.type().is_sequence_container())
            {
                nlohmann::json array_node = nlohmann::json::array();
                if (auto view = data.get(instance).as_sequence_container(); view)
                {
                    for (auto meta_any: view)
                    {
                        save_component(meta_any.as_ref(), array_node);
                    }
                }

                if (member_name.empty())
                {
                    j.emplace_back(std::move(array_node));
                }
                else
                {
                    j[member_name] = std::move(array_node);
                }
            }
            else
            {
                nlohmann::json object_node;
                save_component(data.get(instance).as_ref(), object_node);
                j[member_name] = object_node;
            }
        }
    }

    void load_property(const nlohmann::json &j, entt::meta_data &meta_data, entt::meta_any instance)
    {
        auto hash = meta_data.type().info().hash();
        if (hash == entt::type_hash<glm::vec3>::value())
        {
            if (!meta_data.set(instance, j.get<glm::vec3>()))
            {
                LOG_ERROR("couldn't set meta data");
            }
        }
        else if (hash == entt::type_hash<glm::vec4>::value())
        {
            meta_data.set(instance, j.get<glm::vec4>());
        }
        else if (hash == entt::type_hash<glm::quat>::value())
        {
            meta_data.set(instance, j.get<glm::quat>());
        }
        else if (hash == entt::type_hash<UUID>::value())
        {
            meta_data.set(instance, j.get<UUID>());
        }
        else if (hash == entt::type_hash<glm::mat4>::value())
        {
            meta_data.set(instance, j.get<glm::mat4>());
        }
        else if (hash == entt::type_hash<float>::value())
        {
            meta_data.set(instance, j.get<float>());
        }
        else if (hash == entt::type_hash<bool>::value())
        {
            meta_data.set(instance, j.get<bool>());
        }
        else if (hash == entt::type_hash<std::string>::value())
        {
            meta_data.set(instance, j.get<std::string>());
        }
        else if (hash == entt::type_hash<int>::value())
        {
            meta_data.set(instance, j.get<int>());
        }
        else if (hash == entt::type_hash<uint64_t>::value())
        {
            meta_data.set(instance, j.get<uint64_t>());
        }
        else if (meta_data.type().is_sequence_container())
        {
            if (j.is_array())
            {
                auto container_view = meta_data.get(instance).as_ref().as_sequence_container();
                container_view.resize(j.size());
                for (size_t i = 0; i < j.size(); i++)
                {
                    for (auto [id, data]: container_view.value_type().data())
                    {
                        load_property(j[i], data, container_view[i].as_ref());
                    }
                }
            }
        }
        else if (meta_data.type().is_enum())
        {
        }
        else
        {
            LOG_INFO("TRYING TO RECURSIVELY LOAD UNKNOWN TYPE %s!",
                     std::string(meta_data.type().info().name()).c_str());
            load_property(j, meta_data, meta_data.get(instance).as_ref());
        }
    }

    void load_component(const nlohmann::json &j, entt::meta_any &instance)
    {
        using namespace entt::literals;
        for (auto [id, data]: instance.type().data())
        {
            if (auto *mp = static_cast<const ComponentRegistry::PropertiesMap *>(data.custom()))
            {
                if (auto it = mp->find("name"_hs); it != mp->end())
                {
                    std::string member_name = *it->second.try_cast<const char *>();
                    if (j.contains(member_name))
                    {
                        load_property(j.at(member_name), data, instance.as_ref());
                    }
                }
            }
        }
    }

    void emplace_component(entt::registry &registry, entt::entity entity, entt::meta_any &any)
    {
        using namespace entt::literals;
        if (auto emplace_func = any.type().func("emplace"_hs); emplace_func)
        {
            emplace_func.invoke({}, &registry, entity, any.as_ref());
        }
        else
        {
            LOG_WARN("NO EMPLACE FUNCTION FOR THE COMPONENT %s, WILL BE SKIPPED!",
                     std::string(any.type().info().name()).c_str());
        }
    }

    void SceneSaver::serialize(const std::string &path)
    {
        using namespace entt::literals;
        if (!_scene)
        {
            LOG_ERROR("No scene to serialize!");
            return;
        }

        nlohmann::json j_scene;
        j_scene["scene_name"] = _scene->get_scene_name();
        nlohmann::json j_entities;
        _scene->_registry.view<entt::entity>().each([&](auto entity)
        {
            nlohmann::json j_entity;
            nlohmann::json j_components = nlohmann::json::object();
            for (auto [id, set]: _scene->_registry.storage())
            {
                if (!set.contains(entity))
                {
                    continue;
                }
                const auto hash = set.type().hash();
                if (!ComponentRegistry::get_component_map().contains(hash))
                {
                    LOG_WARN("Component map did not contain %s", std::string(set.type().name()).c_str());
                    continue;
                }
                const std::string &name = ComponentRegistry::get_component_map().at(hash);
                if (auto meta = entt::resolve(entt::hashed_string(name.c_str())))
                {
                    nlohmann::json j_component_data;
                    entt::meta_any instance = meta.from_void(set.value(entity));
                    if (auto func = meta.func("serialize"_hs); func)
                    {
                        func.invoke(instance, instance.as_ref(), entt::forward_as_meta(j_component_data).as_ref());
                    }
                    else
                    {
                        save_component(instance, j_component_data);
                    }
                    j_components[name] = j_component_data;
                }
            }
            j_entity["components"] = j_components;
            j_entities.push_back(j_entity);
        });

        j_scene["entities"] = j_entities;

        FileUtil::create_directory_recursive(path);
        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("error opening file for serialization!");
            return;
        }
        file << j_scene.dump(4);
        file.close();

        FileUtil::create_directory_recursive(RESOURCES_PATH "last_saved_scene.json");
        std::ofstream file2(RESOURCES_PATH "last_saved_scene.json");
        if (!file2.is_open())
        {
            LOG_ERROR("error opening file for serialization!");
            return;
        }
        nlohmann::json j;
        j["scene_name"] = _scene->_scene_name;
        file2 << j.dump(4);
        file2.close();
    }

    void copy_component_meta_data(entt::meta_any &src, entt::meta_any &dest)
    {
        for (auto [i, data]: src.type().data())
        {
            data.set(dest, data.get(src));
        }
    }

    void SceneSaver::deserialize(const std::string &path)
    {
        if (!FileUtil::file_exists(path))
        {
            LOG_ERROR("No file at %s", path.c_str());
            return;
        }

        std::ifstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("Error opening file at %s", path.c_str());
            return;
        }
        using namespace entt::literals;
        using json = nlohmann::json;
        json data = json::parse(file);
        std::string scene_name = data["scene_name"];
        _scene->set_scene_name(scene_name);
        json entities = data["entities"];
        for (const auto &entity: entities)
        {
            entt::entity e = _scene->_registry.create();
            for (auto &[type_name, j_component_data]: entity.at("components").items())
            {
                if (auto meta_type = entt::resolve(entt::hashed_string(type_name.c_str())))
                {
                    auto instance = meta_type.construct();
                    if (!instance)
                    {
                        LOG_INFO("COULDNT CONSTRUCT TYPE %s", type_name.c_str());
                    }
                    if (auto func = instance.type().func("deserialize"_hs); func)
                    {
                        func.invoke(instance, instance.as_ref(), entt::forward_as_meta(j_component_data).as_ref());
                    }
                    else
                    {
                        load_component(j_component_data, instance);
                    }
                    emplace_component(_scene->_registry, e, instance);
                }
            }
        }
        file.close();
    }

    void SceneSaver::serialize_runtime(const std::string &path)
    {
    }

    void SceneSaver::deserialize_runtime(const std::string &path)
    {
    }

}
