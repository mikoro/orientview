// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QMainWindow>

namespace Ui
{
	class MainWindow;
}

namespace OrientView
{
	class VideoWindow;
	class EncodeWindow;
	class VideoDecoder;
	class QuickRouteJpegReader;
	class VideoStabilizer;
	class VideoRenderer;
	class VideoDecoderThread;
	class RenderOnScreenThread;
	class RenderOffScreenThread;
	class VideoEncoderThread;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:

		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private slots:

		void on_pushButtonBrowseVideoFile_clicked();
		void on_pushButtonBrowseMapFile_clicked();
		void on_pushButtonBrowseSettingsFile_clicked();
		void on_pushButtonBrowseOutputVideoFile_clicked();
		void on_pushButtonRun_clicked();
		void on_pushButtonEncode_clicked();

		void videoWindowClosing();

	private:

		void readSettings();
		void writeSettings();

		void closeEvent(QCloseEvent* event);

		Ui::MainWindow* ui = nullptr;
		VideoWindow* videoWindow = nullptr;
		EncodeWindow* encodeWindow = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		QuickRouteJpegReader* quickRouteJpegReader = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		VideoRenderer* videoRenderer = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		RenderOnScreenThread* renderOnScreenThread = nullptr;
		RenderOffScreenThread* renderOffScreenThread = nullptr;
		VideoEncoderThread* videoEncoderThread = nullptr;
	};
}
