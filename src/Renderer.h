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
#include "TrackPoint.h"

namespace OrientView
{
	class VideoDecoder;
	class GpxReader;
	class MapImageReader;
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

		double x = 0.0;
		double y = 0.0;
		double angle = 0.0;
		double scale = 1.0;
		double userX = 0.0;
		double userY = 0.0;
		double userAngle = 0.0;
		double userScale = 1.0;

		GLuint vertexMatrixUniform = 0;
		GLuint vertexPositionAttribute = 0;
		GLuint vertexTextureCoordinateAttribute = 0;
		GLuint textureSamplerUniform = 0;
		GLuint textureWidthUniform = 0;
		GLuint textureHeightUniform = 0;
		GLuint texelWidthUniform = 0;
		GLuint texelHeightUniform = 0;
	};

	enum class SelectedPanel { NONE, VIDEO, MAP };
	enum class RenderMode { BOTH, VIDEO, MAP };

	class Renderer : protected QOpenGLFunctions
	{

	public:

		Renderer();

		bool initialize(VideoDecoder* videoDecoder, GpxReader* gpxReader, MapImageReader* mapImageReader, VideoStabilizer* videoStabilizer, VideoEncoder* videoEncoder, VideoWindow* videoWindow, Settings* settings);
		void shutdown();

		void handleInput();

		void startRendering(double windowWidth, double windowHeight, double frameTime, double spareTime);
		void uploadFrameData(FrameData* frameData);
		void renderAll();
		void stopRendering();

		void setFlipOutput(bool value);

	private:

		bool loadShaders(Panel* panel, const QString& shaderName);
		void loadBuffer(Panel* panel, GLfloat* buffer, int size);
		void loadTrackPoints(const std::vector<TrackPoint>& newTrackPoints);
		void renderVideoPanel();
		void renderMapPanel();
		void renderInfoPanel();
		void renderPanel(Panel* panel);
		void renderRoute();

		VideoDecoder* videoDecoder = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		VideoEncoder* videoEncoder = nullptr;
		VideoWindow* videoWindow = nullptr;

		bool flipOutput = false;
		bool showInfoPanel = false;

		double mapPanelRelativeWidth = 0.0;
		double windowWidth = 0.0;
		double windowHeight = 0.0;
		double frameTime = 0.0;
		double spareTime = 0.0;
		double lastRenderTime = 0.0;

		Panel videoPanel;
		Panel mapPanel;
		SelectedPanel selectedPanel = SelectedPanel::NONE;
		RenderMode renderMode = RenderMode::BOTH;
		Panel* selectedPanelPtr = nullptr;

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
		QPainterPath* routePath = nullptr;
	};
}
