#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <vector>
#include <array>
#include <list>
#include <thread>
#include <atomic>

#include "DataStream.h"
#include "ConsoleWindow.h"
#include "PlotWindow.h"

class DataStream;
class ConsoleWindow;
class PlotWindow;

class Settings {
public:
    void dataStreamSettingsMenu();
    void viewsSettingsMenu();

    [[nodiscard]] const std::string& getResetCmd() const { return m_resetCmd; }
    [[nodiscard]] bool clearBuffOnReset() const { return m_clearBuffOnReset; }

private:
    std::string m_resetCmd;
    bool m_clearBuffOnReset = true;
};

class MicroDebug {
public:
    MicroDebug();
    void dataStreamsWindow();
    void pollDataStreams();
    void viewArea(ImGuiID dock_id);

    void addDataStream();
    void addConsole();
    void addPlot();

    Settings& settings() { return m_settings; }

    std::list<DataStream>& getDataStreams() { return m_dataStreams; }
    std::list<ConsoleWindow>& getConsoles() { return m_consoles; }
    std::list<PlotWindow>& getPlots() { return m_plots; }

private:
    Settings m_settings;
    std::list<DataStream> m_dataStreams;
    std::list<ConsoleWindow> m_consoles;
    std::list<PlotWindow> m_plots;
};

extern MicroDebug microDebug;
