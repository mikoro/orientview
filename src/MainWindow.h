// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QMainWindow>

#include "FFmpegDecoder.h"

namespace Ui
{
	class MainWindow;
}

namespace OrientView
{
	class VideoWindow;

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

	private:

		void readSettings();
		void writeSettings();

		void closeEvent(QCloseEvent* event);

		Ui::MainWindow* ui = nullptr;
		VideoWindow* videoWindow = nullptr;
		FFmpegDecoder decoder;
	};
}
