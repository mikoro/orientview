#pragma once

#include "custom_font_data.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <spdlog/spdlog.h>

#include <exception>

class App {
  public:
    int Run() {
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("Starting OrientView");

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
            spdlog::error("SDL_Init Error: {}", SDL_GetError());
            return -1;
        }

        SDL_Window* window = SDL_CreateWindow("OrientView", 1280, 720, SDL_WINDOW_RESIZABLE);
        if (!window) {
            spdlog::error("SDL_CreateWindow Error: {}", SDL_GetError());
            SDL_Quit();
            return -1;
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer) {
            spdlog::error("SDL_CreateRenderer Error: {}", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImFontConfig font_cfg;
        font_cfg.FontDataOwnedByAtlas = false;

        io.Fonts->AddFontFromMemoryTTF((void*)custom_font_data, sizeof(custom_font_data), 22.0f, &font_cfg);

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);

        bool running = true;
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
                if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) {
                    running = false;
                }
            }

            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();

            ImGui::Render();

            SDL_SetRenderDrawColor(renderer, 114, 144, 154, 255);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                SDL_Window*   backup_current_window  = SDL_GL_GetCurrentWindow();
                SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
            }

            SDL_RenderPresent(renderer);
        }

        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        spdlog::info("OrientView finished cleanly");
        return 0;
    }
};
