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
	class VideoWindow;
	class Settings;
	struct FrameData;

	class VideoRenderer : protected QOpenGLFunctions
	{

	public:

		VideoRenderer();

		bool initialize(VideoDecoder* videoDecoder, QuickRouteJpegReader* quickRouteJpegReader, VideoStabilizer* videoStabilizer, VideoEncoder* videoEncoder, VideoWindow* videoWindow, Settings* settings);
		void shutdown();

		void startRendering(double windowWidth, double windowHeight, double frameTime);
		void uploadFrameData(FrameData* frameData);
		void renderVideoPanel();
		void renderMapPanel();
		void renderInfoPanel(double spareTime);
		void stopRendering();

		void setFlipOutput(bool value);

	private:

		VideoDecoder* videoDecoder = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		VideoEncoder* videoEncoder = nullptr;
		VideoWindow* videoWindow = nullptr;

		bool flipOutput = false;

		double videoFrameWidth = 0.0;
		double videoFrameHeight = 0.0;
		double mapImageWidth = 0.0;
		double mapImageHeight = 0.0;
		double mapPanelRelativeWidth = 0.0;
		double mapPanelScale = 0.0;
		double windowWidth = 0.0;
		double windowHeight = 0.0;
		double frameTime = 0.0;

		QElapsedTimer renderTimer;
		double averageRenderTime = 0.0;

		QOpenGLShaderProgram* shaderProgram = nullptr;
		QOpenGLBuffer* videoPanelBuffer = nullptr;
		QOpenGLTexture* videoPanelTexture = nullptr;
		QOpenGLBuffer* mapPanelBuffer = nullptr;
		QOpenGLTexture* mapPanelTexture = nullptr;
		QOpenGLPaintDevice* paintDevice = nullptr;
		QPainter* painter = nullptr;
		
		QMatrix4x4 videoPanelVertexMatrix;
		QMatrix4x4 mapPanelVertexMatrix;

		GLuint vertexMatrixUniform = 0;
		GLuint vertexPositionAttribute = 0;
		GLuint vertexTextureCoordinateAttribute = 0;
		GLuint textureSamplerUniform = 0;
	};
}
