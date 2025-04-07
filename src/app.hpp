#pragma once

#include "log_ui.hpp"
#include "session.hpp"
#include "settings.hpp"
#include "tinyfiledialogs.hpp"
#include "translate.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_internal.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <exception>

class App {
    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;
    LogUI _logUI;
    bool _running = false;
    uint64_t _lastFrameTicks = 0;
    uint64_t _performanceFrequency = 0;

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

        _window = SDL_CreateWindow(fmt::format("{} {}", TL("app_title"), ORIENTVIEW_VERSION).c_str(), Settings::Instance().windowWidth, Settings::Instance().windowHeight, SDL_WINDOW_RESIZABLE);
        if (!_window) {
            spdlog::error("SDL_CreateWindow Error: {}", SDL_GetError());
            return false;
        }

        _renderer = SDL_CreateRenderer(_window, nullptr);
        if (!_renderer) {
            spdlog::error("SDL_CreateRenderer Error: {}", SDL_GetError());
            return false;
        }

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

        ImGui_ImplSDL3_InitForSDLRenderer(_window, _renderer);
        ImGui_ImplSDLRenderer3_Init(_renderer);

        _performanceFrequency = SDL_GetPerformanceFrequency();
        _lastFrameTicks = SDL_GetPerformanceCounter();
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

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                if (event.window.windowID == SDL_GetWindowID(_window)) {
                    SDL_GetWindowSize(_window, &Settings::Instance().windowWidth, &Settings::Instance().windowHeight);
                    spdlog::debug("Window resized to {}x{}", Settings::Instance().windowWidth, Settings::Instance().windowHeight);
                }
            }
        }
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
        ImGui::SliderFloat("Test Slider", &Settings::Instance().fontSize, 10.0f, 30.0f);
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
        float currentTime = static_cast<float>(currentFrameTicks) / _performanceFrequency;
        float deltaTime = static_cast<float>(currentFrameTicks - _lastFrameTicks) / _performanceFrequency;
        _lastFrameTicks = currentFrameTicks;
        
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        RenderDockSpace();
        RenderPanels();

        ImGui::Render();

        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
        SDL_RenderClear(_renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), _renderer);

        SDL_RenderPresent(_renderer);
    }

    void Cleanup() {
        ImGui::SaveIniSettingsToDisk(GetDataFilePath("imgui.ini").c_str());
        SaveSettings();
        SaveSession(GetDataFilePath("session.json"));

        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();

        if (ImGui::GetCurrentContext()) {
            ImGui::DestroyContext();
        }

        if (_renderer) {
            SDL_DestroyRenderer(_renderer);
            _renderer = nullptr;
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
