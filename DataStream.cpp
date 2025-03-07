#include "DataStream.h"

DataStream::DataStream(std::string name) : m_name(std::move(name)) {
    clear();
    m_BL_inquiry = std::unique_ptr<DeviceINQ>(DeviceINQ::Create());
}

void DataStream::drawSerial() {
    if (ImGui::BeginCombo("Port", m_port.description.c_str())) {
        m_devices = serial::list_ports();
        for (const auto &d: m_devices) {
            if (ImGui::Selectable(d.description.c_str(), m_port.port == d.port)) {
                m_port = d;
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::BeginCombo("Baud Rate", std::to_string(m_baudRate).c_str())) {
        for (const auto &br: m_baudRates) {
            if (ImGui::Selectable(std::to_string(br).c_str(), m_baudRate == br)) {
                m_baudRate = br;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Checkbox("Advanced Settings", &m_advancedSerialMode);
    if (m_advancedSerialMode) {
        ImGui::Text("Advanced Settings not implemented yet");
    }
    if (ImGui::Button("Connect") && !m_port.port.empty()) {
        if (m_serial) {
            m_serial->close();
            m_serial.reset();
        }
        m_serial = std::make_unique<serial::Serial>(m_port.port, m_baudRate);
        clear();
        m_connected = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Disconnect")) {
        m_connected = false;
        if (m_serial) {
            m_serial->close();
            m_serial.reset();
        }
    }
}

void DataStream::drawFile() {
    ImGui::Combo("File Type", reinterpret_cast<int *>(&m_fileType), m_fileTypeOptions);
    if (ImGui::InputText("##File Path", &m_filePath, ImGuiInputTextFlags_EnterReturnsTrue)) {
        parseFile();
    }
    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
        if (NFD_Init() == NFD_OKAY) {
            nfdu8char_t *outPath;
            nfdu8filteritem_t filters[2] = {{"Log file", "csv,txt"}};
            nfdopendialogu8args_t args = {0};
            args.filterList = filters;
            args.filterCount = 1;
            nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
            if (result == NFD_OKAY) {
                m_filePath = outPath;
                NFD_FreePathU8(outPath);
                parseFile();
            }
            NFD_Quit();
        }
    }
}

void DataStream::drawBluetooth() {
    if (ImGui::Button("Fetch devices")) {
        m_BL_devices = m_BL_inquiry->Inquire();
        for (const auto& d : m_BL_devices) {
            std::cout << d.name << " " << d.address << std::endl;
        }
        std::cout << std::endl << "done, found " << m_BL_devices.size() << " device(s)" << std::endl;
    }
    ImGui::SameLine(); ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Query for available devices\n(This may take a bit)");
    }
    std::string label = m_BL_devices.empty() ? "None" : m_BL_devices[m_BL_deviceIdx].name.empty() ? "<Hidden>" : m_BL_devices[m_BL_deviceIdx].name;
    if (ImGui::BeginCombo("Device", label.c_str())) {
        for (int i = 0; i < m_BL_devices.size(); i++) {
            label = m_BL_devices[i].name.empty() ? "<Hidden>##" + std::to_string(i) : m_BL_devices[i].name;
            if (ImGui::Selectable(label.c_str(), m_BL_deviceIdx == i)) {
                m_BL_deviceIdx = i;
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button("Connect") && !m_BL_devices.empty()) {
        try {
            m_BL_serial = BTSerialPortBinding::Create(m_BL_devices[m_BL_deviceIdx].address, 1);
            m_BL_serial->Connect();
            m_connected = true;
        } catch (BluetoothException& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Disconnect")) {
        m_connected = false;
        if (m_BL_serial) {
            m_BL_serial->Close();
            m_BL_serial = nullptr;
        }
    }
}

void DataStream::drawWebSocket() {
    ImGui::Text("Web Socket not implemented yet");
}

void DataStream::draw() {
    ImGui::PushID(this);
    std::string title = m_name + (m_connected ? " (Connected)" : " (Disconnected)");
    if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        // Right-click menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Clear buffer")) { clear(); }
            if (ImGui::MenuItem("Send reset cmd")) { sendResetCmd(); }
            ImGui::EndPopup();
        }

        ImGui::BeginDisabled(m_connected);
        ImGui::Combo("Stream Type", reinterpret_cast<int *>(&m_type), m_streamTypeOptions);
        ImGui::EndDisabled();
        ImGui::Separator();
        switch (m_type) {
            case DataStreamType_SERIAL:
                drawSerial();
                break;
            case DataStreamType_FILE:
                drawFile();
                break;
            case DataStreamType_BLUETOOTH:
                drawBluetooth();
                break;
            case DataStreamType_WEB_SOCKET:
                drawWebSocket();
                break;
        }
    }
    ImGui::PopID();
}

void DataStream::clear() {
    m_buf.clear();
    m_lineOffsets.clear();
    m_lineOffsets.push_back(0);
    m_plotItems.clear();
}

void DataStream::sendResetCmd() {
    switch (m_type) {
        case DataStreamType_SERIAL:
            if (m_serial && m_serial->isOpen()) {
                m_serial->write(microDebug.settings().getResetCmd());
                if (microDebug.settings().clearBuffOnReset()) {
                    clear();
                }
            }
            break;
        default:
            break;
    }
}

void DataStream::saveMsg(const std::string& msg) {
    int old_size = m_buf.size();
    m_buf.append(msg.c_str());
    for (int new_size = m_buf.size(); old_size < new_size; old_size++) {
        if (m_buf[old_size] == '\n') {
            m_lineOffsets.push_back(old_size + 1);
        }
    }
}

void DataStream::saveLine(const std::string& line) {
    int old_size = m_buf.size();
    m_buf.append(line.c_str());
    m_buf.append("\n");
    for (int new_size = m_buf.size(); old_size < new_size; old_size++) {
        if (m_buf[old_size] == '\n') {
            m_lineOffsets.push_back(old_size + 1);
        }
    }
}

void DataStream::pollSerial() {
    try {
        if (m_serial && m_serial->isOpen() && m_serial->available() > 0) {
            std::string msg;
            m_serial->readline(msg);
            saveMsg(msg);
            parseMsg(msg);
        }
    } catch (serial::IOException& e) {
        std::cerr << e.what() << std::endl;
    }
}

void DataStream::parseFile() {
    clear();
    std::ifstream file(m_filePath);
    if (!file.is_open()) { m_filePath.clear(); return; }
    switch (m_fileType) {
        case FileType_TimeSeries:
            for (std::string line; std::getline(file, line);) {
                saveLine(line);
                size_t firstComma = line.find_first_of(',');
                if (firstComma == std::string::npos) { continue; } // skip lines without a comma
                size_t secondComma = line.find_first_of(',', firstComma + 1);
                if (secondComma == std::string::npos) { continue; } // skip lines with only 1 comma
                if (line.find_first_of(',', secondComma + 1) != std::string::npos) { continue; } // skip lines with more than 2 commas
                uint32_t time;
                try { time = std::stoul(line.substr(0, firstComma)); } catch (std::invalid_argument& e) { continue; } // skip lines with non-numeric time
                std::string name = line.substr(firstComma + 1, secondComma - firstComma - 1);
                float value;
                try { value = std::stof(line.substr(secondComma + 1)); } catch (std::invalid_argument& e) { continue; } // skip lines with non-numeric value
                int idx = plotItemIdx(name);
                m_plotItems[idx].data.emplace_back(value);
                m_plotItems[idx].support.emplace_back(time);
            }
            break;
        case FileType_CSV:
            for (std::string line; std::getline(file, line);) {
                saveLine(line);
            }
            rapidcsv::Document csv(m_filePath, rapidcsv::LabelParams(), rapidcsv::SeparatorParams(), rapidcsv::ConverterParams(true));
            for (int i = 0; i < csv.GetColumnCount(); i++) {
                std::string name = csv.GetColumnName(i);
                if (name.empty()) { continue; } // skip empty columns (e.g. trailing commas in CSV file)
                int idx = plotItemIdx(name);
                m_plotItems[idx].data = csv.GetColumn<float>(i);
            }
            break;
    }
}

void DataStream::pollBluetooth() {
    if (m_BL_serial && m_BL_serial->IsDataAvailable()) {
        char buff[512];
        int dataLen;
        try {
            dataLen = m_BL_serial->Read(buff, 512);
        } catch (BluetoothException& e) {
            std::cerr << e.what() << std::endl;
            return;
        }
        std::string msg(buff, dataLen);
        saveMsg(msg);
        parseMsg(msg, true);
    }
}

void DataStream::pollWebSocket() {
    // TODO
}

void DataStream::poll() {
    if (!m_connected) { return; }
    switch (m_type) {
        case DataStreamType_SERIAL:
            pollSerial();
            break;
        case DataStreamType_BLUETOOTH:
            pollBluetooth();
            break;
        case DataStreamType_WEB_SOCKET:
            pollWebSocket();
            break;
    }
}

int DataStream::plotItemIdx(const std::string& name) {
    for (int i = 0; i < m_plotItems.size(); i++) {
        if (m_plotItems[i].name == name) {
            return i;
        }
    }
    m_plotItems.emplace_back(name);
    return m_plotItems.size() - 1;
}

std::vector<std::string> split(const std::string& s, char seperator) {
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = s.find(seperator, pos)) != std::string::npos) {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    return output;
}

bool isSpacesNum(const std::string& s, int num) {
    for (char c : s) {
        if (c == ' ') {
            num--;
        }
    }
    return num == 0;
}

void DataStream::parseMsg(const std::string& msg, bool multiLine) {
    if (multiLine) {
        std::vector<std::string> lines = split(msg, '\n');
        for (const auto& line : lines) {
            parseMsg(line);
        }
        return;
    }
    
    if (msg.length() < 4 || !isSpacesNum(msg, 1)) { return; }
    int numStart = msg.find(' ') + 1;
    if (msg.begin() + numStart == msg.end() || !std::isalnum(msg[0])) { return; }
    float v;
    try { v = std::stof(msg.substr(numStart)); } catch (std::invalid_argument& e) { return; }
    int idx = plotItemIdx(msg.substr(0, numStart - 1));
    m_plotItems[idx].data.emplace_back(v);
}
