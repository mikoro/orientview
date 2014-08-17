// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDesktopServices>

#include "StabilizeWindow.h"
#include "ui_StabilizeWindow.h"
#include "VideoDecoder.h"
#include "VideoStabilizerThread.h"
#include "Settings.h"

using namespace OrientView;

StabilizeWindow::StabilizeWindow(QWidget *parent) : QDialog(parent), ui(new Ui::StabilizeWindow)
{
	ui->setupUi(this);
}

StabilizeWindow::~StabilizeWindow()
{
	if (ui != nullptr)
	{
		delete ui;
		ui = nullptr;
	}
}

bool StabilizeWindow::initialize(VideoDecoder* videoDecoder, VideoStabilizerThread* videoStabilizerThread)
{
	this->videoStabilizerThread = videoStabilizerThread;

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	totalFrameCount = videoDecoder->getTotalFrameCount();

	ui->progressBarMain->setValue(0);
	ui->labelTotalFrames->setText(QString::number(totalFrameCount));
	ui->pushButtonStopClose->setText("Stop");

	startTime.restart();

	return true;
}

void StabilizeWindow::frameProcessed(int frameNumber)
{
	int value = (int)round((double)frameNumber / totalFrameCount * 1000.0);
	ui->progressBarMain->setValue(value);

	int elapsedTimeMs = startTime.elapsed();
	double timePerFrameMs = (double)elapsedTimeMs / frameNumber;
	double framesPerSecond = 1.0 / timePerFrameMs * 1000.0;
	int totalTimeMs = (int)round(timePerFrameMs * totalFrameCount);
	int remainingTimeMs = totalTimeMs - elapsedTimeMs;

	if (remainingTimeMs < 0)
		remainingTimeMs = 0;

	QTime elapsedTime = QTime(0, 0, 0, 0).addMSecs(elapsedTimeMs);
	QTime remainingTime = QTime(0, 0, 0, 0).addMSecs(remainingTimeMs);
	QTime totalTime = QTime(0, 0, 0, 0).addMSecs(totalTimeMs);

	ui->labelElapsedTime->setText(elapsedTime.toString());
	ui->labelRemainingTime->setText(remainingTime.toString());
	ui->labelTotalTime->setText(totalTime.toString());
	ui->labelCurrentFrame->setText(QString::number(frameNumber));
	ui->labelFramesPerSecond->setText(QString::number(framesPerSecond, 'f', 1));
}

void StabilizeWindow::processingFinished()
{
	ui->pushButtonStopClose->setText("Close");
	isRunning = false;
}

void StabilizeWindow::on_pushButtonStopClose_clicked()
{
	if (isRunning)
	{
		videoStabilizerThread->requestInterruption();
		videoStabilizerThread->wait();
	}
	else
		close();
}

bool StabilizeWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	return QDialog::event(event);
}
