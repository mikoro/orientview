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

void EncodeWindow::setProgress(int currentFrame, int totalFrames)
{
	int value = (int)round((double)currentFrame / totalFrames * 100.0);
	ui->progressBar->setValue(value);

	int elapsedTime = startTime.elapsed();
	double timePerFrame = (double)elapsedTime / currentFrame;
	int remainingTimeTemp = ((int)round(timePerFrame * totalFrames)) - elapsedTime;

	if (remainingTimeTemp < 0)
		remainingTimeTemp = 0;

	QTime remainingTime(0, 0, 0, 0);
	remainingTime.addMSecs(remainingTimeTemp);

	ui->labelRemaining->setText(QString("Remaining: %1").arg(remainingTime.toString()));
}

bool EncodeWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	return QDialog::event(event);
}
