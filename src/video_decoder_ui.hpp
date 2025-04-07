#pragma once

#include "translate.hpp"
#include "video_decoder.hpp"

#include <glad/glad.h>
#include <imgui.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>

class VideoDecoderUI {
private:
    VideoDecoder _videoDecoder;
    
    // Video rendering
    GLuint _videoTexture = 0;
    GLuint _videoVAO = 0;
    GLuint _videoVBO = 0;
    GLuint _videoShaderProgram = 0;
    
    // Audio playback
    SDL_AudioStream* _audioStream = nullptr;
    SDL_AudioDeviceID _audioDevice = 0;
    
    std::function<void(double)> _onPositionChanged;

public:
    VideoDecoderUI() = default;
    
    void Init() {
        InitVideoRendering();
        InitAudioPlayback();
    }
    
    void Cleanup() {
        _videoDecoder.Close();

        if (_audioStream) {
            SDL_DestroyAudioStream(_audioStream);
            _audioStream = nullptr;
            _audioDevice = 0;
        }

        if (_videoTexture) {
            glDeleteTextures(1, &_videoTexture);
            _videoTexture = 0;
        }

        if (_videoVAO) {
            glDeleteVertexArrays(1, &_videoVAO);
            _videoVAO = 0;
        }

        if (_videoVBO) {
            glDeleteBuffers(1, &_videoVBO);
            _videoVBO = 0;
        }

        if (_videoShaderProgram) {
            glDeleteProgram(_videoShaderProgram);
            _videoShaderProgram = 0;
        }
    }
    
