// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QTimer>

#include "MainView.h"
#include "ui_MainView.h"
#include "FFmpegDecoder.h"

using namespace OrientView;

MainView::MainView(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainView)
{
	ui->setupUi(this);
	playTimer = new QTimer(this);
	connect(playTimer, SIGNAL(timeout()), this, SLOT(on_playTimer_update()));
}

MainView::~MainView()
{
	delete playTimer;
	delete ui;
}

void MainView::on_pushButtonOpen_clicked()
{
	decoder.Open("testvideo.mp4");
}

void MainView::on_pushButtonClose_clicked()
{
	decoder.Close();
}

void MainView::on_pushButtonGet_clicked()
{
	DecodedPicture* picture = decoder.GetNextPicture();

	if (picture != nullptr)
	{
		ui->labelPicture->setPixmap(QPixmap::fromImage(QImage(picture->data, picture->width, picture->height, picture->stride, QImage::Format_RGB888)));
	}
}

void MainView::on_pushButtonPlay_clicked()
{
	playTimer->start((int)round(decoder.GetFrameTime()));
}

void MainView::on_pushButtonStop_clicked()
{
	playTimer->stop();
}

void MainView::on_playTimer_update()
{
	on_pushButtonGet_clicked();
}