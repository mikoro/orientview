// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "EncodeWindow.h"
#include "ui_EncodeWindow.h"

using namespace OrientView;

EncodeWindow::EncodeWindow(QWidget *parent) : QDialog(parent), ui(new Ui::EncodeWindow)
{
	ui->setupUi(this);
}

EncodeWindow::~EncodeWindow()
{
	delete ui;
}

bool EncodeWindow::initialize()
{
	qDebug("Initializing EncodeWindow");

	ui->progressBar->setValue(0);
	startTime.restart();

	return true;
}

void EncodeWindow::shutdown()
{
	qDebug("Shutting down EncodeWindow");
}

void EncodeWindow::progressUpdate(int currentFrame, int totalFrames)
{
	int value = (int)round((double)currentFrame / totalFrames * 1000.0);
	ui->progressBar->setValue(value);

	int elapsedTime = startTime.elapsed();
	double timePerFrame = (double)elapsedTime / currentFrame;
	int remainingTime1 = ((int)round(timePerFrame * totalFrames)) - elapsedTime;

	if (remainingTime1 < 0)
		remainingTime1 = 0;

	QTime remainingTime2(0, 0, 0, 0);
	QTime remainingTime3 = remainingTime2.addMSecs(remainingTime1);

	ui->labelRemaining->setText(QString("Remaining: %1").arg(remainingTime3.toString()));
}

void EncodeWindow::encodingFinished()
{
	QTime elapsedTime1(0, 0, 0, 0);
	QTime elapsedTime2 = elapsedTime1.addMSecs(startTime.elapsed());

	ui->pushButtonStop->setText("Close");
	ui->labelRemaining->setText(QString("Ready! (%1)").arg(elapsedTime2.toString()));
}

void EncodeWindow::on_pushButtonStop_clicked()
{
	close();
}

bool EncodeWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	return QDialog::event(event);
}
