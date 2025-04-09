#pragma once

#include "video_decoder.hpp"

#include <glad/glad.h>
#include <imgui.h>

#include <memory>

class VideoUI {
  private:
    GLuint _videoTexture = 0;
    GLuint _videoVAO = 0;
    GLuint _videoVBO = 0;
    GLuint _videoShaderProgram = 0;

  public:
    VideoUI() = default;

    void Init() {
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

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        _videoShaderProgram = glCreateProgram();
        glAttachShader(_videoShaderProgram, vertexShader);
        glAttachShader(_videoShaderProgram, fragmentShader);
        glLinkProgram(_videoShaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glGenTextures(1, &_videoTexture);
        glBindTexture(GL_TEXTURE_2D, _videoTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

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

    void Close() {
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

    void UpdateTexture(const std::shared_ptr<VideoFrame>& frame) {
        if (frame) {
            glBindTexture(GL_TEXTURE_2D, _videoTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->width, frame->height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame->data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void RenderVideoWindow(ImVec2 windowSize, int videoWidth, int videoHeight) {
        if (videoWidth > 0 && videoHeight > 0) {
            float videoAspect = (float)videoWidth / videoHeight;
            float windowAspect = windowSize.x / windowSize.y;
            float displayWidth, displayHeight;

            if (videoAspect > windowAspect) {
                displayWidth = windowSize.x;
                displayHeight = windowSize.x / videoAspect;
            } else {
                displayHeight = windowSize.y;
                displayWidth = windowSize.y * videoAspect;
            }

            ImVec2 pos(ImGui::GetCursorPosX() + (windowSize.x - displayWidth) * 0.5f, ImGui::GetCursorPosY() + (windowSize.y - displayHeight) * 0.5f);
            ImGui::SetCursorPos(pos);
            ImGui::Image((ImTextureID)(intptr_t)_videoTexture, ImVec2(displayWidth, displayHeight));
        }
    }
};
