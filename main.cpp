
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <iostream>
#include <implot.h>
#include "MicroDebug.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Micro Debug", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    io.IniFilename = nullptr; // Disable .ini file

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool docking_initialized = false;
    ImGuiID dock_space = 0;
    ImGuiID side_bar_dock = 0;

    std::thread pollThread([]() {
        while (true) {
            microDebug.pollDataStreams();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

//        while (true) {
//            auto start = std::chrono::high_resolution_clock::now();
//            microDebug.pollDataStreams();
//            do {
//                std::this_thread::yield();
//            } while (std::chrono::high_resolution_clock::now() - start < std::chrono::milliseconds(1));
//        }

//        while (true) { microDebug.pollDataStreams(); }
    });

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save As..")) {}
            if (ImGui::MenuItem("Exit")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Streams")) {
            if (ImGui::MenuItem("Add", "Ctrl+N")) { microDebug.addDataStream(); }
            microDebug.settings().dataStreamSettingsMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Views")) {
            if (ImGui::MenuItem("Add console", "Ctrl+T")) { microDebug.addConsole(); }
            if (ImGui::MenuItem("Add plot", "Ctrl+P")) { microDebug.addPlot(); }
            microDebug.settings().viewsSettingsMenu();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        // Handle docking
        dock_space = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        if (!docking_initialized) {
            ImGui::DockBuilderSetNodeSize(dock_space, ImGui::GetMainViewport()->Size);
            side_bar_dock = ImGui::DockBuilderSplitNode(dock_space, ImGuiDir_Left, 0.25f, nullptr, &dock_space);
            docking_initialized = true;
        }

        // Handle keyboad shortcuts
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            if (ImGui::IsKeyPressed(ImGuiKey_N, false)) {
                microDebug.addDataStream();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_T, false)) {
                microDebug.addConsole();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_P, false)) {
                microDebug.addPlot();
            }
        }

        ImGui::SetNextWindowDockID(side_bar_dock, ImGuiCond_Once);
        microDebug.dataStreamsWindow();

        microDebug.viewArea(dock_space);

//        ImPlot::ShowDemoWindow();
//        ImGui::ShowDemoWindow();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


