#include "MicroDebug.h"

MicroDebug microDebug;

void Settings::dataStreamSettingsMenu() {
    if (ImGui::BeginMenu("Settings")) {
        ImGui::InputText("Reset Command", &m_resetCmd);
        ImGui::Checkbox("Clear buffer on reset cmd", &m_clearBuffOnReset);
        ImGui::EndMenu();
    }
}

void Settings::viewsSettingsMenu() {
    if (ImGui::BeginMenu("Settings")) {
        ImGui::Text("No settings yet");
        ImGui::EndMenu();
    }
}

// =================================================================================================

MicroDebug::MicroDebug() {
    m_dataStreams.emplace_back("Stream 1");
    m_consoles.emplace_back();
    m_consoles.back().linkStream(&m_dataStreams.back());
}

void MicroDebug::dataStreamsWindow() {
    ImGui::Begin("Data streams");
    for (auto& ds : m_dataStreams) {
        ds.draw();
        ImGui::Spacing(); ImGui::Separator(); ImGui::Separator(); ImGui::Spacing();
    }
    ImGui::End();
}

void MicroDebug::pollDataStreams() {
    for (auto& ds : m_dataStreams) {
        ds.poll();
    }
}

void MicroDebug::viewArea(ImGuiID dock_id) {
    for (auto& c : m_consoles) {
        ImGui::SetNextWindowDockID(dock_id, ImGuiCond_Once);
        c.draw();
    }
    for (auto& p : m_plots) {
        ImGui::SetNextWindowDockID(dock_id, ImGuiCond_Once);
        p.draw();
    }
}

void MicroDebug::addDataStream() {
    m_dataStreams.emplace_back("Stream " + std::to_string(m_dataStreams.size() + 1));
}

void MicroDebug::addConsole() {
    m_consoles.emplace_back();
}

void MicroDebug::addPlot() {
    m_plots.emplace_back();
}
