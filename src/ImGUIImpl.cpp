#pragma once

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_impl_dx11.cpp"
#include "imgui/imgui_impl_win32.cpp"

bool imgui_hovering_anything() {
  ImGuiHoveredFlags flags = ImGuiHoveredFlags_AnyWindow;
  return ImGui::IsWindowHovered(flags);
}
