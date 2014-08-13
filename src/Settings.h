// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QSettings>
#include <QColor>

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

		struct VideoDecoder
		{
			QString inputVideoFilePath = "";
			int frameCountDivisor = 1;
			int frameDurationDivisor = 1;
			int frameSizeDivisor = 1;
			bool enableVerboseLogging = false;

		} videoDecoder;

		struct MapAndRoute
		{
			QString quickRouteJpegFilePath = "";
			QString mapImageFilePath = "";
			double routeStartOffset = 0.0;

		} mapAndRoute;

		struct SplitTimes
		{
			QString splitTimes = "";

		} splitTimes;

		struct Window
		{
			int width = 1280;
			int height = 720;
			int multisamples = 16;
			bool fullscreen = false;
			bool hideCursor = false;

		} window;

		struct Appearance
		{
			bool showInfoPanel = false;
			double mapPanelWidth = 0.3;
			double videoPanelScale = 1.0;
			double mapPanelScale = 1.0;
			QColor videoPanelBackgroundColor = QColor("#003200");
			QColor mapPanelBackgroundColor = QColor("#ffffff");
			QString videoPanelShader = "default";
			QString mapPanelShader = "default";

		} appearance;

		struct VideoStabilizer
		{
			bool enabled = false;
			int frameSizeDivisor = 8;
			double averagingFactor = 0.05;
			double dampingFactor = 0.5;
			double maxDisplacementFactor = 1.0;
			bool enableClipping = true;
			bool enableClearing = true;
			int inpaintBorderWidth = 0;

		} videoStabilizer;

		struct VideoEncoder
		{
			QString outputVideoFilePath = "";
			QString preset = "veryfast";
			QString profile = "high";
			int constantRateFactor = 23;

		} videoEncoder;

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
