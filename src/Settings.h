// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QSettings>

namespace Ui
{
	class MainWindow;
}

namespace OrientView
{
	class Settings
	{

	public:

		void read(QSettings* settings);
		void write(QSettings* settings);
		void update(Ui::MainWindow* ui);
		void apply(Ui::MainWindow* ui);

		struct Files
		{
			QString videoFilePath = "";
			QString mapFilePath = "";
			QString gpxFilePath = "";
			QString outputFilePath = "";

		} files;

		struct Window
		{
			int width = 1280;
			int height = 720;
			int multisamples = 16;
			bool fullscreen = false;
			bool hideCursor = false;

		} window;

		struct MapCalibration
		{
			double topLeftLat = 0.0;
			double topLeftLon = 0.0;
			double topRightLat = 0.0;
			double topRightLon = 0.0;
			double bottomRightLat = 0.0;
			double bottomRightLon = 0.0;
			double bottomLeftLat = 0.0;
			double bottomLeftLon = 0.0;
			double projectionOriginLat = 0.0;
			double projectionOriginLon = 0.0;

		} mapCalibration;

		struct Timing
		{
			QString splitTimes = "";

		} timing;

		struct Appearance
		{
			bool showInfoPanel = false;
			double mapPanelWidth = 0.3;
			double videoPanelScale = 1.0;
			int mapHeaderCrop = 0;

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
			double dampingFactor = 1.0;

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
	};
}
