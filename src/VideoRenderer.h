// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QElapsedTimer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

namespace OrientView
{
	class VideoDecoder;
	class QuickRouteJpegReader;
	class VideoStabilizer;
	class VideoEncoder;
	class Settings;
	struct FrameData;

	class VideoRenderer : protected QOpenGLFunctions
	{

	public:

		VideoRenderer();

		bool initialize(VideoDecoder* videoDecoder, QuickRouteJpegReader* quickRouteJpegReader, VideoStabilizer* videoStabilizer, VideoEncoder* videoEncoder, Settings* settings);
		void shutdown();

		void startRendering(int windowWidth, int windowHeight);
		void uploadFrameData(FrameData* frameData);
		void renderVideoPanel(int windowWidth, int windowHeight);
		void renderMapPanel(int windowWidth, int windowHeight);
		void renderInfoPanel(int windowWidth, int windowHeight, double frameTime, double spareTime);
		void stopRendering();

		void setFlipOutput(bool value);

	private:

		VideoDecoder* videoDecoder = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		VideoEncoder* videoEncoder = nullptr;

		int videoFrameWidth = 0;
		int videoFrameHeight = 0;
		int mapImageWidth = 0;
		int mapImageHeight = 0;
		float mapPanelWidth = 0.0f;
		bool flipOutput = false;

		QElapsedTimer renderTimer;
		double averageRenderTime = 0.0;

		QOpenGLShaderProgram* shaderProgram = nullptr;
		QOpenGLPaintDevice* paintDevice = nullptr;
		QPainter* painter = nullptr;
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
