// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui
{
	class MainWindow;
}

namespace OrientView
{
	class Settings;
	class VideoWindow;
	class EncodeWindow;
	class VideoDecoder;
	class VideoEncoder;
	class QuickRouteReader;
	class MapImageReader;
	class VideoStabilizer;
	class InputHandler;
	class Renderer;
	class VideoDecoderThread;
	class RenderOnScreenThread;
	class RenderOffScreenThread;
	class VideoEncoderThread;

	// Main window is the first window shown and houses all the other parts of the program.
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:

		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

		void readSettingsFromLocal();
		void writeSettingsToLocal();
		void readSettingsFromIniFile(const QString& fileName);
		void writeSettingsToIniFile(const QString& fileName);
		void addLogMessage(const QString& timeString, const QString& typeString, const QString& messageString);

	private slots:

		void on_actionLoadSettings_triggered();
		void on_actionSaveSettings_triggered();
		void on_actionDefaultSettings_triggered();
		void on_actionPlayVideo_triggered();
		void on_actionEncodeVideo_triggered();
		void on_actionExit_triggered();

		void on_pushButtonBrowseInputVideoFile_clicked();
		void on_pushButtonBrowseQuickRouteJpegFile_clicked();
		void on_pushButtonBrowseMapImageFile_clicked();
		void on_pushButtonBrowseOutputVideoFile_clicked();
		void on_pushButtonPickVideoPanelBackgroundColor_clicked();
		void on_pushButtonPickMapPanelBackgroundColor_clicked();

	private:

		void playVideoFinished();
		void encodeVideoFinished();

		Ui::MainWindow* ui = nullptr;
		QStandardItemModel* logDataModel = nullptr;
		Settings* settings = nullptr;
		VideoWindow* videoWindow = nullptr;
		EncodeWindow* encodeWindow = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		VideoEncoder* videoEncoder = nullptr;
		QuickRouteReader* quickRouteReader = nullptr;
		MapImageReader* mapImageReader = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		InputHandler* inputHandler = nullptr;
		Renderer* renderer = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		RenderOnScreenThread* renderOnScreenThread = nullptr;
		RenderOffScreenThread* renderOffScreenThread = nullptr;
		VideoEncoderThread* videoEncoderThread = nullptr;
	};
}
