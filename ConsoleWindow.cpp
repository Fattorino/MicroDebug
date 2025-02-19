#include "ConsoleWindow.h"

ConsoleWindow::ConsoleWindow() {
    m_title = "Console " + std::to_string(microDebug.getConsoles().size());
    m_autoScroll = true;
}

void ConsoleWindow::draw() {
    ImGui::Begin(m_title.c_str());
    // Right click menu
    if (ImGui::BeginPopupContextItem()) {
        ImGui::Checkbox("Auto-scroll", &m_autoScroll);
        if (ImGui::BeginCombo("Stream", m_dataStream ? m_dataStream->getName().c_str() : "None")) {
            for (auto& dataStream : microDebug.getDataStreams()) {
                if (ImGui::Selectable(dataStream.getName().c_str(), m_dataStream == &dataStream)) {
                    m_dataStream = &dataStream;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::EndPopup();
    }

    m_filter.Draw("##Filter", -1);
    ImGui::Separator();

    if (ImGui::BeginChild("scrolling", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar) && m_dataStream) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char *buf = m_dataStream->getBuffer().begin();
        const char *buf_end = m_dataStream->getBuffer().end();
        if (m_filter.IsActive()) {
            const ImVector<int>& lineOffsets = m_dataStream->getLineOffsets();
            for (int line_no = 0; line_no < lineOffsets.Size; line_no++) {
                const char *line_start = buf + lineOffsets[line_no];
                const char *line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
                if (m_filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        } else {
            ImGui::TextUnformatted(buf, buf_end);
        }
        ImGui::PopStyleVar();

        if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();
}
