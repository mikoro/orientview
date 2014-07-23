// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <memory>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

namespace OrientView
{
	class VideoRenderer : protected QOpenGLFunctions
	{

	public:

		VideoRenderer();
		~VideoRenderer();

		bool initialize();
		void shutdown();

		void render();

		QOpenGLTexture* getVideoPanelTexture();

	private:

		std::unique_ptr<QOpenGLShaderProgram> shaderProgram = nullptr;
		std::unique_ptr<QOpenGLBuffer> videoPanelBuffer = nullptr;
		std::unique_ptr<QOpenGLTexture> videoPanelTexture = nullptr;
		std::unique_ptr<QOpenGLBuffer> mapPanelBuffer = nullptr;
		std::unique_ptr<QOpenGLTexture> mapPanelTexture = nullptr;
		
		GLuint vertexMatrixUniform = 0;
		GLuint textureMatrixUniform = 0;
		GLuint textureSamplerUniform = 0;
		GLuint vertexCoordAttribute = 0;
		GLuint textureCoordAttribute = 0;
	};
}
