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

private:

	Ui::MainView *ui;
	OrientView::FFmpegDecoder decoder;
};
