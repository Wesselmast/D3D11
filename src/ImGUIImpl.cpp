#pragma once

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_impl_dx11.cpp"
#include "imgui/imgui_impl_win32.cpp"

ImGuiWindowFlags imgui_static_window_flags() {
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoScrollbar;
    flags |= ImGuiWindowFlags_NoCollapse;
    return flags;
}

bool32 imgui_hovering_anything() {
  return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

bool32 imgui_focusing_anything() {
  return ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
}
