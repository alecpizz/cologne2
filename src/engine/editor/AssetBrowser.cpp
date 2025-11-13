//
// Created by alecpizz on 6/30/25.
//
#include "Editor.h"
#include <filesystem>
#include <engine/audio/Audio.h>
#include <engine/renderer/types/Texture.h>

namespace cologne
{
    static const std::filesystem::path assets_directory = ASSETS_PATH;
    static std::filesystem::path current_directory = assets_directory;
    static Texture folder_texture;
    static Texture icon_texture;

    void Editor::build_asset_browser()
    {
        ImGui::Begin("Asset Browser", nullptr, _global_window_flags);
        if (folder_texture.get_handle() == 0)
        {
            folder_texture = Texture(RESOURCES_PATH "icons/folder.png");
        }

        if (icon_texture.get_handle() == 0)
        {
            icon_texture = Texture(RESOURCES_PATH "icons/file.png");
        }


        constexpr float thumbnail_size = 16.0f;
        const int column_count = 2;
        const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH;


        if (ImGui::BeginTable("Asset Browser", column_count, flags))
        {
            ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed, thumbnail_size);
            ImGui::TableSetupColumn("Filename", ImGuiTableColumnFlags_WidthStretch);

            if (current_directory / "" != assets_directory)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Selectable("...", false, ImGuiSelectableFlags_SpanAllColumns))
                {

                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    current_directory = current_directory.parent_path();
                    // Audio::play_sound(_move_sound, 30);
                }
            }

            for (auto &dir_entry: std::filesystem::directory_iterator(current_directory))
            {
                const auto &path = dir_entry.path();
                std::string filename_str = path.filename().string();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                uint32_t handle = dir_entry.is_directory() ? folder_texture.get_handle() : icon_texture.get_handle();
                ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(handle)),
                             {thumbnail_size, thumbnail_size}, {0, 1}, {1, 0});
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Selectable(filename_str.c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
                {

                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (dir_entry.is_directory())
                    {
                        current_directory /= path.filename();
                        // Audio::play_sound(_move_sound, 30);
                    }
                }

                if (ImGui::BeginDragDropSource())
                {
                    std::filesystem::path relative_path(path);
                    const auto item_path = relative_path.string().c_str();
                    ImGui::SetDragDropPayload("ASSET_BROWSER_ENTRY", item_path, strlen(item_path) * sizeof(char));
                    ImGui::Text("%s", item_path);
                    ImGui::EndDragDropSource();
                }
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }
}