    void InitVideoRendering() {
        // Create shader program for video rendering
        const char* vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec2 aTexCoord;

            out vec2 TexCoord;

            void main() {
                gl_Position = vec4(aPos, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        const char* fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            in vec2 TexCoord;

            uniform sampler2D videoTexture;

            void main() {
                FragColor = texture(videoTexture, TexCoord);
            }
        )";

        // Compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        // Compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        // Create shader program
        _videoShaderProgram = glCreateProgram();
        glAttachShader(_videoShaderProgram, vertexShader);
        glAttachShader(_videoShaderProgram, fragmentShader);
        glLinkProgram(_videoShaderProgram);

        // Clean up shaders
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Create texture for video frames
        glGenTextures(1, &_videoTexture);
        glBindTexture(GL_TEXTURE_2D, _videoTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Create VAO and VBO for rendering video quad
        float vertices[] = {
            // positions        // texture coords
            -1.0f, 1.0f,  0.0f, 0.0f, 0.0f, // top left
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // bottom left
            1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, // bottom right
            -1.0f, 1.0f,  0.0f, 0.0f, 0.0f, // top left
            1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, // bottom right
            1.0f,  1.0f,  0.0f, 1.0f, 0.0f  // top right
        };

        glGenVertexArrays(1, &_videoVAO);
        glGenBuffers(1, &_videoVBO);

        glBindVertexArray(_videoVAO);

        glBindBuffer(GL_ARRAY_BUFFER, _videoVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void InitAudioPlayback() {
        // Set up SDL audio with SDL3 API
        const SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 44100 };
    
        // Create an audio stream and bind it to the default playback device
        _audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
        if (!_audioStream) {
            spdlog::error("Failed to open audio device stream: {}", SDL_GetError());
            return;
        }
    
        // Get the device ID from the stream
        _audioDevice = SDL_GetAudioStreamDevice(_audioStream);
        if (!_audioDevice) {
            spdlog::error("Failed to get audio device from stream: {}", SDL_GetError());
            SDL_DestroyAudioStream(_audioStream);
            _audioStream = nullptr;
            return;
        }

        spdlog::info("Audio device opened: {} Hz, {} channels", spec.freq, spec.channels);

        // Start paused - we'll resume when playback begins
        SDL_PauseAudioDevice(_audioDevice);
    }

    void FillAudioBuffer(std::shared_ptr<AudioFrame> frame) {
        if (!_audioStream || !frame || !frame->data || frame->size <= 0) {
            return;
        }

        // Put audio data into the stream
        SDL_PutAudioStreamData(_audioStream, frame->data, frame->size);
    }
    
    bool Open(const std::string& filePath) {
        return _videoDecoder.Open(filePath);
    }
    
    void Start() {
        _videoDecoder.Start();
        if (_audioDevice) {
            SDL_ResumeAudioDevice(_audioDevice);
        }
    }
    
    void Pause() {
        _videoDecoder.Pause();
        if (_audioDevice) {
            SDL_PauseAudioDevice(_audioDevice);
        }
    }
    
    void Resume() {
        _videoDecoder.Resume();
        if (_audioDevice) {
            SDL_ResumeAudioDevice(_audioDevice);
        }
    }
    
    void Stop() {
        _videoDecoder.Stop();
        if (_audioDevice) {
            SDL_PauseAudioDevice(_audioDevice);
        }
    }
    
    void Seek(double position) {
        _videoDecoder.Seek(position);
    }
    
    void Update() {
        if (_videoDecoder.IsRunning() && !_videoDecoder.IsPaused()) {
            // Process video frames
            auto frame = _videoDecoder.GetNextVideoFrame();
            if (frame) {
                glBindTexture(GL_TEXTURE_2D, _videoTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->width, frame->height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame->data);
                glBindTexture(GL_TEXTURE_2D, 0);

                // Update timeline position
                if (_onPositionChanged) {
                    _onPositionChanged(frame->pts);
                }
            }
            
            // Process audio frames
            auto audioFrame = _videoDecoder.GetNextAudioFrame();
            if (audioFrame) {
                FillAudioBuffer(audioFrame);
            }
        }
    }
    
    void RenderVideoPanel(ImVec2 windowSize) {
        if (_videoDecoder.IsRunning() && _videoDecoder.GetVideoWidth() > 0) {
            // Calculate aspect ratio
            float videoAspect = (float)_videoDecoder.GetVideoWidth() / _videoDecoder.GetVideoHeight();
            float windowAspect = windowSize.x / windowSize.y;

            float displayWidth, displayHeight;
            if (videoAspect > windowAspect) {
                // Video is wider than window
                displayWidth = windowSize.x;
                displayHeight = windowSize.x / videoAspect;
            } else {
                // Window is wider than video
                displayHeight = windowSize.y;
                displayWidth = windowSize.y * videoAspect;
            }

            // Center the video in the window
            ImVec2 pos(ImGui::GetCursorPosX() + (windowSize.x - displayWidth) * 0.5f, 
                       ImGui::GetCursorPosY() + (windowSize.y - displayHeight) * 0.5f);

            // Render the video texture
            ImGui::SetCursorPos(pos);
            ImGui::Image((ImTextureID)(intptr_t)_videoTexture, ImVec2(displayWidth, displayHeight));
        } else {
            // No video playing, show placeholder text
            ImVec2 textSize = ImGui::CalcTextSize(TL("panel_video"));
            ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
            ImGui::Text("%s", TL("panel_video"));
            ImGui::PopStyleColor();
        }
    }
    
    void RenderControlsPanel(float& timelinePosition, float timelineDuration, bool& isPlaying, const std::string& videoPath = "") {
        if (ImGui::Button(isPlaying ? TL("controls_pause") : TL("controls_play"))) {
            isPlaying = !isPlaying;

            if (isPlaying) {
                // Start or resume playback
                if (!_videoDecoder.IsRunning()) {
                    // Open and start the video if not already running
                    if (!videoPath.empty()) {
                        if (Open(videoPath)) {
                            Start();
                        }
                    }
                } else {
                    // Resume if already running
                    Resume();
                }
            } else {
                // Pause playback
                Pause();
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        float prevPos = timelinePosition;
        if (ImGui::SliderFloat("##timeline", &timelinePosition, 0.0f, timelineDuration, "%.1f s")) {
            // User moved the slider, seek to the new position
            if (_videoDecoder.IsRunning() && prevPos != timelinePosition) {
                Seek(timelinePosition);
            }
        }

        ImGui::PopItemWidth();
    }
    
    bool IsRunning() const {
        return _videoDecoder.IsRunning();
    }
    
    bool IsPaused() const {
        return _videoDecoder.IsPaused();
    }
    
    double GetDuration() const {
        return _videoDecoder.GetDuration();
    }
    
    double GetCurrentPosition() const {
        return _videoDecoder.GetCurrentPosition();
    }
    
    int GetVideoWidth() const {
        return _videoDecoder.GetVideoWidth();
    }
    
    int GetVideoHeight() const {
        return _videoDecoder.GetVideoHeight();
    }
    
    void SetPositionChangedCallback(std::function<void(double)> callback) {
        _onPositionChanged = callback;
    }
};
