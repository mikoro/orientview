// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <memory>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

#include "FFmpegDecoder.h"
#include "QuickRouteJpegReader.h"

namespace OrientView
{
	class VideoRenderer : protected QOpenGLFunctions
	{

	public:

		VideoRenderer();

		bool initialize(const FFmpegDecoder& ffmpegDecoder, const QuickRouteJpegReader& quickRouteJpegReader);
		void shutdown();

		void update(int windowWidth, int windowHeight);
		void render();

		QOpenGLTexture* getVideoPanelTexture();

	private:

		int videoWidth = 0;
		int videoHeight = 0;
		int mapWidth = 0;
		int mapHeight = 0;

		std::unique_ptr<QOpenGLShaderProgram> shaderProgram = nullptr;
		std::unique_ptr<QOpenGLBuffer> videoPanelBuffer = nullptr;
		std::unique_ptr<QOpenGLTexture> videoPanelTexture = nullptr;
		std::unique_ptr<QOpenGLBuffer> mapPanelBuffer = nullptr;
		std::unique_ptr<QOpenGLTexture> mapPanelTexture = nullptr;
		
		QMatrix4x4 videoVertexMatrix;
		QMatrix4x4 videoTextureMatrix;
		QMatrix4x4 mapVertexMatrix;
		QMatrix4x4 mapTextureMatrix;

		GLuint vertexMatrixUniform = 0;
		GLuint textureMatrixUniform = 0;
		GLuint textureSamplerUniform = 0;
		GLuint vertexCoordAttribute = 0;
		GLuint textureCoordAttribute = 0;
	};
}
