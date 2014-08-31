// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QSettings>
#include <QColor>

#include "RouteManager.h"
#include "SplitsManager.h"
#include "VideoStabilizer.h"
#include "Renderer.h"

namespace Ui
{
	class MainWindow;
}

namespace OrientView
{
	// Central structure of all the settings of the program.
	class Settings
	{

	public:

		void readFromQSettings(QSettings* settings);	// Read values from a QSettings instance.
		void writeToQSettings(QSettings* settings);		// Write values to a QSettings instance.
		void readFromUI(Ui::MainWindow* ui);			// Read values from the UI.
		void writeToUI(Ui::MainWindow* ui);				// Write values to the UI.

		struct Map
		{
			QString imageFilePath = "";
			double relativeWidth = 0.25;
			double x = 0.0;
			double y = 0.0;
			double angle = 0.0;
			double scale = 1.0;
			QColor backgroundColor = QColor(255, 255, 255, 255);
			int headerCrop = 0;
			QString rescaleShader = "default";

		} map;

		struct Route
		{
			QString quickRouteJpegFilePath = "";
			QColor discreetColor = QColor(0, 0, 0, 50);
			QColor highlightColor = QColor(0, 100, 255, 200);
			RouteRenderMode routeRenderMode = RouteRenderMode::Discreet;
			double routeWidth = 10.0;
			RouteRenderMode tailRenderMode = RouteRenderMode::None;
			double tailWidth = 10.0;
			double tailLength = 60.0;
			QColor controlBorderColor = QColor(140, 40, 140, 255);
			double controlRadius = 15.0;
			double controlBorderWidth = 5.0;
			bool showControls = true;
			QColor runnerColor = QColor(0, 100, 255, 255);
			QColor runnerBorderColor = QColor(0, 0, 0, 255);
			double runnerBorderWidth = 1.0;
			double runnerScale = 1.0;
			bool showRunner = true;
			double controlTimeOffset = 0.0;
			double runnerTimeOffset = 0.0;
			double scale = 1.0;
			double lowPace = 15.0;
			double highPace = 5.0;

		} route;

		struct RouteManager
		{
			ViewMode viewMode = ViewMode::FixedSplit;
			bool useSmoothSplitTransition = true;
			double smoothSplitTransitionSpeed = 1.0;
			double topBottomMargin = 30.0;
			double leftRightMargin = 10.0;
			double maximumAutomaticZoom = 100.0;
			double runnerAveragingFactor = 4.0;
			double runnerVerticalOffset = 0.0;

		} routeManager;

		struct Video
		{
			QString inputVideoFilePath = "";
			double startTimeOffset = 0.0;
			bool seekToAnyFrame = false;
			double x = 0.0;
			double y = 0.0;
			double angle = 0.0;
			double scale = 1.0;
			QColor backgroundColor = QColor(0, 50, 0, 255);
			QString rescaleShader = "default";
			bool enableClipping = false;
			bool enableClearing = true;
			int frameCountDivisor = 1;
			int frameDurationDivisor = 1;
			int frameSizeDivisor = 1;
			bool enableVerboseLogging = false;

		} video;

		struct Splits
		{
			SplitTimeType type = SplitTimeType::Absolute;
			QString splitTimes = "";

		} splits;

		struct Window
		{
			int width = 1280;
			int height = 720;
			int multisamples = 16;
			bool fullscreen = false;
			bool hideCursor = false;
			bool showInfoPanel = false;

		} window;

		struct Stabilizer
		{
			bool enabled = false;
			VideoStabilizerMode mode = VideoStabilizerMode::RealTime;
			QString inputDataFilePath = "";
			double averagingFactor = 0.1;
			double dampingFactor = 1.0;
			double maxDisplacementFactor = 0.5;
			double maxAngle = 15.0;
			int frameSizeDivisor = 8;
			QString passOneOutputFilePath = "";
			QString passTwoInputFilePath = "";
			QString passTwoOutputFilePath = "";
			int smoothingRadius = 15;

		} stabilizer;

		struct Encoder
		{
			QString outputVideoFilePath = "";
			QString preset = "veryfast";
			QString profile = "high";
			int constantRateFactor = 23;

		} encoder;

		struct Renderer
		{
			RenderMode renderMode = RenderMode::All;

		} renderer;

		struct InputHandler
		{
			double smallSeekAmount = 2.0;
			double normalSeekAmount = 10.0;
			double largeSeekAmount = 60.0;
			double veryLargeSeekAmount = 600.0;

			double slowTranslateSpeed = 0.1;
			double normalTranslateSpeed = 0.5;
			double fastTranslateSpeed = 1.0;
			double veryFastTranslateSpeed = 1.0;

			double slowRotateSpeed = 0.02;
			double normalRotateSpeed = 0.1;
			double fastRotateSpeed = 0.5;
			double veryFastRotateSpeed = 0.5;

			double slowScaleSpeed = 0.00025;
			double normalScaleSpeed = 0.002;
			double fastScaleSpeed = 0.005;
			double veryFastScaleSpeed = 0.005;

			double smallTimeOffset = 0.25;
			double normalTimeOffset = 1.0;
			double largeTimeOffset = 5.0;
			double veryLargeTimeOffset = 20.0;

		} inputHandler;
	};
}
