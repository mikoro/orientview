// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QDialog>
#include <QTime>

namespace Ui
{
	class StabilizeWindow;
}

namespace OrientView
{
	class VideoDecoder;
	class VideoStabilizerThread;
	class Settings;

	class StabilizeWindow : public QDialog
	{
		Q_OBJECT

	public:

		explicit StabilizeWindow(QWidget *parent = 0);
		~StabilizeWindow();

		bool initialize(VideoDecoder* videoDecoder, VideoStabilizerThread* videoStabilizerThread);

	signals:

		void closing();

	public slots:

		void frameProcessed(int frameNumber);
		void processingFinished();

	private slots:
		
		void on_pushButtonStopClose_clicked();

	private:

		bool event(QEvent* event);

		Ui::StabilizeWindow* ui = nullptr;
		VideoStabilizerThread* videoStabilizerThread = nullptr;
		QTime startTime;

		bool isRunning = true;
		int totalFrameCount = 0;
	};
}
