//
// Created by alecpizz on 7/30/25.
//
#include <engine/asset_manager/AssetManager.h>

#include "Editor.h"
#include "imgui.h"
#include <misc/cpp/imgui_stdlib.h>

namespace cologne
{
    using namespace entt::literals;
    using PropertiesMap = std::unordered_map<entt::id_type, entt::meta_any>;

    template<typename Scalar>
    consteval ImGuiDataType scalar_to_imgui_data_type()
    {
        if constexpr (std::is_same_v<Scalar, int8_t>)
        {
            return ImGuiDataType_S8;
        }
        if constexpr (std::is_same_v<Scalar, uint8_t>)
        {
            return ImGuiDataType_U8;
        }
        if constexpr (std::is_same_v<Scalar, int16_t>)
        {
            return ImGuiDataType_S16;
        }
        if constexpr (std::is_same_v<Scalar, uint16_t>)
        {
            return ImGuiDataType_U16;
        }
        if constexpr (std::is_same_v<Scalar, int32_t>)
        {
            return ImGuiDataType_S32;
        }
        if constexpr (std::is_same_v<Scalar, uint32_t>)
        {
            return ImGuiDataType_U32;
        }
        if constexpr (std::is_same_v<Scalar, int64_t>)
        {
            return ImGuiDataType_S64;
        }
        if constexpr (std::is_same_v<Scalar, uint64_t>)
        {
            return ImGuiDataType_U64;
        }
        if constexpr (std::is_same_v<Scalar, float>)
        {
            return ImGuiDataType_Float;
        }
        if constexpr (std::is_same_v<Scalar, double>)
        {
            return ImGuiDataType_Double;
        }

        throw "Error: unsupported type";
    }

    template<typename Scalar>
    consteval const char *scalar_to_format()
    {
        if constexpr (std::is_same_v<Scalar, bool>)
        {
            return "%d";
        }
        if constexpr (std::is_same_v<Scalar, int8_t>)
        {
            return "%d";
        }
        if constexpr (std::is_same_v<Scalar, uint8_t>)
        {
            return "%u";
        }
        if constexpr (std::is_same_v<Scalar, int16_t>)
        {
            return "%d";
        }
        if constexpr (std::is_same_v<Scalar, uint16_t>)
        {
            return "%u";
        }
        if constexpr (std::is_same_v<Scalar, int32_t>)
        {
            return "%d";
        }
        if constexpr (std::is_same_v<Scalar, uint32_t>)
        {
            return "%u";
        }
        if constexpr (std::is_same_v<Scalar, int64_t>)
        {
            return "%lld";
        }
        if constexpr (std::is_same_v<Scalar, uint64_t>)
        {
            return "%llu";
        }
        if constexpr (std::is_same_v<Scalar, float>)
        {
            return "%.3f";
        }
        if constexpr (std::is_same_v<Scalar, double>)
        {
            return "%.3f";
        }

        throw "Error: unsupported type";
    }

    static std::string format_snake_case_member(const std::string &input)
    {
        if (input.empty())
        {
            return "";
        }

        std::string result;
        result.reserve(input.length());
        bool capitalize = true;
        for (const char c: input)
        {
            if (c == '_')
            {
                result += ' ';
                capitalize = true;
            }
            else
            {
                if (capitalize)
                {
                    result += static_cast<char>(std::toupper(c));
                    capitalize = false;
                }
                else
                {
                    result += c;
                }
            }
        }
        return result;
    }

    static std::string get_editor_name(const std::string &label, const PropertiesMap &properties)
    {
        std::string label_str = label;
        if (const auto it = properties.find("name"_hs); it != properties.end())
        {
            if (const auto str = *it->second.try_cast<const char *>())
            {
                label_str = str;
            }
        }
        return format_snake_case_member(label_str);
    }

