// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDesktopServices>

#include "EncodeWindow.h"
#include "ui_EncodeWindow.h"
#include "VideoDecoder.h"
#include "VideoEncoderThread.h"
#include "Settings.h"

using namespace OrientView;

EncodeWindow::EncodeWindow(QWidget *parent) : QDialog(parent), ui(new Ui::EncodeWindow)
{
	ui->setupUi(this);
}

EncodeWindow::~EncodeWindow()
{
	if (context != nullptr)
	{
		delete context;
		context = nullptr;
	}

	if (surface != nullptr)
	{
		surface->destroy();
		delete surface;
		surface = nullptr;
	}

	if (ui != nullptr)
	{
		delete ui;
		ui = nullptr;
	}
}

bool EncodeWindow::initialize(VideoDecoder* videoDecoder, VideoEncoderThread* videoEncoderThread, Settings* settings)
{
	qDebug("Initializing encode window");

	this->videoEncoderThread = videoEncoderThread;

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QSurfaceFormat surfaceFormat;
	surfaceFormat.setSamples(settings->window.multisamples);

	surface = new QOffscreenSurface();
	surface->setFormat(surfaceFormat);
	surface->create();

	if (!surface->isValid())
	{
		qWarning("Could not create offscreen surface");
		return false;
	}

	context = new QOpenGLContext();
	context->setFormat(surfaceFormat);

	if (!context->create())
	{
		qWarning("Could not create OpenGL context");
		return false;
	}

	if (!context->makeCurrent(surface))
	{
		qWarning("Could not make context current");
		return false;
	}

	totalFrameCount = videoDecoder->getTotalFrameCount();
	videoFilePath = settings->videoEncoder.outputVideoFilePath;

	ui->progressBarMain->setValue(0);
	ui->labelTotalFrames->setText(QString::number(totalFrameCount));
	ui->pushButtonOpenVideo->setEnabled(false);
	ui->pushButtonStopClose->setText("Stop");

	startTime.restart();

	isInitialized = true;

	return true;
}

QOffscreenSurface* EncodeWindow::getSurface() const
{
	return surface;
}

QOpenGLContext* EncodeWindow::getContext() const
{
	return context;
}

bool EncodeWindow::getIsInitialized() const
{
	return isInitialized;
}

void EncodeWindow::frameProcessed(int frameNumber, int frameSize)
{
	int value = (int)round((double)frameNumber / totalFrameCount * 1000.0);
	ui->progressBarMain->setValue(value);

	int elapsedTimeMs = startTime.elapsed();
	double timePerFrameMs = (double)elapsedTimeMs / frameNumber;
	double framesPerSecond = 1.0 / timePerFrameMs * 1000.0;
	int totalTimeMs = (int)round(timePerFrameMs * totalFrameCount);
	int remainingTimeMs = totalTimeMs - elapsedTimeMs;
	currentSize += (frameSize / 1000000.0);
	double totalSize = (currentSize / frameNumber) * totalFrameCount;

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
	ui->labelCurrentSize->setText(QString("%1 MB").arg(QString::number(currentSize, 'f', 2)));
	ui->labelTotalSize->setText(QString("%1 MB").arg(QString::number(totalSize, 'f', 2)));
}

void EncodeWindow::encodingFinished()
{
	ui->pushButtonOpenVideo->setEnabled(true);
	ui->pushButtonStopClose->setText("Close");

	isRunning = false;
}

void EncodeWindow::on_pushButtonOpenVideo_clicked()
{
	if (QFile::exists(videoFilePath))
	{
		QFileInfo fileInfo(videoFilePath);
		QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(fileInfo.absoluteFilePath())));
	}
}

void EncodeWindow::on_pushButtonStopClose_clicked()
{
	if (isRunning)
	{
		videoEncoderThread->requestInterruption();
		videoEncoderThread->wait();
	}
	else
		close();
}

bool EncodeWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	return QDialog::event(event);
}
