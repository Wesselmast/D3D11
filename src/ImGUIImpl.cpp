#pragma once

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_impl_dx11.cpp"
#include "imgui/imgui_impl_win32.cpp"

ImGuiWindowFlags imgui_default_window_flags() {
  ImGuiWindowFlags flags = 0;
  return flags;
}

ImGuiWindowFlags imgui_noinput_window_flags() {
  ImGuiWindowFlags flags = imgui_default_window_flags();
  flags |= ImGuiWindowFlags_NoInputs;
  return flags;
}

ImGuiWindowFlags imgui_static_window_flags() {
  ImGuiWindowFlags flags = imgui_default_window_flags();

  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoScrollbar;
  flags |= ImGuiWindowFlags_NoCollapse;
  return flags;
}

ImGuiWindowFlags imgui_static_noinput_window_flags() {
  ImGuiWindowFlags flags = imgui_static_window_flags();
  flags |= ImGuiWindowFlags_NoInputs;
  return flags;
}

bool32 imgui_hovering_anything() {
  return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

bool32 imgui_focusing_anything() {
  return ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
}

void imgui_popup_window(const char* name, ImVec2 position, const char* text) {
  const ImVec2 nws = { 200, 80 };
  const ImVec2 bts = { 150, 20 };
  ImGui::SetNextWindowPos(position - (nws * 0.5f));
  ImGui::SetNextWindowSize(nws);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoScrollbar;
  flags |= ImGuiWindowFlags_NoCollapse;

  bool imgui_popup_open = true;
  if(ImGui::BeginPopupModal(name, &imgui_popup_open, flags)) {
    ImGui::TextWrapped("%s", text);
    ImGui::NewLine();

    ImGui::SetCursorPosX(nws.x * 0.5f - bts.x * 0.5f);
    ImGui::SetCursorPosY(nws.y - bts.y * 1.5f);
    if(ImGui::Button("OK", bts)) ImGui::CloseCurrentPopup();
    ImGui::SetCursorPos({0.0f, 0.0f});

    ImGui::EndPopup();
  }
}

static int32 filter_username(ImGuiTextEditCallbackData* data) { 
  ImWchar& c = data->EventChar;
  if ((c >= 'A' && c <= 'Z') ||
      (c >= 'a' && c <= 'z') ||
      (c >= '0' && c <= '9')) { 
    return 0;
  }
  return 1;
}
