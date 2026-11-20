#ifndef IMGUI_BUTTON_CHECKED_H
#define IMGUI_BUTTON_CHECKED_H

#include "imgui.h"

namespace ImGui
{
    // Button drawn with the "active" color while checked (used for tab-like selectors)
    inline bool ButtonChecked(const char* label, bool checked, const ImVec2& size = ImVec2(0, 0))
    {
        if (checked)
        {
            const ImVec4 activeColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
            ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeColor);
        }

        const bool pressed = ImGui::Button(label, size);

        if (checked)
        {
            ImGui::PopStyleColor(2);
        }

        return pressed;
    }
}

#endif // IMGUI_BUTTON_CHECKED_H
