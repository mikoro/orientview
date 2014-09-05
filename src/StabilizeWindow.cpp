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
	resize(10, 10);

	totalFrameCount = videoDecoder->getTotalFrameCount();

	QTime totalVideoDuration = QTime(0, 0, 0, 0).addMSecs(videoDecoder->getTotalDuration() * 1000.0);

	ui->progressBarMain->setValue(0);
	ui->labelTotalVideoDuration->setText(totalVideoDuration.toString());
	ui->labelTotalFrames->setText(QString::number(totalFrameCount));

	startTime.start();

	return true;
}

void StabilizeWindow::frameProcessed(int frameNumber, double currentTime)
{
	int value = (int)round((double)frameNumber / totalFrameCount * 1000.0);
	ui->progressBarMain->setValue(value);

	int elapsedTimeMs = startTime.elapsed() - totalPauseTime;
	double timePerFrameMs = (double)elapsedTimeMs / frameNumber;
	double framesPerSecond = 1.0 / timePerFrameMs * 1000.0;
	int totalTimeMs = (int)round(timePerFrameMs * totalFrameCount);
	int remainingTimeMs = totalTimeMs - elapsedTimeMs;

	if (remainingTimeMs < 0)
		remainingTimeMs = 0;

	QTime elapsedTime = QTime(0, 0, 0, 0).addMSecs(elapsedTimeMs);
	QTime remainingTime = QTime(0, 0, 0, 0).addMSecs(remainingTimeMs);
	QTime totalTime = QTime(0, 0, 0, 0).addMSecs(totalTimeMs);
	QTime currentVideoTime = QTime(0, 0, 0, 0).addMSecs(currentTime * 1000.0);

	ui->labelElapsedTime->setText(elapsedTime.toString());
	ui->labelRemainingTime->setText(remainingTime.toString());
	ui->labelTotalAnalysisTime->setText(totalTime.toString());
	ui->labelCurrentVideoTime->setText(currentVideoTime.toString());
	ui->labelCurrentFrame->setText(QString::number(frameNumber));
	ui->labelFramesPerSecond->setText(QString::number(framesPerSecond, 'f', 1));
}

void StabilizeWindow::processingFinished()
{
	ui->pushButtonPauseContinue->setEnabled(false);
	ui->pushButtonStopClose->setText("Close");

	isRunning = false;
}

void StabilizeWindow::on_pushButtonPauseContinue_clicked()
{
	videoStabilizerThread->togglePaused();

	if (videoStabilizerThread->getIsPaused())
	{
		ui->pushButtonPauseContinue->setText("Continue");
		pauseTime.restart();
	}
	else
	{
		ui->pushButtonPauseContinue->setText("Pause");
		totalPauseTime += pauseTime.elapsed();
	}
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
