// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

namespace OrientView
{
	class Settings
	{

	public:

		Settings();

		bool initialize(const QString& fileName);
		void shutdown();

		struct Display
		{
			int width = 0;
			int height = 0;
			int multisamples = 0;
			bool fullscreen = false;
			bool hideCursor = false;

		} display;

		struct Shaders
		{
			QString videoPanelShader;
			QString mapPanelShader;

		} shaders;

		struct Appearance
		{
			int mapImageHeaderCrop = 0;
			double mapPanelWidth = 0.0;
			bool showInfoPanel = false;

		} appearance;

		struct Stabilization
		{
			bool enabled = true;
			int imageSizeDivisor = 0;

		} stabilization;

		struct Decoder
		{
			int frameCountDivisor = 0;
			int frameDurationDivisor = 0;

		} decoder;

		struct Encoder
		{
			QString preset;
			QString profile;
			int constantRateFactor = 0;

		} encoder;
	};
}
