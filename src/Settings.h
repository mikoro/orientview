// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QSettings>
#include <QColor>

#include "RouteManager.h"
#include "SplitTimeManager.h"
#include "VideoStabilizer.h"

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
			double relativeWidth = 0.3;
			double x = 0.0;
			double y = 0.0;
			double angle = 0.0;
			double scale = 1.0;
			QColor backgroundColor = QColor(255, 255, 255, 255);
			QString rescaleShader = "legacy";

		} map;

		struct Route
		{
			QString quickRouteJpegFilePath = "";
			double controlTimeOffset = 0.0;
			double runnerTimeOffset = 0.0;
			double scale = 1.0;
			double minimumScale = 0.0;
			double maximumScale = 9999.0;
			double highPace = 5.0;
			double lowPace = 15.0;
			double topBottomMargin = 30.0;
			double leftRightMargin = 10.0;
			bool showRunner = true;
			bool showControls = true;
			RouteRenderMode wholeRouteRenderMode = RouteRenderMode::Normal;
			QColor wholeRouteColor = QColor(0, 0, 0, 80);
			double wholeRouteWidth = 10.0;
			QColor controlBorderColor = QColor(140, 40, 140, 255);
			double controlRadius = 15.0;
			double controlBorderWidth = 5.0;
			QColor runnerColor = QColor(0, 100, 255, 220);
			QColor runnerBorderColor = QColor(0, 0, 0, 255);
			double runnerBorderWidth = 1.0;
			double runnerScale = 1.0;

		} route;

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
			QString rescaleShader = "legacy";
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
			double maxDisplacementFactor = 1.0;
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

		struct InputHandler
		{
			double smallSeekAmount = 2.0;
			double normalSeekAmount = 10.0;
			double largeSeekAmount = 60.0;
			double veryLargeSeekAmount = 600.0;

			double slowTranslateVelocity = 0.1;
			double normalTranslateVelocity = 0.5;
			double fastTranslateVelocity = 1.0;

			double slowRotateVelocity = 0.02;
			double normalRotateVelocity = 0.1;
			double fastRotateVelocity = 0.5;

			double smallScaleConstant = 5000.0;
			double normalScaleConstant = 500.0;
			double largeScaleConstant = 100.0;

		} inputHandler;
	};
}
