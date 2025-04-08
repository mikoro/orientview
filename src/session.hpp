#pragma once

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <string>

struct Session {
    bool showLogWindow = false;
    bool showSessionWindow = true;
    bool showMapWindow = true;
    bool showVideoWindow = true;
    bool showTimelineWindow = true;
    bool showUIDemoWindow = false;

    float timelinePosition = 0.0f;
    bool isPlaying = false;

    std::string name;
    std::string runVideoPath;
    std::string outputVideoPath;
    std::string mapImagePath;
    std::string gpxFilePath;
    
    std::string videoDecoder;
    std::string audioDecoder;

    Session() = default;

    static Session& Instance() {
        static Session instance;
        return instance;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Session, 
    showLogWindow, showSessionWindow, showMapWindow, showVideoWindow, showTimelineWindow, showUIDemoWindow,
    timelinePosition, isPlaying,
    name, runVideoPath, outputVideoPath, mapImagePath, gpxFilePath,
    videoDecoder, audioDecoder)

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
