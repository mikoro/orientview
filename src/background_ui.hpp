#pragma once

#include "settings.hpp"

#include <glad/glad.h>
#include <stdexcept>
#include <string>

class BackgroundUI {
  private:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _shaderProgram = 0;
    GLint _timeLocation = -1;
    GLint _resolutionLocation = -1;

    const std::string _vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const std::string _fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;
        
        uniform float uTime;
        uniform vec2 uResolution;
        
        // Function to create a smooth noise pattern
        float noise(vec2 st) {
            return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
        }
        
        void main() {
            vec2 uv = TexCoord;
            
            // Create a moving wave pattern that covers the whole screen
            float wave = sin((uv.x + uv.y) * 4.0 - uTime * 0.5) * 0.5 + 0.5;
            
            // Create color palette with more interesting colors
            vec3 deepBlue = vec3(0.05, 0.05, 0.15);
            vec3 purple = vec3(0.2, 0.05, 0.3);
            vec3 teal = vec3(0.0, 0.2, 0.25);
            
            // Create multiple oscillating values for color mixing
            float t1 = sin(uTime * 0.2) * 0.5 + 0.5;
            float t2 = cos(uTime * 0.3) * 0.5 + 0.5;
            
            // Mix colors in a more complex way
            vec3 color = mix(deepBlue, purple, t1);
            color = mix(color, teal, t2 * wave);
            
            // Add subtle noise
            float noiseValue = noise(uv + uTime * 0.05) * 0.03;
            
            // Calculate distance for the pulse effect
            float dist = length(uv - vec2(0.5, 0.5));
            
            // Add subtle pulsing glow
            float pulse = 0.05 * sin(uTime * 0.7 + dist * 5.0);
            
            // Add subtle color variations based on position
            color += vec3(0.02, 0.01, 0.04) * sin(uv.x * 10.0 + uTime);
            color += vec3(0.01, 0.02, 0.03) * cos(uv.y * 8.0 - uTime * 0.2);
            
            // Apply pulse and noise without vignette
            color = color + pulse + noiseValue;
            
            FragColor = vec4(color, 1.0);
        }
    )";

    GLuint CompileShader(GLenum type, const std::string& source) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            throw std::runtime_error(std::string("Shader compilation failed: ") + infoLog);
        }

        return shader;
    }

    void CreateShaderProgram() {
        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, _vertexShaderSource);
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, _fragmentShaderSource);

        _shaderProgram = glCreateProgram();
        glAttachShader(_shaderProgram, vertexShader);
        glAttachShader(_shaderProgram, fragmentShader);
        glLinkProgram(_shaderProgram);

        GLint success;
        glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(_shaderProgram, 512, nullptr, infoLog);
            throw std::runtime_error(std::string("Shader program linking failed: ") + infoLog);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        _timeLocation = glGetUniformLocation(_shaderProgram, "uTime");
        _resolutionLocation = glGetUniformLocation(_shaderProgram, "uResolution");
    }

    void SetupQuad() {
        // Vertices for a fullscreen quad (2 triangles)
        float vertices[] = {-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);

        glBindVertexArray(_vao);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
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

  public:
    BackgroundUI() = default;

    void Init() {
        CreateShaderProgram();
        SetupQuad();
    }

    void Close() {
        if (_vbo) glDeleteBuffers(1, &_vbo);
        if (_vao) glDeleteVertexArrays(1, &_vao);
        if (_shaderProgram) glDeleteProgram(_shaderProgram);

        _vbo = 0;
        _vao = 0;
        _shaderProgram = 0;
    }

    void Render(float currentTime) {
        glUseProgram(_shaderProgram);

        if (_timeLocation != -1) {
            glUniform1f(_timeLocation, currentTime);
        }

        if (_resolutionLocation != -1) {
            glUniform2f(_resolutionLocation, static_cast<float>(Settings::Instance().windowWidth), static_cast<float>(Settings::Instance().windowHeight));
        }

        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glUseProgram(0);
    }
};
