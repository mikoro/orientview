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
	class QuickRouteReader;
	class MapImageReader;
	class VideoStabilizer;
	class InputHandler;
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

		void on_pushButtonBrowseInputVideoFile_clicked();
		void on_pushButtonBrowseQuickRouteJpegMapImageFile_clicked();
		void on_pushButtonBrowseAlternativeMapImageFile_clicked();
		void on_pushButtonBrowseOutputVideoFile_clicked();
		void on_pushButtonPickVideoPanelBackgroundColor_clicked();
		void on_pushButtonPickMapPanelBackgroundColor_clicked();

		void videoWindowClosing();
		void encodeWindowClosing();

	private:

		void readSettings();
		void writeSettings();

		void closeEvent(QCloseEvent* event);

		Ui::MainWindow* ui = nullptr;
		Settings* settings = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		QuickRouteReader* quickRouteReader = nullptr;
		MapImageReader* mapImageReader = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		InputHandler* inputHandler = nullptr;
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
