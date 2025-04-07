#pragma once

#include "background.hpp"
#include "log_ui.hpp"
#include "session.hpp"
#include "settings.hpp"
#include "tinyfiledialogs.hpp"
#include "translate.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <exception>
#include <fmt/core.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <imgui_internal.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

class App {
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    LogUI _logUI;
    Background _background;
    bool _running = false;
    uint64_t _lastPerformanceCounter = 0;
    uint64_t _performanceCounterFrequency = 0;

    bool Init() {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(GetDataFilePath("orientview.log"), true);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        file_sink->set_level(spdlog::level::trace);
        auto logger = spdlog::default_logger();
        logger->sinks().push_back(file_sink);
        logger->flush_on(spdlog::level::trace);

        _logUI.Init();

        spdlog::set_level(spdlog::level::debug);
        spdlog::info("Starting OrientView");

        LoadSettings();
        LoadTranslations();
        LoadSession(GetDataFilePath("session.json"));

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != true) {
            spdlog::error("SDL_Init Error: {}", SDL_GetError());
            return false;
        }

        _window = SDL_CreateWindow(fmt::format("{} {}", TL("app_title"), ORIENTVIEW_VERSION).c_str(), Settings::Instance().windowWidth, Settings::Instance().windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

        if (!_window) {
            spdlog::error("SDL_CreateWindow Error: {}", SDL_GetError());
            return false;
        }

        _glContext = SDL_GL_CreateContext(_window);

        if (!_glContext) {
            spdlog::error("SDL_GL_CreateContext Error: {}", SDL_GetError());
            return false;
        }

        SDL_GL_MakeCurrent(_window, _glContext);

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            spdlog::error("Failed to initialize GLAD");
            return false;
        }

