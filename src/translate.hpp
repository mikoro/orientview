#pragma once

#include "settings.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

struct Translations {
    std::map<std::string, std::map<std::string, std::string>> translations; // map<languageCode, map<translationKey, translationString>>

    Translations() = default;

    static Translations& Instance() {
        static Translations instance;
        return instance;
    }

    std::string Get(const std::string& translationKey) const {
        const std::string& currentLanguageCode = Settings::Instance().language;
        auto currentLangIt = translations.find(currentLanguageCode);

        if (currentLangIt != translations.end()) {
            auto keyIt = currentLangIt->second.find(translationKey);

            if (keyIt != currentLangIt->second.end()) {
                return keyIt->second;
            }

            spdlog::trace("Translation key '{}' not found for language '{}'", translationKey, currentLanguageCode);
        } else {
            spdlog::trace("Language '{}' not found in translations", currentLanguageCode);
        }

        if (currentLanguageCode != "en") {
            auto fallbackLangIt = translations.find("en");

            if (fallbackLangIt != translations.end()) {
                auto keyIt = fallbackLangIt->second.find(translationKey);

                if (keyIt != fallbackLangIt->second.end()) {
                    return keyIt->second;
                }

                spdlog::trace("Translation key '{}' not found in fallback language 'en'", translationKey);
            } else {
                spdlog::trace("Fallback language 'en' not found in translations");
            }
        }

        return translationKey;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Translations, translations)

inline void LoadTranslations() {
    std::string translationsFilePath = GetDataFilePath("translations.json");

    if (!std::filesystem::exists(translationsFilePath)) {
        spdlog::error("Translations file not found: '{}'. No translations loaded", translationsFilePath);
        Translations::Instance().translations.clear();
        return;
    }

    try {
        std::ifstream f(translationsFilePath);
        if (!f.is_open()) {
            spdlog::error("Failed to open translations file for reading: '{}'", translationsFilePath);
            return;
        }

        nlohmann::json jsonData = nlohmann::json::parse(f);
        f.close();

        Translations::Instance() = jsonData.get<Translations>();

        spdlog::info("Loaded translations from '{}'", translationsFilePath);
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("Failed to parse translations file '{}': {}", translationsFilePath, e.what());
        Translations::Instance().translations.clear();
    } catch (const nlohmann::json::exception& e) {
        spdlog::error("JSON error processing translations file '{}': {}", translationsFilePath, e.what());
        Translations::Instance().translations.clear();
    } catch (const std::exception& e) {
        spdlog::error("Error loading translations from '{}': {}", translationsFilePath, e.what());
        Translations::Instance().translations.clear();
    } catch (...) {
        spdlog::error("Unknown error loading translations from '{}'", translationsFilePath);
        Translations::Instance().translations.clear();
    }
}

inline void SaveTranslations(const std::string& filePath) {
    try {
        nlohmann::json data = Translations::Instance();

        std::ofstream f(filePath);
        if (!f.is_open()) {
            spdlog::error("Failed to open translations file for writing: '{}'", filePath);
            return;
        }

        f << data.dump(4);
        f.close();

        spdlog::info("Saved translations to '{}'", filePath);
    } catch (const std::exception& e) {
        spdlog::error("Error saving translations to '{}': {}", filePath, e.what());
    } catch (...) {
        spdlog::error("Unknown error saving translations to '{}'", filePath);
    }
}

#define TL(key) Translations::Instance().Get(key).c_str()
