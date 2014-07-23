// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <memory>

#include <QMainWindow>

#include "ui_MainWindow.h"
#include "FFmpegDecoder.h"
#include "VideoRenderer.h"
#include "VideoWindow.h"
#include "Threads.h"

namespace OrientView
{
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

		std::unique_ptr<Ui::MainWindow> ui = nullptr;
		FFmpegDecoder ffmpegDecoder;
		VideoRenderer videoRenderer;
		VideoWindow videoWindow;
		RenderOnScreenThread renderOnScreenThread;
	};
}