    template<typename Scalar>
    static bool editor_write_scalar(Scalar &f, const PropertiesMap &properties)
    {
        const char *label = "float";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        if constexpr (std::is_same_v<Scalar, bool>)
        {
            return ImGui::Checkbox(name, &f);
        }
        else
        {
            return ImGui::DragScalar(name, scalar_to_imgui_data_type<Scalar>(), &f, 0.01, nullptr, nullptr, "%.6f");
        }
    }

    template<typename Scalar>
    static void editor_read_scalar(Scalar s, const PropertiesMap &properties)
    {
        const char *label = "scalar";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        ImGui::Text((std::string("%s: ") + scalar_to_format<Scalar>()).c_str(), name, s);
    }

    static bool write_vec3(glm::vec3 &v, const PropertiesMap &properties)
    {
        const char *label = "vec3";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        return ImGui::DragFloat3(name, glm::value_ptr(v), 0.01f, 0, 0, "%.6f");
    }

    static void read_vec3(glm::vec3 v, const PropertiesMap &properties)
    {
        const char *label = "vec3";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        ImGui::Text("%s: %f, %f, %f", name, v.x, v.y, v.z);
    }

    static bool write_vec4(glm::vec4 &v, const PropertiesMap &properties)
    {
        const char *label = "vec4";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        return ImGui::DragFloat4(name, glm::value_ptr(v));
    }

    static void read_vec4(glm::vec4 v, const PropertiesMap &properties)
    {
        const char *label = "vec4";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        ImGui::Text("%s: %f %f %f %f", name, v.x, v.y, v.z, v.w);
    }

    static bool write_color(Color &color, const PropertiesMap &properties)
    {
        const char *label = "color";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        return ImGui::ColorEdit4(name, glm::value_ptr(color.color));
    }

    static void read_color(Color c, const PropertiesMap& properties)
    {
        const char *label = "color";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        ImGui::ColorEdit4(name, glm::value_ptr(c.color));
    }

    static bool write_quat(glm::quat &q, const PropertiesMap &properties)
    {
        const char *label = "quat";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        auto euler = glm::degrees(glm::eulerAngles(q));
        bool changed = ImGui::DragFloat3(name, glm::value_ptr(euler));
        if (changed)
        {
            q = glm::quat(glm::radians(euler));
        }
        return changed;
    }

    static void read_quat(glm::quat q, const PropertiesMap &properties)
    {
        const char *label = "quat";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        ImGui::Text("%s: %f %f %f %f", name, q.x, q.y, q.z, q.w);
    }

    static bool write_string(std::string &s, const PropertiesMap &properties)
    {
        const char *label = "string";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        return ImGui::InputText(name, &s, ImGuiInputTextFlags_EnterReturnsTrue);
    }

    static void read_string(const std::string &s, const PropertiesMap &properties)
    {
        const char *label = "string";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        ImGui::LabelText(name, "%.*s", static_cast<int>(s.size()), s.c_str());
    }

    static bool write_mat4(glm::mat4 &mat, const PropertiesMap &properties)
    {
        const char *label = "mat4";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        bool c0 = ImGui::DragFloat4(std::string(std::string(name) + "col 0").c_str(), &mat[0][0]);
        bool c1 = ImGui::DragFloat4(std::string(std::string(name) + "col 1").c_str(), &mat[1][0]);
        bool c2 = ImGui::DragFloat4(std::string(std::string(name) + "col 2").c_str(), &mat[2][0]);
        bool c3 = ImGui::DragFloat4(std::string(std::string(name) + "col 3").c_str(), &mat[3][0]);
        return c0 || c1 || c2 || c3;
    }

    static bool write_uuid(UUID &uuid, const PropertiesMap& properties)
    {
        const char *label = "uuid";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        return ImGui::DragScalar(name, ImGuiDataType_U64, &uuid._uuid);
    }

