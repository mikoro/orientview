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
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:

		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private slots:

		void on_pushButtonOpen_clicked();
		void on_pushButtonClose_clicked();
		void on_pushButtonGet_clicked();
		void on_pushButtonPlay_clicked();
		void on_pushButtonStop_clicked();
		void on_playTimer_update();

	private:

		Ui::MainWindow *ui;
		QTimer* playTimer;
		FFmpegDecoder decoder;
	};
}
