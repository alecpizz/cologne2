//
// Created by alecpizz on 8/25/25.
//
#include <engine/editor/Editor.h>

namespace cologne
{
    struct ImageCmd
    {
        uint32_t id;
        std::string name;
        glm::vec2 image_size;
        bool flip = true;
    };

    std::vector<ImageCmd> image_cmds;

    void Editor::add_image_entry(const char *name, uint32_t value, const glm::vec2 &image_size)
    {
        for (auto &image_cmd: image_cmds)
        {
            if (image_cmd.name == name)
            {
                image_cmd.id = value;
                image_cmd.image_size = image_size;
                return;
            }
        }
        image_cmds.emplace_back(ImageCmd{value, name, image_size});
    }

    void Editor::build_images_window()
    {
        static int selected_image_idx = -1;
        if (_image_window_active && ImGui::Begin("Images", &_image_window_active))
        {
            ImGui::Columns(2, "images_columns");
            ImGui::SetColumnWidth(0, 250.0f);
            ImGui::BeginChild("##image_list", ImVec2(0, 0));
            for (int i = 0; i < image_cmds.size(); i++)
            {
                ImGuiTreeNodeFlags flags = (selected_image_idx == i ? ImGuiTreeNodeFlags_Selected : 0) |
                                           ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;
                bool opened = ImGui::TreeNodeEx(image_cmds[i].name.c_str(), flags);
                if (ImGui::IsItemClicked())
                {
                    selected_image_idx = i;
                }
                if (opened)
                {
                    ImGui::TreePop();
                }
            }
            ImGui::EndChild();

            ImGui::NextColumn();
            ImGui::BeginChild("##image_viewport", ImVec2(0, 0), 0,
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            if (selected_image_idx > 0 && selected_image_idx < image_cmds.size())
            {
                ImGui::Text("Label: %s", image_cmds[selected_image_idx].name.c_str());
                ImGui::Text("Dimensions: %f %f", image_cmds[selected_image_idx].image_size.x,
                            image_cmds[selected_image_idx].image_size.y);
                static float image_scale = 1.0f;
                ImGui::SliderFloat("Scale", &image_scale, 0.1f, 5.0f);
                ImGui::Separator();

                float available_width = ImGui::GetContentRegionAvail().x;
                ImVec2 original_size = ImVec2(image_cmds[selected_image_idx].image_size.x, image_cmds[selected_image_idx].image_size.y);
                ImVec2 new_size = original_size;
                if (original_size.x > 0)
                {
                    float aspect_ratio = original_size.y / original_size.x;
                    new_size.x = available_width;
                    new_size.y = available_width * aspect_ratio;
                }

                new_size.x *= image_scale;
                new_size.y *= image_scale;

                ImVec2 available_region = ImGui::GetContentRegionAvail();
                ImVec2 cursor_pos = ImGui::GetCursorPos();

                float offset_x = (available_region.x - new_size.x) * 0.5f;
                float offset_y = (available_region.y - new_size.y) * 0.5f;

                // Ensure offsets are not negative
                if (offset_x > 0)
                {
                    ImGui::SetCursorPosX(cursor_pos.x + offset_x);
                }
                if (offset_y > 0)
                {
                    ImGui::SetCursorPosY(cursor_pos.y + offset_y);
                }

                ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(image_cmds[selected_image_idx].id)),
                             new_size, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::Separator();
            }
            else
            {
                ImGui::Text("Select an image to view it!");
            }
            ImGui::EndChild();
            ImGui::Columns(1);
            ImGui::End();
        }
    }
}
