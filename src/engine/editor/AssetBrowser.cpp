//
// Created by alecpizz on 6/30/25.
//
#include "Editor.h"
#include <filesystem>
#include <engine/audio/Audio.h>

namespace cologne
{
    static const std::filesystem::path assets_directory = RESOURCES_PATH;
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

        if (current_directory != assets_directory)
        {
            if (ImGui::Button("<-"))
            {
                current_directory = current_directory.parent_path();
                Audio::play_sound(_move_sound, 30);
            }
        }

        static float padding = 0.6f;
        static float thumbnail_size = 38.0f;
        float cell_size = thumbnail_size / padding;

        const float panel_width = ImGui::GetContentRegionAvail().x;
        int column_count = static_cast<int>(panel_width / cell_size);
        if (column_count < 1)
        {
            column_count = 1;
        }

        ImGui::Columns(column_count, 0, false);

        for (auto &dir_entry: std::filesystem::directory_iterator(current_directory))
        {
            const auto &path = dir_entry.path();
            std::string filename_str = path.filename().string();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            uint32_t handle = dir_entry.is_directory() ? folder_texture.get_handle() : icon_texture.get_handle();
            ImGui::ImageButton(filename_str.c_str(), static_cast<ImTextureID>(static_cast<intptr_t>(handle)),
                               {thumbnail_size, thumbnail_size}, {0, 1}, {1, 0});

            if (ImGui::BeginDragDropSource())
            {
                std::filesystem::path relative_path(path);
                const auto item_path = relative_path.c_str();
                ImGui::SetDragDropPayload("ASSET_BROWSER_ENTRY", item_path, strlen(item_path) * sizeof(char));
                ImGui::Text("%s", item_path);
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (dir_entry.is_directory())
                {
                    current_directory /= path.filename();
                    Audio::play_sound(_move_sound, 30);
                }
            }
            ImGui::TextWrapped(filename_str.c_str());
            ImGui::NextColumn();
        }

        ImGui::Columns(1);
        ImGui::SliderFloat("Thumbnail Size", &thumbnail_size, 16, 512);
        ImGui::SliderFloat("Padding", &padding, 0, 32);

        ImGui::End();
    }
}
