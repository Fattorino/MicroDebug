#pragma once

#include <imgui.h>
#include <imgui_stdlib.h>
#include <implot.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <fstream>

#include <serial/serial.h>
#include <nfd.h>
#include "includes/rapidcsv.h"
#include <DeviceINQ.h>
#include <BluetoothException.h>
#include <BTSerialPortBinding.h>

#include "MicroDebug.h"

enum DataStreamType {
    DataStreamType_SERIAL,
    DataStreamType_FILE,
    DataStreamType_BLUETOOTH,
    DataStreamType_WEB_SOCKET
};

enum FileType {
    FileType_TimeSeries,
    FileType_CSV
};

template <typename T>
inline T RandomRange(T min, T max) {
    T scale = rand() / (T) RAND_MAX;
    return min + scale * ( max - min );
}

inline ImVec4 RandomColor() {
    ImVec4 col;
    col.x = RandomRange(0.0f,1.0f);
    col.y = RandomRange(0.0f,1.0f);
    col.z = RandomRange(0.0f,1.0f);
    col.w = 1.0f;
    return col;
}

struct PlotItem {
    PlotItem(const std::string& name) : name(name) {}
    std::string name;
    ImAxis axis = -1;
    ImVec4 color = RandomColor();
    std::vector<float> data;
    std::vector<float> support;
};

class DataStream {
public:
    explicit DataStream(std::string name);

    void draw();
    void clear();
    void sendResetCmd();
    void poll();

    [[nodiscard]] const std::string& getName() const { return m_name; }
    [[nodiscard]] const ImGuiTextBuffer& getBuffer() const { return m_buf; }
    [[nodiscard]] const ImVector<int>& getLineOffsets() const { return m_lineOffsets; }
    [[nodiscard]] std::vector<PlotItem>& getPlotItems() { return m_plotItems; }

private:
    void drawSerial();
    void drawFile();
    void drawBluetooth();
    void drawWebSocket();

    void pollSerial();
    void parseFile();
    void pollBluetooth();
    void pollWebSocket();

    void saveMsg(const std::string& msg);
    void saveLine(const std::string& line);
    int plotItemIdx(const std::string& name);
    void parseMsg(const std::string& msg, bool multiLine = false);

    std::string m_name = "New Data Stream";
    bool m_connected = false;

    const char* m_streamTypeOptions = "SERIAL\0FILE\0BLUETOOTH\0WEB SOCKET\0\0";
    DataStreamType m_type = DataStreamType_SERIAL;

    ImGuiTextBuffer m_buf;
    ImVector<int> m_lineOffsets;
    std::vector<PlotItem> m_plotItems;

    std::unique_ptr<serial::Serial> m_serial;
    std::vector<serial::PortInfo> m_devices;
    serial::PortInfo m_port;
    std::vector<int> m_baudRates = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    int m_baudRate = 115200;
    bool m_advancedSerialMode = false;
    // std::string m_parity;
    // std::string m_stopBits;
    // std::string m_dataBits;
    // std::string m_flowControl;

    const char* m_fileTypeOptions = "Time Series\0CSV\0\0";
    FileType m_fileType = FileType_TimeSeries;
    std::string m_filePath;

    std::unique_ptr<DeviceINQ> m_BL_inquiry;
    std::vector<device> m_BL_devices;
    int m_BL_deviceIdx = 0;
    BTSerialPortBinding* m_BL_serial = nullptr;
};
