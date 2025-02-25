#pragma once

#include <imgui.h>

#include "MicroDebug.h"
#include "DataStream.h"

class DataStream;

class ConsoleWindow {
public:
    ConsoleWindow();
    void draw();
    void linkStream(DataStream* dataStream);

private:
    std::string m_title;
    bool m_autoScroll;
    ImGuiTextFilter m_filter;
    DataStream* m_dataStream = nullptr;
};
