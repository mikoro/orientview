#pragma once

#include "settings.hpp"
#include "translate.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include <exception>

class App {
    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;
    bool _running = false;

    bool Init() {
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("Starting OrientView");

        Settings::Instance().Load();
        Translate::Instance().Load();

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != true) {
            spdlog::error("SDL_Init Error: {}", SDL_GetError());
            return false;
        }

        _window = SDL_CreateWindow(fmt::format("{} {}", TR("app_title"), ORIENTVIEW_VERSION).c_str(), Settings::Instance().windowWidth, Settings::Instance().windowHeight, SDL_WINDOW_RESIZABLE);
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

        io.IniFilename = Settings::GetDataFilePath("imgui.ini").c_str();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.Fonts->AddFontFromFileTTF(Settings::GetDataFilePath(Settings::Instance().fontName).c_str(), Settings::Instance().fontSize);

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplSDL3_InitForSDLRenderer(_window, _renderer);
        ImGui_ImplSDLRenderer3_Init(_renderer);

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

    void Render() {
        ImGuiIO& io = ImGui::GetIO();

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
        SDL_RenderClear(_renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), _renderer);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_RenderPresent(_renderer);
    }

    void Cleanup() {
        ImGui::SaveIniSettingsToDisk(Settings::GetDataFilePath("imgui.ini").c_str());
        Settings::Instance().Save();

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
