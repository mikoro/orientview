#pragma once

#include "background.hpp"
#include "log_ui.hpp"
#include "session.hpp"
#include "settings.hpp"
#include "tinyfiledialogs.hpp"
#include "translate.hpp"
#include "video_decoder_ui.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <exception>
#include <fmt/core.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

class App {
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    LogUI _logUI;
    Background _background;
    VideoDecoderUI _videoDecoderUI;
    bool _running = false;
    uint64_t _lastPerformanceCounter = 0;
    uint64_t _performanceCounterFrequency = 0;

    uint64_t _fpsCounterStart = 0;
    int _frameCount = 0;
    float _fpsLogInterval = 10.0f;

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

        VideoDecoder::LogAvailableCodecs();

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 1) {
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

        if (Settings::Instance().rememberImguiLayout) {
            io.IniFilename = imguiIniPath.c_str();
        } else {
            io.IniFilename = nullptr;
        }

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        std::string fontPath = GetDataFilePath(Settings::Instance().fontName);

        if (std::filesystem::exists(fontPath)) {
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), Settings::Instance().fontSize);
        } else {
            spdlog::warn("Font file not found: '{}'. Using default font", fontPath);
        }

        if (Settings::Instance().rememberImguiLayout && std::filesystem::exists(imguiIniPath)) {
            spdlog::info("Loading ImGui layout from '{}'", imguiIniPath);
            ImGui::LoadIniSettingsFromDisk(imguiIniPath.c_str());
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
        _videoDecoderUI.Init();

        // Set up position changed callback
        _videoDecoderUI.SetPositionChangedCallback([this](double position) { Session::Instance().timelinePosition = position; });

        _performanceCounterFrequency = SDL_GetPerformanceFrequency();
        _lastPerformanceCounter = SDL_GetPerformanceCounter();
        _fpsCounterStart = _lastPerformanceCounter;
        _frameCount = 0;
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

            if (event.type == SDL_EVENT_KEY_DOWN) {
                const bool* state = SDL_GetKeyboardState(nullptr);

                if (event.key.key == SDLK_ESCAPE) {
                    _running = false;
                }

                if (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]) {
                    if (event.key.key == SDLK_PLUS) {
                        Settings::Instance().fontSize += 1.0f;
                        ReloadFont();
                    } else if (event.key.key == SDLK_MINUS) {
                        Settings::Instance().fontSize = std::max(8.0f, Settings::Instance().fontSize - 1.0f);
                        ReloadFont();
                    }
                }
            }
        }

        SDL_GetWindowSize(_window, &Settings::Instance().windowWidth, &Settings::Instance().windowHeight);
    }

    void ReloadFont() {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        std::string fontPath = GetDataFilePath(Settings::Instance().fontName);
        if (std::filesystem::exists(fontPath)) {
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), Settings::Instance().fontSize);
            spdlog::info("Font size changed to: {}", Settings::Instance().fontSize);
        } else {
            spdlog::warn("Font file not found: '{}'. Using default font", fontPath);
        }

        // Rebuild font atlas
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui_ImplOpenGL3_CreateFontsTexture();
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

        ImGui::DockBuilderDockWindow("###window_session", dockIdLeft);
        ImGui::DockBuilderDockWindow("###window_map", dockIdMap);
        ImGui::DockBuilderDockWindow("###window_video", dockIdVideo);
        ImGui::DockBuilderDockWindow("###window_timeline", dockIdControls);

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
            if (ImGui::BeginMenu(TL("menu_session"))) {
                if (ImGui::MenuItem(TL("menu_load"))) {
                    const char* filters[] = {"*.json"};
                    char* selectedFile = tinyfd_openFileDialog(TL("load_session"), "", 1, filters, TL("json_files"), 0);

                    if (selectedFile) {
                        LoadSession(selectedFile);
                    }
                }
                if (ImGui::MenuItem(TL("menu_save"))) {
                    const char* filters[] = {"*.json"};
                    char* selectedFile = tinyfd_saveFileDialog(TL("save_session"), "", 1, filters, TL("json_files"));

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

            if (ImGui::BeginMenu(TL("menu_windows"))) {
                ImGui::MenuItem(TL("window_session"), nullptr, &Session::Instance().showSessionWindow);
                ImGui::MenuItem(TL("window_map"), nullptr, &Session::Instance().showMapWindow);
                ImGui::MenuItem(TL("window_video"), nullptr, &Session::Instance().showVideoWindow);
                ImGui::MenuItem(TL("window_timeline"), nullptr, &Session::Instance().showTimelineWindow);
                ImGui::MenuItem(TL("window_log"), nullptr, &Session::Instance().showLogWindow);
                ImGui::Separator();
                ImGui::MenuItem(TL("window_ui_demo"), nullptr, &Session::Instance().showUIDemoWindow);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(TL("menu_language"))) {
                for (const auto& [langCode, translations] : Translations::Instance().translations) {
                    std::string langNameKey = "language_" + langCode;
                    bool isSelected = (Settings::Instance().language == langCode);

                    if (ImGui::MenuItem(TL(langNameKey), nullptr, isSelected)) {
                        if (!isSelected) {
                            Settings::Instance().language = langCode;
                            spdlog::info("Language changed to: {}", langCode);
                        }
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGuiID dockSpaceId = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton);

        SetupDockSpace();

        ImGui::End();
        ImGui::PopStyleColor();
    }

    void RenderWindows() {
        if (Session::Instance().showSessionWindow) {
            if (ImGui::Begin(fmt::format("{}###window_session", TL("window_session")).c_str(), &Session::Instance().showSessionWindow)) {
                float browseButtonWidth = ImGui::CalcTextSize(TL("browse")).x + ImGui::GetStyle().FramePadding.x * 2.0f + 10.0f;

                if (ImGui::CollapsingHeader(TL("input"), ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("%s:", TL("run_video"));
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - browseButtonWidth - ImGui::GetStyle().ItemSpacing.x);
                    ImGui::InputText("##run_video", &Session::Instance().runVideoPath);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();

                    if (ImGui::Button(fmt::format("{}##browse_input", TL("browse")).c_str(), ImVec2(browseButtonWidth, 0))) {
                        const char* filters[] = {"*.*"};
                        char* selectedFile = tinyfd_openFileDialog(TL("select_run_video"), Session::Instance().runVideoPath.c_str(), 1, filters, TL("video_files"), 0);

                        if (selectedFile) {
                            Session::Instance().runVideoPath = selectedFile;
                        }
                    }

                    ImGui::Text("%s:", TL("map_image"));
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - browseButtonWidth - ImGui::GetStyle().ItemSpacing.x);
                    ImGui::InputText("##map_image", &Session::Instance().mapImagePath);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();

                    if (ImGui::Button(fmt::format("{}##browse_map", TL("browse")).c_str(), ImVec2(browseButtonWidth, 0))) {
                        const char* filters[] = {"*.*"};
                        char* selectedFile = tinyfd_openFileDialog(TL("select_map_file"), Session::Instance().mapImagePath.c_str(), 1, filters, TL("image_files"), 0);

                        if (selectedFile) {
                            Session::Instance().mapImagePath = selectedFile;
                        }
                    }

                    ImGui::Text("%s:", TL("gpx_file"));
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - browseButtonWidth - ImGui::GetStyle().ItemSpacing.x);
                    ImGui::InputText("##gpx_file", &Session::Instance().gpxFilePath);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();

                    if (ImGui::Button(fmt::format("{}##browse_gpx", TL("browse")).c_str(), ImVec2(browseButtonWidth, 0))) {
                        const char* filters[] = {"*.*"};
                        char* selectedFile = tinyfd_openFileDialog(TL("select_gpx_file"), Session::Instance().gpxFilePath.c_str(), 1, filters, TL("gpx_files"), 0);

                        if (selectedFile) {
                            Session::Instance().gpxFilePath = selectedFile;
                        }
                    }
                }

                if (ImGui::CollapsingHeader(TL("output"), ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("%s:", TL("output_video"));
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - browseButtonWidth - ImGui::GetStyle().ItemSpacing.x);
                    ImGui::InputText("##output_video", &Session::Instance().outputVideoPath);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();

                    if (ImGui::Button(fmt::format("{}##browse_output", TL("browse")).c_str(), ImVec2(browseButtonWidth, 0))) {
                        const char* filters[] = {"*.*"};
                        char* selectedFile = tinyfd_saveFileDialog(TL("select_output_file"), Session::Instance().outputVideoPath.c_str(), 1, filters, TL("video_files"));

                        if (selectedFile) {
                            Session::Instance().outputVideoPath = selectedFile;
                        }
                    }
                }
            }
            ImGui::End();
        }

        if (Session::Instance().showMapWindow) {
            if (ImGui::Begin(fmt::format("{}###window_map", TL("window_map")).c_str(), &Session::Instance().showMapWindow)) {
                ImVec2 windowSize = ImGui::GetContentRegionAvail();
                ImVec2 textSize = ImGui::CalcTextSize(TL("window_map"));
                ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
                ImGui::Text("%s", TL("window_map"));
                ImGui::PopStyleColor();
            }
            ImGui::End();
        }

        if (Session::Instance().showVideoWindow) {
            if (ImGui::Begin(fmt::format("{}###window_video", TL("window_video")).c_str(), &Session::Instance().showVideoWindow)) {
                ImVec2 windowSize = ImGui::GetContentRegionAvail();
                _videoDecoderUI.RenderVideoWindow(windowSize);
            }
            ImGui::End();
        }

        if (Session::Instance().showTimelineWindow) {
            if (ImGui::Begin(fmt::format("{}###window_timeline", TL("window_timeline")).c_str(), &Session::Instance().showTimelineWindow)) {
                _videoDecoderUI.RenderTimelineWindow(Session::Instance().timelinePosition, Session::Instance().timelineDuration, Session::Instance().isPlaying, Session::Instance().runVideoPath);

                // Update timeline duration when video is loaded
                if (_videoDecoderUI.IsRunning() && Session::Instance().timelineDuration <= 0.1f) {
                    Session::Instance().timelineDuration = _videoDecoderUI.GetDuration();
                }
            }
            ImGui::End();
        }

        if (Session::Instance().showLogWindow) {
            _logUI.Render();
        }

        if (Session::Instance().showUIDemoWindow) {
            ImGui::ShowDemoWindow(&Session::Instance().showUIDemoWindow);
        }
    }

    void Render() {
        uint64_t currentPerformanceCounter = SDL_GetPerformanceCounter();
        float currentTime = (double)currentPerformanceCounter / (double)_performanceCounterFrequency;
        float deltaTime = (double)(currentPerformanceCounter - _lastPerformanceCounter) / (double)_performanceCounterFrequency;
        _lastPerformanceCounter = currentPerformanceCounter;

        _frameCount++;
        double elapsedSinceLastFpsLog = (double)(currentPerformanceCounter - _fpsCounterStart) / (double)_performanceCounterFrequency;

        if (elapsedSinceLastFpsLog >= _fpsLogInterval) {
            float averageFps = (double)_frameCount / elapsedSinceLastFpsLog;
            spdlog::info("Average FPS over last {:.1f} seconds: {:.1f}", elapsedSinceLastFpsLog, averageFps);
            _frameCount = 0;
            _fpsCounterStart = currentPerformanceCounter;
        }

        _videoDecoderUI.Update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        RenderDockSpace();
        RenderWindows();
        ImGui::Render();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        _background.Render(currentTime);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(_window);
    }

    void Cleanup() {
        if (Settings::Instance().rememberImguiLayout) {
            ImGui::SaveIniSettingsToDisk(GetDataFilePath("imgui.ini").c_str());
        }

        SaveSettings();
        SaveSession(GetDataFilePath("session.json"));

        _videoDecoderUI.Cleanup();

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
