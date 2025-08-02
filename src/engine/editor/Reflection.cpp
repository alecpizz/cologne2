//
// Created by alecpizz on 7/30/25.
//
#include "Editor.h"
#include "imgui.h"
#include <misc/cpp/imgui_stdlib.h>

namespace cologne
{
    using namespace entt::literals;
    using PropertiesMap = std::unordered_map<entt::id_type, entt::meta_any>;

    template <typename Scalar>
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

    template <typename Scalar>
    consteval const char* scalar_to_format()
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

    static void get_editor_name(const char*& label, const PropertiesMap& properties)
    {
        if (auto it = properties.find("name"_hs); it != properties.end())
        {
            label = *it->second.try_cast<const char*>();
        }
    }

    template <typename Scalar>
    static bool editor_write_scalar(Scalar& f, const PropertiesMap& properties)
    {
        const char* label = "float";
        get_editor_name(label, properties);
        if constexpr (std::is_same_v<Scalar, bool>)
        {
            return ImGui::Checkbox(label, &f);
        }
        else
        {
            return ImGui::DragScalar(label, scalar_to_imgui_data_type<Scalar>(), &f);
        }
    }

    template <typename Scalar>
    static void editor_read_scalar(Scalar s, const PropertiesMap& prop)
    {
        const char* label = "scalar";
        get_editor_name(label, prop);
        ImGui::Text((std::string("%s: ") + scalar_to_format<Scalar>()).c_str(), label, s);
    }

    static bool write_vec3(glm::vec3& v)
    {
        const char* lable = "vec3";
        return ImGui::DragFloat3(lable, glm::value_ptr(v));
    }

    static void read_vec3(glm::vec3 v, const PropertiesMap& properties)
    {
        const char* label = "vec3";
        get_editor_name(label, properties);
        ImGui::Text("%s: %f, %f, %f", label, v.x, v.y, v.z);
    }

    static bool write_vec4(glm::vec4& v)
    {
        const char* label = "vec4";
        return ImGui::DragFloat4(label, glm::value_ptr(v));
    }

    static void read_vec4(glm::vec4 v, const PropertiesMap& properties)
    {
        const char* label = "vec4";
        get_editor_name(label, properties);
        ImGui::Text("%s: %f %f %f %f", label, v.x, v.y, v.z, v.w);
    }

    static bool write_quat(glm::quat& q)
    {
        const char* label = "quat";
        auto euler = glm::degrees(glm::eulerAngles(q));
        bool changed = ImGui::DragFloat3(label, glm::value_ptr(euler));
        if (changed)
        {
            q = glm::quat(glm::radians(euler));
        }
        return changed;
    }

    static void read_quat(glm::quat q, const PropertiesMap& properties)
    {
        const char* label = "quat";
        get_editor_name(label, properties);
        ImGui::Text("%s: %f %f %f %f", label, q.x, q.y, q.z, q.w);
    }

    static bool write_string(std::string& s, const PropertiesMap& properties)
    {
        const char* label = "string";
        get_editor_name(label, properties);
        return ImGui::InputText(label, &s, ImGuiInputTextFlags_EnterReturnsTrue);
    }

    static void read_string(const std::string& s, const PropertiesMap& properties)
    {
        const char* label = "string";
        get_editor_name(label, properties);
        ImGui::Text("%.*s", static_cast<int>(s.size()), s.c_str());
    }

    void Editor::initialize_reflection_editor()
    {
        entt::meta_factory<int>()
            .func<&editor_write_scalar<int>>("editor_write"_hs)
            .func<&editor_read_scalar<int>>("editor_read"_hs);
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
    }
}
