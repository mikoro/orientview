#pragma once

#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <string>

struct Settings {
    std::string language = "en";
    std::string fontName = "noto-sans-regular.ttf";
    float fontSize = 24.0f;
    int windowWidth = 1920;
    int windowHeight = 1080;

    Settings() = default;

    static Settings& Instance() {
        static Settings instance;
        return instance;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, language, fontName, fontSize, windowWidth, windowHeight)

inline std::string GetDataFilePath(const std::string& fileName) {
    std::string dataDir = ORIENTVIEW_DATA_DIR_PATH;

    if (!dataDir.empty() && std::filesystem::exists(dataDir)) {
        std::filesystem::path fullPath = std::filesystem::absolute(std::filesystem::path(dataDir) / fileName);
        return fullPath.string();
    }

    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path dataPath = std::filesystem::absolute(currentPath / "data" / fileName);
    return dataPath.string();
}

inline void LoadSettings() {
    std::string settingsFilePath = GetDataFilePath("settings.json");

    if (!std::filesystem::exists(settingsFilePath)) {
        spdlog::info("Settings file not found: '{}'. Using default settings", settingsFilePath);
        return;
    }

    try {
        std::ifstream f(settingsFilePath);
        if (!f.is_open()) {
            spdlog::error("Failed to open settings file for reading: '{}'", settingsFilePath);
            return;
        }

        nlohmann::json data = nlohmann::json::parse(f);
        f.close();

        Settings::Instance() = data.get<Settings>();

        spdlog::info("Loaded settings from '{}'", settingsFilePath);
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("Failed to parse settings file '{}': {}", settingsFilePath, e.what());
    } catch (const std::exception& e) {
        spdlog::error("Error loading settings from '{}': {}", settingsFilePath, e.what());
    } catch (...) {
        spdlog::error("Unknown error loading settings from '{}'", settingsFilePath);
    }
}

inline void SaveSettings() {
    std::string settingsFilePath = GetDataFilePath("settings.json");

    try {
        nlohmann::json data = Settings::Instance();

        std::ofstream f(settingsFilePath);
        if (!f.is_open()) {
            spdlog::error("Failed to open settings file for writing: '{}'", settingsFilePath);
            return;
        }

        f << data.dump(4);
        f.close();

        spdlog::info("Saved settings to '{}'", settingsFilePath);
    } catch (const std::exception& e) {
        spdlog::error("Error saving settings to '{}': {}", settingsFilePath, e.what());
    } catch (...) {
        spdlog::error("Unknown error saving settings to '{}'", settingsFilePath);
    }
}
