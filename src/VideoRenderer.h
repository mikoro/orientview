// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

namespace OrientView
{
	class VideoDecoder;
	class QuickRouteJpegReader;
	class Settings;

	class VideoRenderer : protected QOpenGLFunctions
	{

	public:

		VideoRenderer();

		bool initialize(VideoDecoder* videoDecoder, QuickRouteJpegReader* quickRouteJpegReader, Settings* settings);
		void shutdown();

		void update(int windowWidth, int windowHeight);
		void render();

		void setFlipOutput(bool value);

		QOpenGLTexture* getVideoPanelTexture();

	private:

		int videoFrameWidth = 0;
		int videoFrameHeight = 0;
		int mapImageWidth = 0;
		int mapImageHeight = 0;

		float mapPanelWidth = 0.0f;

		bool flipOutput = false;

		QOpenGLShaderProgram* shaderProgram = nullptr;
		QOpenGLBuffer* videoPanelBuffer = nullptr;
		QOpenGLTexture* videoPanelTexture = nullptr;
		QOpenGLBuffer* mapPanelBuffer = nullptr;
		QOpenGLTexture* mapPanelTexture = nullptr;
		
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
