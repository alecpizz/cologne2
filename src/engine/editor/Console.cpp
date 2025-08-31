//
// Created by alecpizz on 8/30/25.
//
#include <engine/editor/Editor.h>

namespace cologne
{
    ImGuiTextFilter console_filter;

    void Editor::build_console()
    {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Console");

        console_filter.Draw("Filter", -100.0f);
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            std::shared_lock lock(_records_mutex);
            _records.clear();
        }
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar); {
            constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner;
            if (ImGui::BeginTable("Console_table", 1, table_flags))
            {
                std::shared_lock lock(_records_mutex);
                for (const auto &record: _records)
                {
                    if (console_filter.PassFilter(record.message.c_str()))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushStyleColor(ImGuiCol_Text, record.color);
                        ImGui::TextUnformatted(record.message.c_str());
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::EndTable();
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
