// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QSettings>
#include <QColor>

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
			QString mapImageFilePath = "";
			double mapPanelWidth = 0.3;
			double mapPanelScale = 1.0;
			QColor mapPanelBackgroundColor = QColor("#ffffff");
			QString mapPanelRescaleShader = "default";

		} map;

		struct Route
		{
			QString quickRouteJpegFilePath = "";
			double startOffset = 0.0;

		} route;

		struct Splits
		{
			SplitTimeType type = SplitTimeType::Absolute;
			QString splitTimes = "";

		} splits;

		struct Video
		{
			QString inputVideoFilePath = "";
			double startOffset = 0.0;
			double videoPanelScale = 1.0;
			QColor videoPanelBackgroundColor = QColor("#003200");
			QString videoPanelRescaleShader = "default";
			int frameCountDivisor = 1;
			int frameDurationDivisor = 1;
			int frameSizeDivisor = 1;
			bool enableVerboseLogging = false;

		} video;

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
			bool enableClipping = false;
			bool enableClearing = true;
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
			int smallSeekAmount = 2;
			int normalSeekAmount = 10;
			int largeSeekAmount = 60;
			int veryLargeSeekAmount = 600;

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
