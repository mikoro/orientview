// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLFramebufferObjectFormat>

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

	surface = new QOffscreenSurface();
	surface->create();

	if (!surface->isValid())
	{
		qWarning("Could not create offscreen surface");
		return false;
	}

	QSurfaceFormat contextFormat;
	contextFormat.setSamples(4);

	context = new QOpenGLContext();
	context->setFormat(contextFormat);

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

	QOpenGLFramebufferObjectFormat fboFormat;
	fboFormat.setSamples(4);
	fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

	framebuffer = new QOpenGLFramebufferObject(1280, 720, fboFormat);

	if (!framebuffer->isValid())
	{
		qWarning("Could not create framebuffer");
		return false;
	}

	ui->progressBar->setValue(0);
	ui->pushButtonStop->setText("Stop");

	startTime.restart();

	return true;
}

void EncodeWindow::shutdown()
{
	qDebug("Shutting down EncodeWindow");

	if (framebuffer != nullptr)
	{
		delete framebuffer;
		framebuffer = nullptr;
	}

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
}

QOffscreenSurface* EncodeWindow::getSurface() const
{
	return surface;
}

QOpenGLContext* EncodeWindow::getContext() const
{
	return context;
}

QOpenGLFramebufferObject* EncodeWindow::getFramebuffer() const
{
	return framebuffer;
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
