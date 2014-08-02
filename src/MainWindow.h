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
	class Settings;
	class VideoDecoder;
	class MapImageReader;
	class GpxReader;
	class VideoStabilizer;
	class Renderer;
	class VideoEncoder;
	class VideoDecoderThread;
	class RenderOnScreenThread;
	class RenderOffScreenThread;
	class VideoEncoderThread;
	class VideoWindow;
	class EncodeWindow;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:

		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private slots:

		void on_actionLoadSettings_triggered();
		void on_actionSaveSettings_triggered();
		void on_actionDefaultSettings_triggered();
		void on_actionPlayVideo_triggered();
		void on_actionEncodeVideo_triggered();
		void on_actionExit_triggered();

		void on_pushButtonBrowseVideoFile_clicked();
		void on_pushButtonBrowseMapFile_clicked();
		void on_pushButtonBrowseGpxFile_clicked();
		void on_pushButtonBrowseOutputFile_clicked();
		void on_pushButtonLoadMapCalibrationData_clicked();

		void videoWindowClosing();
		void encodeWindowClosing();

	private:

		void readSettings();
		void writeSettings();

		void closeEvent(QCloseEvent* event);

		Ui::MainWindow* ui = nullptr;
		Settings* settings = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		MapImageReader* mapImageReader = nullptr;
		GpxReader* gpxReader = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		Renderer* renderer = nullptr;
		VideoEncoder* videoEncoder = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		RenderOnScreenThread* renderOnScreenThread = nullptr;
		RenderOffScreenThread* renderOffScreenThread = nullptr;
		VideoEncoderThread* videoEncoderThread = nullptr;
		VideoWindow* videoWindow = nullptr;
		EncodeWindow* encodeWindow = nullptr;
	};
}
