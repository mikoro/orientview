#pragma once

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <string>

class Settings {
  private:
    Settings() = default;

  public:
    std::string language = "en";
    std::string fontName = "noto-sans-regular.ttf";
    float fontSize = 22.0f;
    int windowWidth = 1920;
    int windowHeight = 1080;

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    static Settings& Instance() {
        static Settings instance;
        return instance;
    }

    static std::string GetDataFilePath(const std::string& fileName) {
        std::string dataDir = ORIENTVIEW_DATA_DIR_PATH;

        if (!dataDir.empty() && std::filesystem::exists(dataDir)) {
            return fmt::format("{}/{}", dataDir, fileName);
        }

        return fmt::format("data/{}", fileName);
    }

    void Load() {
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

            language = data.value("language", language);
            fontName = data.value("fontName", fontName);
            fontSize = data.value("fontSize", fontSize);
            windowWidth = data.value("windowWidth", windowWidth);
            windowHeight = data.value("windowHeight", windowHeight);

            spdlog::info("Loaded settings from '{}'", settingsFilePath);
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("Failed to parse settings file '{}': {}", settingsFilePath, e.what());
        } catch (const std::exception& e) {
            spdlog::error("Error loading settings from '{}': {}", settingsFilePath, e.what());
        } catch (...) {
            spdlog::error("Unknown error loading settings from '{}'", settingsFilePath);
        }
    }

    void Save() {
        std::string settingsFilePath = GetDataFilePath("settings.json");

        try {
            nlohmann::json data;

            data["language"] = language;
            data["fontName"] = fontName;
            data["fontSize"] = fontSize;
            data["windowWidth"] = windowWidth;
            data["windowHeight"] = windowHeight;

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
};
