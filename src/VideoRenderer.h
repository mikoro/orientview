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

		bool initialize(int videoWidth, int videoHeight);
		void shutdown();

		void update(int windowWidth, int windowHeight);
		void render();

		QOpenGLTexture* getVideoPanelTexture();

	private:

		int videoWidth = 0;
		int videoHeight = 0;

		std::unique_ptr<QOpenGLShaderProgram> shaderProgram = nullptr;
		std::unique_ptr<QOpenGLBuffer> videoPanelBuffer = nullptr;
		std::unique_ptr<QOpenGLTexture> videoPanelTexture = nullptr;
		std::unique_ptr<QOpenGLBuffer> mapPanelBuffer = nullptr;
		std::unique_ptr<QOpenGLTexture> mapPanelTexture = nullptr;
		
		QMatrix4x4 vertexMatrix;
		QMatrix4x4 textureMatrix;

		GLuint vertexMatrixUniform = 0;
		GLuint textureMatrixUniform = 0;
		GLuint textureSamplerUniform = 0;
		GLuint vertexCoordAttribute = 0;
		GLuint textureCoordAttribute = 0;
	};
}