    static bool write_light(LightComponent &light, const PropertiesMap &properties)
    {
        bool changed = false;
        const char *items[] = {"Directional", "Point", "Spot"};
        int type = light.type;
        if (ImGui::Combo("Light Type", &type, items, 3))
        {
            light.type = type;
            changed = true;
        }
        if (light.type == LightComponent::Spot)
        {
            changed |= ImGui::DragFloat("outer cutoff", &light.outer_cutoff, 0.01f);
            changed |= ImGui::DragFloat("inner cutoff", &light.inner_cutoff, 0.01f);
        }
        changed |= ImGui::Checkbox("Cast Shadows", &light.cast_shadows);
        changed |= ImGui::Checkbox("Always Update Shadows", &light.always_update_shadows);
        changed |= ImGui::DragFloat("Radius", &light.radius, 0.01f);
        changed |= ImGui::DragFloat("Strength", &light.strength, 0.01f);
        changed |= ImGui::ColorEdit3("Color", glm::value_ptr(light.color), ImGuiColorEditFlags_HDR
                                                                           | ImGuiColorEditFlags_Float);
        return changed;
    }

    static void read_light(const LightComponent &light, const PropertiesMap &properties)
    {
    }

    static void read_mat4(const glm::mat4 &mat, const PropertiesMap &properties)
    {
        const char *label = "mat4";
        const auto name_str = get_editor_name(label, properties);
        const auto name = name_str.c_str();
        ImGui::Text("%s: %f %f %f %f", name, mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
        ImGui::Text("%s: %f %f %f %f", name, mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
        ImGui::Text("%s: %f %f %f %f", name, mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
        ImGui::Text("%s: %f %f %f %f", name, mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    }

    static bool write_model(ModelComponent &model, const PropertiesMap &properties)
    {
        std::vector<const char *> model_names;
        for (auto &model: AssetManager::get_models())
        {
            model_names.emplace_back(model.get_name());
        }
        int idx = AssetManager::get_model_index_by_name(model.model_name);

        if (ImGui::Combo("Model", &idx, model_names.data(), model_names.size()))
        {
            model.model_name = model_names[idx];
            return true;
        }
        return false;
    }

    static bool write_mesh(MeshComponent &mesh, const PropertiesMap &properties)
    {
        bool changed = false;
        std::vector<const char *> mesh_names;
        for (auto &mesh: AssetManager::get_meshes())
        {
            const std::string &mesh_name = mesh.get_name();
            mesh_names.emplace_back(mesh_name.c_str());
        }
        int idx = AssetManager::get_mesh_index_by_name(mesh.mesh_name);

        if (ImGui::Combo("Mesh", &idx, mesh_names.data(), mesh_names.size()))
        {
            mesh.mesh_name = mesh_names[idx];
            changed = true;
        }
        ImGui::TextUnformatted("Stats");
        ImGui::Text("Verts: %d", AssetManager::get_mesh_by_name(mesh.mesh_name)->get_vertices().size());
        ImGui::Text("Indices: %d", AssetManager::get_mesh_by_name(mesh.mesh_name)->get_indices_count());
        return changed;
    }

    static bool write_convex_mesh(ConvexMeshColliderComponent &mesh, const PropertiesMap &properties)
    {
        std::vector<const char *> mesh_names;
        for (auto &mesh: AssetManager::get_meshes())
        {
            const std::string &mesh_name = mesh.get_name();
            mesh_names.emplace_back(mesh_name.c_str());
        }
        int idx = AssetManager::get_mesh_index_by_name(mesh.mesh_name);

        if (ImGui::Combo("Mesh", &idx, mesh_names.data(), mesh_names.size()))
        {
            mesh.mesh_name = mesh_names[idx];
            return true;
        }
        return false;
    }

    static bool write_skinned_model_component(SkinnedModelComponent &skinned_model_component,
                                              const PropertiesMap &properties)
    {
        bool changed = false;
        if (ImGui::BeginCombo("Skinned Model", skinned_model_component.model_name.c_str()))
        {
            for (const auto &model: AssetManager::get_skinned_models())
            {
                auto name = model.get_name();
                if (ImGui::Selectable(name.c_str()))
                {
                    skinned_model_component.model_name = name;
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }

        return changed;
    }

    static bool write_anim_component(AnimatorComponent &anim_component, const PropertiesMap &properties)
    {
        bool changed = false;
        if (ImGui::BeginCombo("Source Clip Name", anim_component.source_clip_name.c_str()))
        {
            for (const auto &anim: AssetManager::get_animations())
            {
                auto name = anim.get_name();
                if (ImGui::Selectable(name.c_str()))
                {
                    anim_component.source_clip_name = name;
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::DragFloat("Blend Duration", &anim_component.blend_duration, 0.01f);

        return changed;
    }



    static bool editor_write_dummy()
    {
        return true;
    }

    static void editor_read_dummy()
    {
    }

    void Editor::initialize_reflection_editor()
    {
        entt::meta_factory<int>()
                .func<&editor_write_scalar<int>>("editor_write"_hs)
                .func<&editor_read_scalar<int>>("editor_read"_hs);
        entt::meta_factory<uint64_t>()
                .func<&editor_write_scalar<uint64_t>>("editor_write"_hs)
                .func<&editor_read_scalar<uint64_t>>("editor_read"_hs);
        entt::meta_factory<uint32_t>()
                .func<&editor_write_scalar<uint32_t>>("editor_write"_hs)
                .func<&editor_read_scalar<uint32_t>>("editor_read"_hs);
        entt::meta_factory<uint16_t>()
                .func<&editor_write_scalar<uint16_t>>("editor_write"_hs)
                .func<&editor_read_scalar<uint16_t>>("editor_read"_hs);
        entt::meta_factory<uint8_t>()
                .func<&editor_write_scalar<uint8_t>>("editor_write"_hs)
                .func<&editor_read_scalar<uint8_t>>("editor_read"_hs);
        entt::meta_factory<float>()
                .func<&editor_write_scalar<float>>("editor_write"_hs)
                .func<&editor_read_scalar<float>>("editor_read"_hs);
        entt::meta_factory<bool>()
                .func<&editor_write_scalar<bool>>("editor_write"_hs)
                .func<&editor_read_scalar<bool>>("editor_read"_hs);
        entt::meta_factory<glm::vec3>()
                .func<&read_vec3>("editor_read"_hs)
                .func<&write_vec3>("editor_write"_hs);
        entt::meta_factory<glm::quat>()
                .func<&read_quat>("editor_read"_hs)
                .func<&write_quat>("editor_write"_hs);
        entt::meta_factory<glm::vec4>()
                .func<&read_vec4>("editor_read"_hs)
                .func<&write_vec4>("editor_write"_hs);
        entt::meta_factory<std::string>()
                .func<&write_string>("editor_write"_hs)
                .func<&read_string>("editor_read"_hs);
        entt::meta_factory<glm::mat4>()
                .func<&write_mat4>("editor_write"_hs)
                .func<&read_mat4>("editor_read"_hs);

        entt::meta_factory<UUID>()
            .func<&write_uuid>("editor_read"_hs);

        entt::meta_factory<Color>()
            .func<&write_color>("editor_write"_hs)
            .func<&read_color>("editor_read"_hs);

        entt::meta_factory<TagComponent>()
                .func<&editor_read_dummy>("editor_read"_hs)
                .func<&editor_write_dummy>("editor_write"_hs);
        entt::meta_factory<LightComponent>()
                .func<&write_light>("editor_write"_hs)
                .func<&read_light>("editor_read"_hs);
        entt::meta_factory<ModelComponent>()
                .func<&write_model>("editor_write"_hs);
        entt::meta_factory<MeshComponent>()
                .func<&write_mesh>("editor_write"_hs);
        entt::meta_factory<ConvexMeshColliderComponent>()
                .func<&write_convex_mesh>("editor_write"_hs);

        entt::meta_factory<SkinnedModelComponent>()
                .func<&write_skinned_model_component>("editor_write"_hs);
        entt::meta_factory<AnimatorComponent>()
                .func<&write_anim_component>("editor_write"_hs);
    }
}
