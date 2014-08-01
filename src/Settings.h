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
			QString videoFilePath;
			QString mapFilePath;
			QString gpxFilePath;
			QString outputFilePath;

		} files;

		struct Window
		{
			int width = 0;
			int height = 0;
			int multisamples = 0;
			bool fullscreen = false;
			bool hideCursor = false;

		} window;

		struct MapCalibration
		{
			double topLeftLat = 0;
			double topLeftLong = 0;
			double bottomRightLat = 0;
			double bottomRightLong = 0;

		} mapCalibration;

		struct VideoCalibration
		{
			double startOffset = 0;

		} videoCalibration;

		struct Appearance
		{
			bool showInfoPanel = false;
			double mapPanelWidth = 0.0;
			int mapHeaderCrop = 0;

		} appearance;

		struct Decoder
		{
			int frameCountDivisor = 0;
			int frameDurationDivisor = 0;

		} decoder;

		struct Stabilization
		{
			bool enabled = false;
			int imageSizeDivisor = 0;

		} stabilization;

		struct Shaders
		{
			QString videoPanelShader;
			QString mapPanelShader;

		} shaders;

		struct Encoder
		{
			QString preset;
			QString profile;
			int constantRateFactor = 0;

		} encoder;
	};
}
