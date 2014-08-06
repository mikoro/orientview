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
	struct Settings
	{
		void read(QSettings* settings);
		void write(QSettings* settings);
		void update(Ui::MainWindow* ui);
		void apply(Ui::MainWindow* ui);

		struct Files
		{
			QString inputVideoFilePath = "";
			QString quickRouteJpegMapImageFilePath = "";
			QString alternativeMapImageFilePath = "";
			QString outputVideoFilePath = "";

		} files;

		struct Window
		{
			int width = 1280;
			int height = 720;
			int multisamples = 16;
			bool fullscreen = false;
			bool hideCursor = false;

		} window;

		struct Timing
		{
			QString splitTimes = "";

		} timing;

		struct Appearance
		{
			bool showInfoPanel = false;
			double mapPanelWidth = 0.3;
			double videoPanelScale = 1.0;
			double mapPanelScale = 1.0;
			QColor videoPanelBackgroundColor = QColor("#003200");
			QColor mapPanelBackgroundColor = QColor("#ffffff");

		} appearance;

		struct Decoder
		{
			int frameCountDivisor = 1;
			int frameDurationDivisor = 1;
			int frameSizeDivisor = 1;

		} decoder;

		struct Stabilizer
		{
			bool enabled = true;
			int frameSizeDivisor = 8;
			double averagingFactor = 0.1;
			double dampingFactor = 0.5;
			bool disableVideoClear = false;
			int inpaintBorderWidth = 0;

		} stabilizer;

		struct Shaders
		{
			QString videoPanelShader = "default";
			QString mapPanelShader = "default";

		} shaders;

		struct Encoder
		{
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
