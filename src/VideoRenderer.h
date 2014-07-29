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
	class VideoStabilizer;
	class Settings;

	class VideoRenderer : protected QOpenGLFunctions
	{

	public:

		VideoRenderer();

		bool initialize(VideoDecoder* videoDecoder, QuickRouteJpegReader* quickRouteJpegReader, VideoStabilizer* videoStabilizer, Settings* settings);
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

		VideoStabilizer* videoStabilizer = nullptr;

		QOpenGLShaderProgram* shaderProgram = nullptr;
		QOpenGLBuffer* videoPanelBuffer = nullptr;
		QOpenGLTexture* videoPanelTexture = nullptr;
		QOpenGLBuffer* mapPanelBuffer = nullptr;
		QOpenGLTexture* mapPanelTexture = nullptr;
		
		QMatrix4x4 videoPanelVertexMatrix;
		QMatrix4x4 videoPanelTextureMatrix;
		QMatrix4x4 mapPanelVertexMatrix;
		QMatrix4x4 mapPanelTextureMatrix;

		GLuint vertexMatrixUniform = 0;
		GLuint textureMatrixUniform = 0;
		GLuint textureSamplerUniform = 0;
		GLuint vertexCoordAttribute = 0;
		GLuint textureCoordAttribute = 0;
	};
}
