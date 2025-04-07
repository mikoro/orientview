#pragma once

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <string>

struct Session {
    bool showLogWindow = false;
    bool isPlaying = false;
    float timelinePosition = 0.0f;
    float timelineDuration = 100.0f;
    std::string inputVideoPath;
    std::string outputVideoPath;
    char inputVideoPathBuffer[1024] = {0};
    char outputVideoPathBuffer[1024] = {0};

    Session() = default;

    static Session& Instance() {
        static Session instance;
        return instance;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Session, showLogWindow, isPlaying, timelinePosition, timelineDuration, inputVideoPath, outputVideoPath)

inline void LoadSession(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        spdlog::info("Session file not found: '{}'. Using default session", filePath);
        return;
    }

    try {
        std::ifstream f(filePath);
        if (!f.is_open()) {
            spdlog::error("Failed to open session file for reading: '{}'", filePath);
            return;
        }

        nlohmann::json data = nlohmann::json::parse(f);
        f.close();

        Session::Instance() = data.get<Session>();

        if (!Session::Instance().inputVideoPath.empty()) {
            strncpy(Session::Instance().inputVideoPathBuffer, Session::Instance().inputVideoPath.c_str(), sizeof(Session::Instance().inputVideoPathBuffer) - 1);
        }
        
        if (!Session::Instance().outputVideoPath.empty()) {
            strncpy(Session::Instance().outputVideoPathBuffer, Session::Instance().outputVideoPath.c_str(), sizeof(Session::Instance().outputVideoPathBuffer) - 1);
        }

        spdlog::info("Loaded session from '{}'", filePath);
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("Failed to parse session file '{}': {}", filePath, e.what());
    } catch (const std::exception& e) {
        spdlog::error("Error loading session from '{}': {}", filePath, e.what());
    } catch (...) {
        spdlog::error("Unknown error loading session from '{}'", filePath);
    }
}

inline void SaveSession(const std::string& filePath) {
    try {
        Session::Instance().inputVideoPath = Session::Instance().inputVideoPathBuffer;
        Session::Instance().outputVideoPath = Session::Instance().outputVideoPathBuffer;

        nlohmann::json data = Session::Instance();

        std::ofstream f(filePath);
        if (!f.is_open()) {
            spdlog::error("Failed to open session file for writing: '{}'", filePath);
            return;
        }

        f << data.dump(4);
        f.close();

        spdlog::info("Saved session to '{}'", filePath);
    } catch (const std::exception& e) {
        spdlog::error("Error saving session to '{}': {}", filePath, e.what());
    } catch (...) {
        spdlog::error("Unknown error saving session to '{}'", filePath);
    }
}
