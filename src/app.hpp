#pragma once

#include "background_ui.hpp"
#include "log_ui.hpp"
#include "session.hpp"
#include "settings.hpp"
#include "tinyfiledialogs.hpp"
#include "translate.hpp"
#include "video_decoder.hpp"
#include "video_ui.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cmath>
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
#include <vector>

class App {
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    SDL_AudioStream* _outputAudioStream = nullptr;
    SDL_AudioSpec _outputAudioSpec{};
    VideoDecoder _videoDecoder;
    LogUI _logUI;
    BackgroundUI _background;
    VideoUI _videoUI;
    bool _running = false;
    uint64_t _performanceCounterFrequency = 0;
    uint64_t _lastRenderTimeTicks = 0;
    uint64_t _fpsCounterStartTimeTicks = 0;
    int _frameCount = 0;
    float _fpsLogInterval = 10.0f;
    bool _timeLineIsDragged = false;

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

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 1) {
            spdlog::error("SDL_Init Error: {}", SDL_GetError());
            return false;
        }

        bool hdrDisplaySupported = false;

        if (Settings::Instance().forceEnableHdr) {
            hdrDisplaySupported = true;
            spdlog::info("HDR display support forced enabled by settings");
        } else if (Settings::Instance().forceDisableHdr) {
            hdrDisplaySupported = false;
            spdlog::info("HDR display support forced disabled by settings");
        } else {
            int numDisplays;
            SDL_DisplayID* displays = SDL_GetDisplays(&numDisplays);

            if (!displays || numDisplays == 0) {
                spdlog::error("SDL_GetDisplays Error: {}", SDL_GetError());
                return false;
            }

            SDL_PropertiesID displayProps = SDL_GetDisplayProperties(displays[0]);

            if (displayProps) {
                hdrDisplaySupported = SDL_GetBooleanProperty(displayProps, SDL_PROP_DISPLAY_HDR_ENABLED_BOOLEAN, false);
            } else {
                spdlog::warn("Could not query display properties: {}", SDL_GetError());
            }

            SDL_free(displays);
            spdlog::info("HDR display support detected: {}", hdrDisplaySupported ? "Yes" : "No");
        }

        if (hdrDisplaySupported) {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 10);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 10);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 10);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 2);
        } else {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        }

        _window = SDL_CreateWindow(GetWindowTitle().c_str(), Settings::Instance().windowWidth, Settings::Instance().windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

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

        int redBits, greenBits, blueBits, alphaBits;
        SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &redBits);
        SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &greenBits);
        SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blueBits);
        SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alphaBits);
        spdlog::info("Color buffer depth: R{} G{} B{} A{}", redBits, greenBits, blueBits, alphaBits);

        _outputAudioSpec.format = SDL_AUDIO_F32;
        _outputAudioSpec.channels = 2;
        _outputAudioSpec.freq = 48000;

        _outputAudioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_outputAudioSpec, nullptr, nullptr);

        if (!_outputAudioStream) {
            spdlog::warn("Failed to open output audio device stream: {}", SDL_GetError());
        } else {
            SDL_AudioDeviceID outputAudioDevice = SDL_GetAudioStreamDevice(_outputAudioStream);

            if (!outputAudioDevice) {
                spdlog::warn("Failed to get output audio device from stream: {}", SDL_GetError());
                SDL_DestroyAudioStream(_outputAudioStream);
                _outputAudioStream = nullptr;
            } else {
                spdlog::info("Output audio device name: {}", SDL_GetAudioDeviceName(outputAudioDevice));
            }
        }

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
        _videoUI.Init();

        _performanceCounterFrequency = SDL_GetPerformanceFrequency();
        _lastRenderTimeTicks = SDL_GetPerformanceCounter();
        _fpsCounterStartTimeTicks = _lastRenderTimeTicks;
        _frameCount = 0;
        _running = true;

        if (std::filesystem::exists(Session::Instance().runVideoPath)) {
            if (_videoDecoder.Init(Session::Instance().runVideoPath)) {
                _videoDecoder.Seek(Session::Instance().timelinePosition);
            }
        }

        _videoDecoder.SetAudioFrameCallback([this](std::shared_ptr<AudioFrame> audioFrame) {
            if (audioFrame && audioFrame->data && audioFrame->size > 0 && _outputAudioStream) {
                SDL_PutAudioStreamData(_outputAudioStream, audioFrame->data, audioFrame->size);
            }
        });

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

    static std::string GetWindowTitle() {
        if (Session::Instance().name.empty()) {
            return fmt::format("{} {}", TL("app_title"), ORIENTVIEW_VERSION);
        } else {
            return fmt::format("{} {} - {}", TL("app_title"), ORIENTVIEW_VERSION, Session::Instance().name);
        }
    }

    void UpdateWindowTitle() {
        if (_window) {
            SDL_SetWindowTitle(_window, GetWindowTitle().c_str());
        }
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

            if (ImGui::BeginMenu(TL("menu_window"))) {
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

                ImGui::Text("%s:", TL("session_name"));
                ImGui::PushItemWidth(-1);
                if (ImGui::InputText("##session_name", &Session::Instance().name)) {
                    UpdateWindowTitle();
                }
                ImGui::PopItemWidth();

                ImGui::SeparatorText(TL("inputs"));
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
                        Session::Instance().videoDecoder = "";
                        Session::Instance().audioDecoder = "";
                        Session::Instance().timelinePosition = 0.0;

                        _videoDecoder.Close();
                        _videoDecoder.Init(Session::Instance().runVideoPath);
                        SDL_ClearAudioStream(_outputAudioStream);
                    }
                }

                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                ImVec4 hardwareDecoderColor = ImVec4(0.6f, 0.9f, 0.6f, 1.0f);

                if (ImGui::BeginCombo(TL("video_decoder"), Session::Instance().videoDecoder.c_str())) {
                    for (const auto& codec : _videoDecoder.GetVideoDecoders()) {
                        bool isSelected = (Session::Instance().videoDecoder == codec);
                        bool isHardware = _videoDecoder.IsHardwareVideoDecoder(codec);

                        if (isHardware) {
                            ImGui::PushStyleColor(ImGuiCol_Text, hardwareDecoderColor);
                        }

                        if (ImGui::Selectable(codec.c_str(), isSelected)) {
                            if (Session::Instance().videoDecoder != codec) {
                                Session::Instance().videoDecoder = codec;
                                double currentPosition = _videoDecoder.GetCurrentPosition();
                                bool wasPlaying = _videoDecoder.IsPlaying();
                                _videoDecoder.Close();
                                SDL_ClearAudioStream(_outputAudioStream);

                                if (_videoDecoder.Init(Session::Instance().runVideoPath)) {
                                    _videoDecoder.Seek(currentPosition);

                                    if (wasPlaying) {
                                        _videoDecoder.Play();
                                    }
                                }
                            }
                        }

                        if (isHardware) {
                            ImGui::PopStyleColor();
                        }

                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::BeginCombo(TL("audio_decoder"), Session::Instance().audioDecoder.c_str())) {
                    for (const auto& codec : _videoDecoder.GetAudioDecoders()) {
                        bool isSelected = (Session::Instance().audioDecoder == codec);
                        bool isHardware = _videoDecoder.IsHardwareAudioDecoder(codec);

                        if (isHardware) {
                            ImGui::PushStyleColor(ImGuiCol_Text, hardwareDecoderColor);
                        }

                        if (ImGui::Selectable(codec.c_str(), isSelected)) {
                            if (Session::Instance().audioDecoder != codec) {
                                Session::Instance().audioDecoder = codec;
                                double currentPosition = _videoDecoder.GetCurrentPosition();
                                bool wasPlaying = _videoDecoder.IsPlaying();
                                _videoDecoder.Close();
                                SDL_ClearAudioStream(_outputAudioStream);

                                if (_videoDecoder.Init(Session::Instance().runVideoPath)) {
                                    _videoDecoder.Seek(currentPosition);

                                    if (wasPlaying) {
                                        _videoDecoder.Play();
                                    }
                                }
                            }
                        }

                        if (isHardware) {
                            ImGui::PopStyleColor();
                        }

                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();

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

                ImGui::SeparatorText(TL("output"));
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
                _videoUI.RenderVideoWindow(ImGui::GetContentRegionAvail(), _videoDecoder.GetVideoWidth(), _videoDecoder.GetVideoHeight());
            }
            ImGui::End();
        }

        if (Session::Instance().showTimelineWindow) {
            if (ImGui::Begin(fmt::format("{}###window_timeline", TL("window_timeline")).c_str(), &Session::Instance().showTimelineWindow)) {
                RenderTimelineWindow();
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

    static std::string FormatTime(float seconds) {
        int hours = static_cast<int>(seconds) / 3600;
        int minutes = (static_cast<int>(seconds) % 3600) / 60;
        int secs = static_cast<int>(seconds) % 60;
        int milliseconds = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);
        return fmt::format("{:02d}:{:02d}:{:02d}.{:03d}", hours, minutes, secs, milliseconds);
    }

    void RenderTimelineWindow() {
        float timeWidth = ImGui::CalcTextSize("00:00:00.000").x + 10.0f;
        ImGui::PushItemWidth(-timeWidth);
        float prevPos = Session::Instance().timelinePosition;

        if (ImGui::SliderFloat("##timeline", &Session::Instance().timelinePosition, 0.0f, _videoDecoder.GetDuration(), "")) {
            if (prevPos != Session::Instance().timelinePosition) {
                _videoDecoder.Seek(Session::Instance().timelinePosition);
                SDL_ClearAudioStream(_outputAudioStream);
            }
        }

        _timeLineIsDragged = ImGui::IsItemActive();

        ImGui::SameLine();
        ImGui::Text("%s", FormatTime(Session::Instance().timelinePosition).c_str());
        ImGui::PopItemWidth();

        float playTextWidth = ImGui::CalcTextSize(TL("timeline_play")).x;
        float pauseTextWidth = ImGui::CalcTextSize(TL("timeline_pause")).x;
        float buttonWidth = std::max(playTextWidth, pauseTextWidth) + 20.0f;
        float windowWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

        if (ImGui::Button(_videoDecoder.IsPlaying() ? TL("timeline_pause") : TL("timeline_play"), ImVec2(buttonWidth, 0))) {
            if (_videoDecoder.IsPlaying()) {
                _videoDecoder.Pause();
                SDL_PauseAudioStreamDevice(_outputAudioStream);
                SDL_ClearAudioStream(_outputAudioStream);
            } else {
                _videoDecoder.Play();
                SDL_ResumeAudioStreamDevice(_outputAudioStream);
            }
        }
    }

    void Render() {
        uint64_t currentPerformanceCounter = SDL_GetPerformanceCounter();
        float currentTime = (double)currentPerformanceCounter / (double)_performanceCounterFrequency;
        float deltaTime = (double)(currentPerformanceCounter - _lastRenderTimeTicks) / (double)_performanceCounterFrequency;
        _lastRenderTimeTicks = currentPerformanceCounter;

        _frameCount++;
        double elapsedSinceLastFpsLog = (double)(currentPerformanceCounter - _fpsCounterStartTimeTicks) / (double)_performanceCounterFrequency;

        if (elapsedSinceLastFpsLog >= _fpsLogInterval) {
            float averageFps = (double)_frameCount / elapsedSinceLastFpsLog;
            spdlog::info("Average FPS over last {:.1f} seconds: {:.1f}", elapsedSinceLastFpsLog, averageFps);
            _frameCount = 0;
            _fpsCounterStartTimeTicks = currentPerformanceCounter;
        }

        auto videoFrame = _videoDecoder.GetNextVideoFrame();

        if (videoFrame) {
            _videoUI.UpdateTexture(videoFrame);

            if (!_timeLineIsDragged) {
                Session::Instance().timelinePosition = videoFrame->timestamp;
            }
        }

        int windowWidth, windowHeight;
        SDL_GetWindowSize(_window, &windowWidth, &windowHeight);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        RenderDockSpace();
        RenderWindows();
        ImGui::Render();

        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        _background.Render(currentTime);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(_window);
    }

    void Close() {
        if (Settings::Instance().rememberImguiLayout) {
            ImGui::SaveIniSettingsToDisk(GetDataFilePath("imgui.ini").c_str());
        }

        SaveSettings();
        SaveSession(GetDataFilePath("session.json"));

        _videoDecoder.Close();
        _videoUI.Close();

        if (_outputAudioStream) {
            SDL_DestroyAudioStream(_outputAudioStream);
            _outputAudioStream = nullptr;
        }

        _background.Close();

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
            Close();
            return -1;
        }

        while (_running) {
            ProcessEvents();

            if (!_running) {
                break;
            }

            Render();
        }

        Close();
        return 0;
    }
};