        spdlog::info("OpenGL Version: {}.{}", GLVersion.major, GLVersion.minor);
        spdlog::info("OpenGL Renderer: {}", (const char*)glGetString(GL_RENDERER));
        spdlog::info("OpenGL Vendor: {}", (const char*)glGetString(GL_VENDOR));
        spdlog::info("OpenGL Shading Language Version: {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        std::string imguiIniPath = GetDataFilePath("imgui.ini");
        io.IniFilename = imguiIniPath.c_str();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.Fonts->AddFontFromFileTTF(GetDataFilePath(Settings::Instance().fontName).c_str(), Settings::Instance().fontSize);

        if (std::filesystem::exists(imguiIniPath)) {
            // ImGui::LoadIniSettingsFromDisk(imguiIniPath.c_str());
        }

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.70f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.80f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.08f, 0.80f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.15f, 0.80f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.15f, 0.70f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.12f, 0.22f, 0.80f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.15f, 0.30f, 0.80f);
        style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.20f, 0.40f, 0.80f);
        style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        ImGui_ImplSDL3_InitForOpenGL(_window, _glContext);
        ImGui_ImplOpenGL3_Init("#version 330");

        _background.Init();

        _performanceCounterFrequency = SDL_GetPerformanceFrequency();
        _lastPerformanceCounter = SDL_GetPerformanceCounter();
        _running = true;

        return true;
    }

    void ProcessEvents() {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                _running = false;
            }

            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(_window)) {
                _running = false;
            }
        }

        SDL_GetWindowSize(_window, &Settings::Instance().windowWidth, &Settings::Instance().windowHeight);
    }

    void SetupDockSpace() {
        static bool firstTime = true;

        if (!firstTime) {
            return;
        }

        firstTime = false;

        ImGuiID dockSpaceId = ImGui::GetID("MainDockSpace");

        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->Size);

        ImGuiID dockIdLeft;
        ImGuiID dockIdRight;
        ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.2f, &dockIdLeft, &dockIdRight);

        ImGuiID dockIdMap;
        ImGuiID dockIdVideo;
        ImGui::DockBuilderSplitNode(dockIdRight, ImGuiDir_Left, 0.4f, &dockIdMap, &dockIdVideo);

        ImGuiID dockIdControls;
        ImGui::DockBuilderSplitNode(dockIdRight, ImGuiDir_Down, 0.1f, &dockIdControls, nullptr);

        ImGui::DockBuilderDockWindow(TL("panel_settings"), dockIdLeft);
        ImGui::DockBuilderDockWindow(TL("panel_map"), dockIdMap);
        ImGui::DockBuilderDockWindow(TL("panel_video"), dockIdVideo);
        ImGui::DockBuilderDockWindow(TL("panel_controls"), dockIdControls);

        ImGui::DockBuilderFinish(dockSpaceId);
    }

    void RenderDockSpace() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, windowFlags);
        ImGui::PopStyleVar();

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu(TL("menu_file"))) {
                if (ImGui::MenuItem(TL("menu_load"))) {
                    const char* filters[] = {"*.json"};
                    char* selectedFile = tinyfd_openFileDialog(TL("menu_load"), "", 1, filters, "JSON Files", 0);

                    if (selectedFile) {
                        LoadSession(selectedFile);
                    }
                }
                if (ImGui::MenuItem(TL("menu_save"))) {
                    const char* filters[] = {"*.json"};
                    char* selectedFile = tinyfd_saveFileDialog(TL("menu_save"), "", 1, filters, "JSON Files");

                    if (selectedFile) {
                        SaveSession(selectedFile);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem(TL("menu_exit"))) {
                    _running = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem(TL("menu_log"))) {
                Session::Instance().showLogWindow = true;
            }
            ImGui::EndMenuBar();
        }

        ImGuiID dockSpaceId = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton);

        SetupDockSpace();

        ImGui::End();
        ImGui::PopStyleColor();
    }

    void RenderPanels() {
        ImGui::Begin(TL("panel_settings"));

        ImGui::Text("%s:", TL("input_video"));
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 100);
        ImGui::InputText("##input_video", Session::Instance().inputVideoPathBuffer, sizeof(Session::Instance().inputVideoPathBuffer));
        ImGui::PopItemWidth();

        ImGui::SameLine();

        ImGui::PushID("browse_input_button");
        if (ImGui::Button(TL("browse_input"))) {
            const char* filters[] = {"*.mp4", "*.avi", "*.mov", "*.mkv"};
            char* selectedFile = tinyfd_openFileDialog(TL("select_video_file"), Session::Instance().inputVideoPathBuffer, 4, filters, TL("video_files"), 0);

            if (selectedFile) {
                strncpy(Session::Instance().inputVideoPathBuffer, selectedFile, sizeof(Session::Instance().inputVideoPathBuffer) - 1);
                Session::Instance().inputVideoPath = selectedFile;
            }
        }
        ImGui::PopID();
        
        ImGui::Text("%s:", TL("output_video"));
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 100);
        ImGui::InputText("##output_video", Session::Instance().outputVideoPathBuffer, sizeof(Session::Instance().outputVideoPathBuffer));
        ImGui::PopItemWidth();

        ImGui::SameLine();

        ImGui::PushID("browse_output_button");
        if (ImGui::Button(TL("browse_output"))) {
            const char* filters[] = {"*.mp4", "*.avi", "*.mov", "*.mkv"};
            char* selectedFile = tinyfd_saveFileDialog(TL("select_output_file"), Session::Instance().outputVideoPathBuffer, 4, filters, TL("video_files"));

            if (selectedFile) {
                strncpy(Session::Instance().outputVideoPathBuffer, selectedFile, sizeof(Session::Instance().outputVideoPathBuffer) - 1);
                Session::Instance().outputVideoPath = selectedFile;
            }
        }
        ImGui::PopID();

        ImGui::End();

        ImGui::Begin(TL("panel_map"));
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        ImVec2 textSize = ImGui::CalcTextSize(TL("panel_map"));
        ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        ImGui::Text("%s", TL("panel_map"));
        ImGui::PopStyleColor();
        ImGui::End();

        ImGui::Begin(TL("panel_video"));
        windowSize = ImGui::GetContentRegionAvail();
        textSize = ImGui::CalcTextSize(TL("panel_video"));
        ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        ImGui::Text("%s", TL("panel_video"));
        ImGui::PopStyleColor();
        ImGui::End();

        ImGui::Begin(TL("panel_controls"));

        if (ImGui::Button(Session::Instance().isPlaying ? TL("controls_pause") : TL("controls_play"))) {
            Session::Instance().isPlaying = !Session::Instance().isPlaying;
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##timeline", &Session::Instance().timelinePosition, 0.0f, Session::Instance().timelineDuration, "%.1f s");
        ImGui::PopItemWidth();

        ImGui::End();

        if (Session::Instance().showLogWindow) {
            _logUI.Render();
        }
    }

    void Render() {
        uint64_t currentFrameTicks = SDL_GetPerformanceCounter();
        float currentTime = (float)currentFrameTicks / _performanceCounterFrequency;
        float deltaTime = (float)(currentFrameTicks - _lastPerformanceCounter) / _performanceCounterFrequency;
        _lastPerformanceCounter = currentFrameTicks;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        RenderDockSpace();
        RenderPanels();
        ImGui::Render();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        _background.Render(currentTime);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(_window);
    }

    void Cleanup() {
        ImGui::SaveIniSettingsToDisk(GetDataFilePath("imgui.ini").c_str());
        SaveSettings();
        SaveSession(GetDataFilePath("session.json"));

        _background.Cleanup();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();

        if (ImGui::GetCurrentContext()) {
            ImGui::DestroyContext();
        }

        if (_window) {
            SDL_DestroyWindow(_window);
            _window = nullptr;
        }

        SDL_Quit();
        spdlog::info("OrientView finished cleanly");
    }

  public:
    int Run() {
        if (!Init()) {
            Cleanup();
            return -1;
        }

        while (_running) {
            ProcessEvents();

            if (!_running) {
                break;
            }

            Render();
        }

        Cleanup();
        return 0;
    }
};
