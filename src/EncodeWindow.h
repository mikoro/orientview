// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QDialog>
#include <QTime>
#include <QOffscreenSurface>
#include <QOpenGLContext>

namespace Ui
{
	class EncodeWindow;
}

namespace OrientView
{
	class VideoDecoder;
	class VideoEncoderThread;
	struct Settings;

	class EncodeWindow : public QDialog
	{
		Q_OBJECT

	public:

		explicit EncodeWindow(QWidget *parent = 0);
		~EncodeWindow();

		bool initialize(VideoDecoder* videoDecoder, VideoEncoderThread* videoEncoderThread, Settings* settings);
		void shutdown();

		QOffscreenSurface* getSurface() const;
		QOpenGLContext* getContext() const;
		bool isInitialized() const;

	signals:

		void closing();

	public slots:

		void frameProcessed(int frameNumber, int frameSize);
		void encodingFinished();

	private slots:
		
		void on_pushButtonOpenVideo_clicked();
		void on_pushButtonStopClose_clicked();

	private:

		bool event(QEvent* event);

		VideoEncoderThread* videoEncoderThread;

		Ui::EncodeWindow* ui = nullptr;
		QTime startTime;

		QOffscreenSurface* surface = nullptr;
		QOpenGLContext* context = nullptr;

		bool initialized = false;
		bool isRunning = false;
		int totalFrameCount = 0;
		double currentSize = 0.0;
		QString videoFilePath;
	};
}
