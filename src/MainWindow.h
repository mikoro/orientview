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
	class StabilizeWindow;
	class VideoDecoder;
	class VideoEncoder;
	class QuickRouteReader;
	class MapImageReader;
	class VideoStabilizer;
	class InputHandler;
	class SplitsManager;
	class RouteManager;
	class Renderer;
	class VideoDecoderThread;
	class RenderOnScreenThread;
	class RenderOffScreenThread;
	class VideoEncoderThread;
	class VideoStabilizerThread;

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
		void on_actionHelp_triggered();
		void on_actionExit_triggered();

		void on_pushButtonBrowseMapImageFile_clicked();
		void on_pushButtonBrowseQuickRouteJpegFile_clicked();
		void on_pushButtonBrowseInputVideoFile_clicked();
		void on_pushButtonBrowseOutputVideoFile_clicked();

		void on_pushButtonPickMapBackgroundColor_clicked();
		void on_pushButtonPickRouteDiscreetColor_clicked();
		void on_pushButtonPickRouteHighlightColor_clicked();
		void on_pushButtonPickRouteRunnerColor_clicked();
		void on_pushButtonPickRouteRunnerBorderColor_clicked();
		void on_pushButtonPickRouteControlBorderColor_clicked();
		void on_pushButtonPickVideoBackgroundColor_clicked();

		void on_pushButtonVideoStabilizerBrowseInputDataFile_clicked();
		void on_pushButtonVideoStabilizerBrowsePassOneOutputFile_clicked();
		void on_pushButtonVideoStabilizerBrowsePassTwoInputFile_clicked();
		void on_pushButtonVideoStabilizerBrowsePassTwoOutputFile_clicked();
		void on_pushButtonVideoStabilizerPassOneRun_clicked();
		void on_pushButtonVideoStabilizerPassTwoRun_clicked();

	private:

		void playVideoFinished();
		void encodeVideoFinished();
		void stabilizeVideoFinished();

		Ui::MainWindow* ui = nullptr;
		QStandardItemModel* logDataModel = nullptr;
		Settings* settings = nullptr;
		VideoWindow* videoWindow = nullptr;
		EncodeWindow* encodeWindow = nullptr;
		StabilizeWindow* stabilizeWindow = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		VideoEncoder* videoEncoder = nullptr;
		QuickRouteReader* quickRouteReader = nullptr;
		MapImageReader* mapImageReader = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		InputHandler* inputHandler = nullptr;
		SplitsManager* splitsManager = nullptr;
		RouteManager* routeManager = nullptr;
		Renderer* renderer = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		RenderOnScreenThread* renderOnScreenThread = nullptr;
		RenderOffScreenThread* renderOffScreenThread = nullptr;
		VideoEncoderThread* videoEncoderThread = nullptr;
		VideoStabilizerThread* videoStabilizerThread = nullptr;
	};
}
