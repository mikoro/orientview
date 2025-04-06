#pragma once

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <string>

class Settings {
  private:
    const std::string _settingsFilePath = "settings.json";

    Settings() = default;

  public:
    int   windowWidth  = 1920;
    int   windowHeight = 1080;
    float fontSize     = 22.0f;

    Settings(const Settings&)            = delete;
    Settings& operator=(const Settings&) = delete;

    static Settings& Instance() {
        static Settings instance;
        return instance;
    }

    void Load() {
        if (!std::filesystem::exists(_settingsFilePath)) {
            spdlog::info("Settings file not found: '{}'. Using default settings", _settingsFilePath);
            return;
        }

        try {
            std::ifstream f(_settingsFilePath);
            if (!f.is_open()) {
                spdlog::error("Failed to open settings file for reading: '{}'", _settingsFilePath);
                return;
            }

            nlohmann::json data = nlohmann::json::parse(f);
            f.close();

            windowWidth  = data.value("windowWidth", windowWidth);
            windowHeight = data.value("windowHeight", windowHeight);
            fontSize     = data.value("fontSize", fontSize);

            spdlog::info("Loaded settings from '{}'", _settingsFilePath);
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("Failed to parse settings file '{}': {}", _settingsFilePath, e.what());
        } catch (const std::exception& e) {
            spdlog::error("Error loading settings from '{}': {}", _settingsFilePath, e.what());
        } catch (...) {
            spdlog::error("Unknown error loading settings from '{}'", _settingsFilePath);
        }
    }

    void Save() {
        try {
            nlohmann::json data;
            data["windowWidth"]  = windowWidth;
            data["windowHeight"] = windowHeight;
            data["fontSize"]     = fontSize;

            std::ofstream f(_settingsFilePath);

            if (!f.is_open()) {
                spdlog::error("Failed to open settings file for writing: '{}'", _settingsFilePath);
                return;
            }

            f << data.dump(4);
            f.close();

            spdlog::info("Saved settings to '{}'", _settingsFilePath);
        } catch (const std::exception& e) {
            spdlog::error("Error saving settings to '{}': {}", _settingsFilePath, e.what());
        } catch (...) {
            spdlog::error("Unknown error saving settings to '{}'", _settingsFilePath);
        }
    }
};
