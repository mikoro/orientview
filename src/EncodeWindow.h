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
	class Settings;

	// Display the progress of the encoding process.
	class EncodeWindow : public QDialog
	{
		Q_OBJECT

	public:

		explicit EncodeWindow(QWidget *parent = 0);
		~EncodeWindow();

		bool initialize(VideoDecoder* videoDecoder, VideoEncoderThread* videoEncoderThread, Settings* settings);

		QOffscreenSurface* getSurface() const;
		QOpenGLContext* getContext() const;
		bool getIsInitialized() const;

	signals:

		void closing();

	public slots:

		void frameProcessed(int frameNumber, int frameSize, double currentTime);
		void encodingFinished();

	private slots:

		void on_pushButtonPauseContinue_clicked();
		void on_pushButtonStopClose_clicked();
		void on_pushButtonOpen_clicked();
		
	private:

		bool event(QEvent* event);

		Ui::EncodeWindow* ui = nullptr;
		VideoEncoderThread* videoEncoderThread = nullptr;

		QTime startTime;
		QTime pauseTime;
		int totalPauseTime = 0;

		QOffscreenSurface* surface = nullptr;
		QOpenGLContext* context = nullptr;

		bool isInitialized = false;
		bool isRunning = true;
		int totalFrameCount = 0;
		double currentSize = 0.0;
		QString videoFilePath;
	};
}
