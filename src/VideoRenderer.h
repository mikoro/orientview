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

#include "MovingAverage.h"

namespace OrientView
{
	class VideoDecoder;
	class QuickRouteJpegReader;
	class VideoStabilizer;
	class VideoEncoder;
	class VideoWindow;
	class Settings;
	struct FrameData;

	struct Panel
	{
		QOpenGLShaderProgram* program = nullptr;
		QOpenGLBuffer* buffer = nullptr;
		QOpenGLTexture* texture = nullptr;

		QMatrix4x4 vertexMatrix;

		double textureWidth = 0.0;
		double textureHeight = 0.0;
		double texelWidth = 0.0;
		double texelHeight = 0.0;

		GLuint vertexMatrixUniform = 0;
		GLuint vertexPositionAttribute = 0;
		GLuint vertexTextureCoordinateAttribute = 0;
		GLuint textureSamplerUniform = 0;
		GLuint textureWidthUniform = 0;
		GLuint textureHeightUniform = 0;
		GLuint texelWidthUniform = 0;
		GLuint texelHeightUniform = 0;
	};

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

		bool loadShaders(Panel* panel, const QString& shaderName);
		void loadBuffer(Panel* panel, GLfloat* buffer, int size);
		void renderPanel(Panel* panel);

		VideoDecoder* videoDecoder = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		VideoEncoder* videoEncoder = nullptr;
		VideoWindow* videoWindow = nullptr;

		bool flipOutput = false;

		double mapPanelRelativeWidth = 0.0;
		double mapPanelScale = 0.0;
		double mapPanelX = 0.0;
		double mapPanelY = 0.0;
		double windowWidth = 0.0;
		double windowHeight = 0.0;
		double frameTime = 0.0;
		double lastRenderTime = 0.0;

		QElapsedTimer renderTimer;

		MovingAverage averageFps;
		MovingAverage averageFrameTime;
		MovingAverage averageDecodeTime;
		MovingAverage averageStabilizeTime;
		MovingAverage averageRenderTime;
		MovingAverage averageEncodeTime;
		MovingAverage averageSpareTime;

		QOpenGLPaintDevice* paintDevice = nullptr;
		QPainter* painter = nullptr;

		Panel videoPanel;
		Panel mapPanel;
	};
}
