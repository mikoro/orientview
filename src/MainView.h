// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QMainWindow>

#include "FFmpegDecoder.h"

namespace Ui
{
	class MainView;
}

class MainView : public QMainWindow
{
	Q_OBJECT

public:

	explicit MainView(QWidget *parent = 0);
	~MainView();

private slots:

	void on_pushButtonOpen_clicked();
	void on_pushButtonClose_clicked();
	void on_pushButtonGet_clicked();
	void on_pushButtonPlay_clicked();
	void on_pushButtonStop_clicked();
	void on_playTimer_update();

private:

	Ui::MainView *ui;
	QTimer* playTimer;
	OrientView::FFmpegDecoder decoder;
};
