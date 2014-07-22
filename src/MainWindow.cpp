// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QTimer>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "FFmpegDecoder.h"

using namespace OrientView;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	playTimer = new QTimer(this);
	connect(playTimer, SIGNAL(timeout()), this, SLOT(on_playTimer_update()));
}

MainWindow::~MainWindow()
{
	delete playTimer;
	delete ui;
}

void MainWindow::on_pushButtonOpen_clicked()
{
	decoder.Open("testvideo.mp4");
}

void MainWindow::on_pushButtonClose_clicked()
{
	decoder.Close();
}

void MainWindow::on_pushButtonGet_clicked()
{
	DecodedPicture* picture = decoder.GetNextPicture();

	if (picture != nullptr)
	{
		ui->labelPicture->setPixmap(QPixmap::fromImage(QImage(picture->data, picture->width, picture->height, picture->stride, QImage::Format_RGB888)));
	}
}

void MainWindow::on_pushButtonPlay_clicked()
{
	playTimer->start((int)round(decoder.GetFrameTime()));
}

void MainWindow::on_pushButtonStop_clicked()
{
	playTimer->stop();
}

void MainWindow::on_playTimer_update()
{
	on_pushButtonGet_clicked();
}