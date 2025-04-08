#pragma once

#include "session.hpp"
#include "translate.hpp"

#include <imgui.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

#include <mutex>
#include <string>

class LogUI {
  private:
    ImGuiTextBuffer _buffer;

    class LogSink : public spdlog::sinks::base_sink<std::mutex> {
      protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t formatted;
            formatter_->format(msg, formatted);
            std::string str = fmt::to_string(formatted);
            LogUI* log = static_cast<LogUI*>(_customData);
            if (log) {
                log->AddMessage(str);
            }
        }

        void flush_() override {}

      public:
        void* _customData = nullptr;

        explicit LogSink(void* logPtr) { _customData = logPtr; }
    };

    std::shared_ptr<LogSink> _sink;

  public:
    LogUI() = default;

    void Init() {
        _sink = std::make_shared<LogSink>(this);
        _sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::default_logger()->sinks().push_back(_sink);
    }

    void AddMessage(const std::string& msg) { _buffer.append(msg.c_str()); }

    static ImVec4 GetLevelColor(const std::string& logLevel) {
        if (logLevel == "trace") return {0.5f, 0.5f, 0.5f, 1.0f};    // More gray
        if (logLevel == "debug") return {0.7f, 0.7f, 0.7f, 1.0f};    // Gray
        if (logLevel == "info") return {0.0f, 0.8f, 0.0f, 1.0f};     // Green
        if (logLevel == "warning") return {1.0f, 0.8f, 0.0f, 1.0f};     // Yellow
        if (logLevel == "error") return {1.0f, 0.2f, 0.2f, 1.0f};    // Red
        if (logLevel == "critical") return {1.0f, 0.3f, 1.0f, 1.0f}; // Magenta
        return {1.0f, 1.0f, 1.0f, 1.0f};                             // White for unknown levels
    }

    void Render() {
        ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);

        if (ImGui::Begin(fmt::format("{}###window_log", TL("window_log")).c_str(), &Session::Instance().showLogWindow)) {
            ImGui::BeginChild("LogScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            // Parse and colorize log messages
            const char* text = _buffer.begin();
            const char* textEnd = _buffer.end();

            while (text < textEnd) {
                const char* lineStart = text;
                const char* lineEnd = text;

                // Find end of line
                while (lineEnd < textEnd && *lineEnd != '\n') lineEnd++;

                // Process this line
                std::string line(lineStart, lineEnd);

                // Look for log level pattern [level]
                size_t levelStart = line.find('[', 0);
                size_t levelEnd = line.find(']', levelStart);

                if (levelStart != std::string::npos && levelEnd != std::string::npos) {
                    // Find the second set of brackets which contains the log level
                    levelStart = line.find('[', levelStart + 1);
                    levelEnd = line.find(']', levelStart);

                    if (levelStart != std::string::npos && levelEnd != std::string::npos) {
                        // Extract timestamp part
                        ImGui::TextUnformatted(line.substr(0, levelStart).c_str());
                        ImGui::SameLine(0.0f, 0.0f);

                        // Extract and colorize log level - only color the text inside brackets
                        std::string logLevel = line.substr(levelStart + 1, levelEnd - levelStart - 1);

                        // Display opening bracket
                        ImGui::TextUnformatted("[");
                        ImGui::SameLine(0.0f, 0.0f);

                        // Display colored log level text
                        ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(logLevel));
                        ImGui::TextUnformatted(logLevel.c_str());
                        ImGui::PopStyleColor();
                        ImGui::SameLine(0.0f, 0.0f);

                        // Display closing bracket
                        ImGui::TextUnformatted("]");
                        ImGui::SameLine(0.0f, 0.0f);

                        // Extract message part
                        ImGui::TextUnformatted(line.substr(levelEnd + 1).c_str());
                    } else {
                        ImGui::TextUnformatted(line.c_str());
                    }
                } else {
                    ImGui::TextUnformatted(line.c_str());
                }

                // Move to next line
                text = lineEnd + 1;
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }
};
