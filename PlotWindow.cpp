#include "PlotWindow.h"

PlotWindow::PlotWindow() {
    m_name = "Plot " + std::to_string(microDebug.getPlots().size() + 1);
    m_plotTitle = m_name;
}

void PlotWindow::draw() {
    ImGui::Begin(m_name.c_str());
    // Right click menu
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::BeginCombo("Stream", m_dataStream ? m_dataStream->getName().c_str() : "None")) {
            for (auto& dataStream : microDebug.getDataStreams()) {
                if (ImGui::Selectable(dataStream.getName().c_str(), m_dataStream == &dataStream)) {
                    m_dataStream = &dataStream;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Combo("Mode", reinterpret_cast<int*>(&m_plotMode), "Window\0Free\0Full\0\0");

        if (m_plotMode == PlotMode_WINDOW) {
            ImGui::InputFloat("Window start", &m_windowStart);
        }

        ImGui::Checkbox("##ShowTitle", &m_showTitle);
        ImGui::SameLine();
        ImGui::BeginDisabled(!m_showTitle);
        ImGui::InputText("Title", &m_plotTitle);
        ImGui::EndDisabled();

        ImGui::Checkbox("Show list", &m_showPlotItemList);

        ImGui::Separator();
        if (ImGui::Button("Clear X axis")) {
            m_xAxis = nullptr;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear all")) {
            m_xAxis = nullptr;
            m_yAxis.clear();
        }

        ImGui::EndPopup();
    }

    // Quick combo for first setup
    if (!m_dataStream && ImGui::BeginCombo("Stream", m_dataStream ? m_dataStream->getName().c_str() : "None")) {
        for (auto& dataStream : microDebug.getDataStreams()) {
            if (ImGui::Selectable(dataStream.getName().c_str(), m_dataStream == &dataStream)) {
                m_dataStream = &dataStream;
            }
        }
        ImGui::EndCombo();
    }

    if (!m_dataStream || m_dataStream->getPlotItems().empty()) {
        if (m_xAxis) {
            m_xAxis = nullptr;
        }
        m_yAxis.clear();
        ImGui::End();
        return;
    }

    if (m_showPlotItemList) {
        ImGui::Columns(2);
        if (!aux_init_col_width) {
            aux_init_col_width = true;
            ImGui::SetColumnWidth(-1, ImGui::GetWindowWidth() * 0.1f);
        }

        ImGui::BeginChild("PlotItemsList");
        m_plotItemFilter.Draw("##Filter", -1);
        ImGui::Separator();
        for (int i = 0; i < m_dataStream->getPlotItems().size(); i++) {
            auto &pi = m_dataStream->getPlotItems()[i];
            if (!m_plotItemFilter.PassFilter(pi.name.c_str())) {
                continue;
            }
            ImPlot::ItemIcon(pi.color);
            ImGui::SameLine();
            ImGui::Selectable(pi.name.c_str());
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                ImGui::SetDragDropPayload("PlotItem", &i, sizeof(PlotItem));
                ImPlot::ItemIcon(pi.color);
                ImGui::SameLine();
                ImGui::TextUnformatted(pi.name.c_str());
                ImGui::EndDragDropSource();
            }
            if (ImGui::BeginPopupContextItem()) {
                ImGui::ColorEdit3("Color", &pi.color.x);
                ImGui::Combo("Axis", reinterpret_cast<int *>(&pi.axis), "Y1\0Y2\0Y3\0\0");
                if (ImGui::InputFloat2("Scaling (Y/X)", &pi.auxScaling.x)) {
                    for (int j = 0; j < pi.data.size(); j++) {
                        pi.data[j] /= pi.scaling.x;
                        pi.data[j] *= pi.auxScaling.x;
                    }
                    for (int j = 0; j < pi.support.size(); j++) {
                        pi.support[j] /= pi.scaling.x;
                        pi.support[j] *= pi.auxScaling.x;
                    }
                    pi.scaling = pi.auxScaling;
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("PlotItemRemove")) {
                int i = *static_cast<int *>(payload->Data);
                m_yAxis.erase(m_yAxis.begin() + i);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::NextColumn();
    }

    switch (m_plotMode) {
        case PlotMode_WINDOW: {
            if (!m_xAxis) break;
            float v_max = m_xAxis->data.back();
            float v_min;
            if (m_windowStart < 0) {
                v_min = v_max + m_windowStart;
            } else {
                v_min = m_windowStart;
            }
            ImPlot::SetNextAxisLimits(ImAxis_X1, v_min, v_max, ImGuiCond_Always);
            break; }
        case PlotMode_FULL:
            ImPlot::SetNextAxesToFit();
            break;
        default:
            break;
    }

    if (m_plotTitle.empty()) { m_plotTitle = m_name; }
    if (ImPlot::BeginPlot(m_plotTitle.c_str(), ImVec2(-1, -1), !m_showTitle)) { // when false, !m_showTitle -> 1 -> ImPlotFlags_NoTitle
        ImPlot::SetupAxis(ImAxis_X1, m_xAxis == nullptr ? "[drop here]" : m_xAxis->name.c_str());
        for (PlotItem* p : m_yAxis) {
            if (p->axis == 1) {
                ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_Opposite);
                break;
            }
        }
        for (PlotItem* p : m_yAxis) {
            if (p->axis == 2) {
                ImPlot::SetupAxis(ImAxis_Y3, nullptr, ImPlotAxisFlags_Opposite);
                break;
            }
        }

        for (int i = 0; i < m_yAxis.size(); i++) {
            PlotItem* p = m_yAxis[i];
            if (m_xAxis) {
                int count = (int)std::min(m_xAxis->data.size(), p->data.size());
                ImPlot::SetNextLineStyle(p->color);
                ImPlot::SetAxis(ImAxis_Y1 + p->axis);
                ImPlot::PlotLine(p->name.c_str(), &m_xAxis->data[0], &p->data[0], count);
            } else if (!p->support.empty()) {
                int count = (int)std::min(p->support.size(), p->data.size());
                ImPlot::SetNextLineStyle(p->color);
                ImPlot::SetAxis(ImAxis_Y1 + p->axis);
                ImPlot::PlotLine(p->name.c_str(), &p->support[0], &p->data[0], count);
            } else {
                ImPlot::SetNextLineStyle(p->color);
                ImPlot::SetAxis(ImAxis_Y1 + p->axis);
                ImPlot::PlotLine(p->name.c_str(), &p->data[0], p->data.size());
            }

            // allow legend item labels to be DND sources
            if (ImPlot::BeginDragDropSourceItem(p->name.c_str())) {
                ImGui::SetDragDropPayload("PlotItemRemove", &i, sizeof(int));
                ImPlot::ItemIcon(p->color); ImGui::SameLine();
                ImGui::TextUnformatted(p->name.c_str());
                ImPlot::EndDragDropSource();
            }
        }

        // allow the x-axis to be a DND target
        if (ImPlot::BeginDragDropTargetAxis(ImAxis_X1)) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PlotItem")) {
                int i = *static_cast<int*>(payload->Data);
                m_xAxis = &m_dataStream->getPlotItems()[i];
            }
            ImPlot::EndDragDropTarget();
        }

        // allow the y-axis and the plot area to be a DND target
        if (ImPlot::BeginDragDropTargetAxis(ImAxis_Y1) || ImPlot::BeginDragDropTargetPlot()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PlotItem")) {
                int i = *static_cast<int*>(payload->Data);
                PlotItem* p = &m_dataStream->getPlotItems()[i];
                if (std::find(m_yAxis.begin(), m_yAxis.end(), p) == m_yAxis.end()) {
                    m_yAxis.push_back(p);
                }
            }
            ImPlot::EndDragDropTarget();
        }
        ImPlot::EndPlot();
    }

    if (m_showPlotItemList) {
        ImGui::Columns(1);
    }
    ImGui::End();
}

void PlotWindow::linkStream(DataStream* dataStream) {
    m_dataStream = dataStream;
}
