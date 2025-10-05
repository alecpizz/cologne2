//
// Created by alecpizz on 9/12/25.
//

#include "Prefab.h"

#include <fstream>
#include <engine/util/FileUtil.h>
#include <nlohmann/json.hpp>
#include <engine/scene/components/ComponentRegistry.h>
#include "Entity.h"
#include "SceneSaver.h"
#include "components/ChildComponent.h"
#include "components/IDComponent.h"
#include "components/ParentComponent.h"

namespace cologne
{
    void save_component(entt::meta_any instance, nlohmann::json &j);

    void load_component(const nlohmann::json &j, entt::meta_any &instance);

    void emplace_component(entt::registry &registry, entt::entity entity, entt::meta_any &any);

    void SceneSaver::serialize_entity(nlohmann::json &j, Entity entity)
    {
        using namespace entt::literals;
        nlohmann::json j_components = nlohmann::json::object();
        for (auto [id, set]: _scene->get_raw_registry().storage())
        {
            if (!set.contains(entity))
            {
                continue;
            }

            const auto hash = set.type().hash();
            if (!ComponentRegistry::get_component_map().contains(hash))
            {
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
        j["components"] = j_components;

        if (entity.has_component<ParentComponent>())
        {
            nlohmann::json j_children = nlohmann::json::array();
            auto& parent_component = entity.get_component<ParentComponent>();
            for (const auto& child_uuid : parent_component.children)
            {
                Entity child = _scene->get_entity_by_uuid(child_uuid);
                if (child)
                {
                    nlohmann::json j_child;
                    serialize_entity(j_child, child);
                    j_children.push_back(j_child);
                }
            }
            j["children"] = j_children;
        }
    }

    Entity SceneSaver::deserialize_entity(nlohmann::json &j, std::unordered_map<UUID, UUID> &old_to_new_uuid_map)
    {
        using namespace entt::literals;

        Entity new_entity = _scene->create_entity();
        UUID old_id;

        for (auto& [type_name, j_component_data] : j.at("components").items())
        {
            if (auto meta_type = entt::resolve(entt::hashed_string(type_name.c_str())))
            {
                auto instance = meta_type.construct();
                if (!instance)
                {
                    continue;
                }

                if (auto func = instance.type().func("deserialize"_hs); func)
                {
                    func.invoke(instance, instance.as_ref(), entt::forward_as_meta(j_component_data).as_ref());
                }
                else
                {
                    load_component(j_component_data, instance);
                }

                if (type_name == "IDComponent")
                {
                    old_id = instance.cast<IDComponent>().id;
                    continue;
                }
                emplace_component(_scene->get_raw_registry(), new_entity, instance);
            }
        }

        old_to_new_uuid_map[old_id] = new_entity.get_uuid();

        if (new_entity.has_component<ChildComponent>())
        {
            auto& child_comp = new_entity.get_component<ChildComponent>();
            UUID old_parent =child_comp.parent;
            if (old_to_new_uuid_map.contains(old_parent))
            {
                child_comp.parent = old_to_new_uuid_map.at(old_parent);
            }
        }

        if (new_entity.has_component<ParentComponent>())
        {
            auto& pc = new_entity.get_component<ParentComponent>();
            std::vector<UUID> new_children_uuids;
            new_children_uuids.reserve(pc.children.size());
            pc.children.clear();
        }

        if (j.contains("children"))
        {
            for (auto& j_child : j["children"])
            {
                Entity child_entity = deserialize_entity(j_child, old_to_new_uuid_map);

                new_entity.get_component<ParentComponent>().children.push_back(child_entity.get_uuid());
            }
        }
        return new_entity;
    }

    void Prefab::create(Entity entity, const std::string &path)
    {
        SceneSaver saver(entity._scene);
        nlohmann::json j;
        saver.serialize_entity(j, entity);

        FileUtil::create_directory_recursive(path);
        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("error opening file for serialization :(");
            return;
        }
        file << j.dump(4);
        file.close();
    }

    Entity Prefab::instantiate(Scene *scene, const std::string &path)
    {
        if (!FileUtil::file_exists(path))
        {
            LOG_ERROR("No file at %s", path.c_str());
            return {};
        }

        std::ifstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("Error opening file at %s", path.c_str());
            return {};
        }
        nlohmann::json data = nlohmann::json::parse(file);

        SceneSaver saver(scene);
        std::unordered_map<UUID, UUID> uuid_map;
        return saver.deserialize_entity(data, uuid_map);
    }
}
