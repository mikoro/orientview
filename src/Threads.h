// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

namespace OrientView
{
	class VideoWindow;
	class VideoRenderer;
	class FFmpegDecoder;

	class DecodeThread : public QThread
	{
		Q_OBJECT

	public:

		DecodeThread();

		void run();
	};

	class RenderOnScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOnScreenThread();

		void initialize(VideoWindow* videoWindow, VideoRenderer* videoRenderer, FFmpegDecoder* ffmpegDecoder);
		void run();

	private:

		VideoWindow* videoWindow = nullptr;
		VideoRenderer* videoRenderer = nullptr;
		FFmpegDecoder* ffmpegDecoder = nullptr;
	};

	class RenderOffScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOffScreenThread();

		void run();
	};

	class EncodeThread : public QThread
	{
		Q_OBJECT

	public:

		EncodeThread();

		void run();
	};
}
