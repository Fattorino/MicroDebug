#pragma once

#include <imgui.h>
#include <implot.h>
#include <algorithm>

#include "MicroDebug.h"
#include "DataStream.h"

class DataStream;
struct PlotItem;

enum PlotMode {
    PlotMode_WINDOW,
    PlotMode_FREE,
    PlotMode_FULL
};

class PlotWindow {
public:
    PlotWindow();
    void draw();
    void linkStream(DataStream* dataStream);

private:
    std::string m_name;
    std::string m_plotTitle;
    bool m_showTitle = false;
    bool m_showPlotItemList = true;
    DataStream* m_dataStream = nullptr;
    PlotMode m_plotMode = PlotMode_WINDOW;
    float m_windowStart = -10.0f;
    PlotItem* m_xAxis = nullptr;
    std::vector<PlotItem*> m_yAxis;
    ImGuiTextFilter m_plotItemFilter;

    bool aux_init_col_width = false;
};
