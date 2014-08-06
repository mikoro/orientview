// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

namespace OrientView
{
	class VideoWindow;
	class Renderer;
	class VideoDecoder;
	class VideoDecoderThread;
	class RenderOnScreenThread;
	class Renderer;
	struct Settings;

	enum class SelectedPanel { NONE, VIDEO, MAP };

	// Read user input and act accordingly.
	class InputHandler
	{

	public:

		bool initialize(VideoWindow* videoWindow, Renderer* renderer, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, RenderOnScreenThread* renderOnScreenThread, Settings* settings);
		void handleInput(double frameTime);

		SelectedPanel getSelectedPanel() const;

	private:

		VideoWindow* videoWindow = nullptr;
		Renderer* renderer = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		RenderOnScreenThread* renderOnScreenThread = nullptr;
		Settings* settings = nullptr;

		SelectedPanel selectedPanel = SelectedPanel::NONE;
	};
}
