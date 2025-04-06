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
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Translations, translations)

class Translate {
  private:
    Translations _translations;

    Translate() = default;

  public:
    Translate(const Translate&) = delete;
    Translate& operator=(const Translate&) = delete;

    static Translate& Instance() {
        static Translate instance;
        return instance;
    }

    void Load() {
        std::string translationsFilePath = Settings::GetDataFilePath("translations.json");

        if (!std::filesystem::exists(translationsFilePath)) {
            spdlog::error("Translations file not found: '{}'. No translations loaded", translationsFilePath);
            _translations.translations.clear();
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

            _translations = jsonData.get<Translations>();

            spdlog::info("Loaded translations from '{}'", translationsFilePath);
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("Failed to parse translations file '{}': {}", translationsFilePath, e.what());
            _translations.translations.clear();
        } catch (const nlohmann::json::exception& e) {
            spdlog::error("JSON error processing translations file '{}': {}", translationsFilePath, e.what());
            _translations.translations.clear();
        } catch (const std::exception& e) {
            spdlog::error("Error loading translations from '{}': {}", translationsFilePath, e.what());
            _translations.translations.clear();
        } catch (...) {
            spdlog::error("Unknown error loading translations from '{}'", translationsFilePath);
            _translations.translations.clear();
        }
    }

    std::string Get(const std::string& translationKey) const {
        const std::string& currentLanguageCode = Settings::Instance().language;
        auto currentLangIt = _translations.translations.find(currentLanguageCode);

        if (currentLangIt != _translations.translations.end()) {
            auto keyIt = currentLangIt->second.find(translationKey);

            if (keyIt != currentLangIt->second.end()) {
                return keyIt->second;
            }

            spdlog::trace("Translation key '{}' not found for language '{}'", translationKey, currentLanguageCode);
        } else {
            spdlog::trace("Language '{}' not found in translations", currentLanguageCode);
        }

        if (currentLanguageCode != "en") {
            auto fallbackLangIt = _translations.translations.find("en");

            if (fallbackLangIt != _translations.translations.end()) {
                auto keyIt = fallbackLangIt->second.find(translationKey);

                if (keyIt != fallbackLangIt->second.end()) {
                    return keyIt->second;
                }

                spdlog::trace("Translation key '{}' not found in fallback language 'en'", translationKey);
            } else {
                spdlog::trace("Fallback language 'en' not found in translations");
            }
        }

        spdlog::warn("Translation key '{}' not found for language '{}' or fallback 'en'", translationKey, currentLanguageCode);
        return translationKey;
    }
};

#define TR(key) Translate::Instance().Get(key)
