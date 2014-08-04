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
	delete ui;
}

bool EncodeWindow::initialize(VideoDecoder* videoDecoder, VideoEncoderThread* videoEncoderThread, Settings* settings)
{
	qDebug("Initializing EncodeWindow");

	this->videoEncoderThread = videoEncoderThread;

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QSurfaceFormat surfaceFormat;
	surfaceFormat.setSamples(settings->window.multisamples);

	qDebug("Creating offscreen surface");

	surface = new QOffscreenSurface();
	surface->setFormat(surfaceFormat);
	surface->create();

	if (!surface->isValid())
	{
		qWarning("Could not create offscreen surface");
		return false;
	}

	qDebug("Creating OpenGL context");

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

	ui->progressBarMain->setValue(0);
	ui->pushButtonOpenVideo->setEnabled(false);
	ui->pushButtonStopClose->setText("Stop");

	startTime.restart();

	initialized = true;
	isRunning = true;
	totalFrameCount = videoDecoder->getTotalFrameCount();
	currentSize = 0.0;
	videoFilePath = settings->files.outputVideoFilePath;

	return true;
}

void EncodeWindow::shutdown()
{
	qDebug("Shutting down EncodeWindow");

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

	initialized = false;
}

QOffscreenSurface* EncodeWindow::getSurface() const
{
	return surface;
}

QOpenGLContext* EncodeWindow::getContext() const
{
	return context;
}

bool EncodeWindow::isInitialized() const
{
	return initialized;
}

void EncodeWindow::frameProcessed(int frameNumber, int frameSize)
{
	int value = (int)round((double)frameNumber / totalFrameCount * 1000.0);
	ui->progressBarMain->setValue(value);

	int elapsedTime1 = startTime.elapsed();

	QTime elapsedTime2(0, 0, 0, 0);
	QTime elapsedTime3 = elapsedTime2.addMSecs(elapsedTime1);

	double timePerFrame = (double)elapsedTime1 / frameNumber;
	int remainingTime1 = ((int)round(timePerFrame * totalFrameCount)) - elapsedTime1;

	if (remainingTime1 < 0)
		remainingTime1 = 0;

	QTime remainingTime2(0, 0, 0, 0);
	QTime remainingTime3 = remainingTime2.addMSecs(remainingTime1);

	currentSize += frameSize / 1000000.0;

	ui->labelElapsed->setText(elapsedTime3.toString());
	ui->labelRemaining->setText(remainingTime3.toString());
	ui->labelFrame->setText(QString("%1/%2").arg(QString::number(frameNumber), QString::number(totalFrameCount)));
	ui->labelSize->setText(QString("%1 MB").arg(QString::number(currentSize, 'f', 2)));
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
