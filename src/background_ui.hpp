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

        void main() {
            vec2 uv = TexCoord;

            float wave = sin((uv.x + uv.y) * 4.0 - uTime * 0.5) * 0.5 + 0.5;

            vec3 deepBlue = vec3(0.05, 0.05, 0.15);
            vec3 purple = vec3(0.2, 0.05, 0.3);
            vec3 teal = vec3(0.0, 0.2, 0.25);

            float t1 = sin(uTime * 0.2) * 0.5 + 0.5;
            float t2 = cos(uTime * 0.3) * 0.5 + 0.5;

            vec3 color = mix(deepBlue, purple, t1);
            color = mix(color, teal, t2 * wave);

            float dist = length(uv - vec2(0.5, 0.5));
            float pulse = 0.05 * sin(uTime * 0.7 + dist * 5.0);

            color += vec3(0.02, 0.01, 0.04) * sin(uv.x * 10.0 + uTime);
            color += vec3(0.01, 0.02, 0.03) * cos(uv.y * 8.0 - uTime * 0.2);

            color = color + pulse;
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
    }

    void SetupQuad() {
        float vertices[] = {
            // positions        // texture coords
            -1.0f, 1.0f,  0.0f, 0.0f, 0.0f, // top left
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // bottom left
            1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, // bottom right
            -1.0f, 1.0f,  0.0f, 0.0f, 0.0f, // top left
            1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, // bottom right
            1.0f,  1.0f,  0.0f, 1.0f, 0.0f  // top right
        };

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);

        glBindVertexArray(_vao);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

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
            glUniform1f(_timeLocation, currentTime * 0.1f);
        }

        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glUseProgram(0);
    }
};
