// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>

#include "VideoWindow.h"
#include "Settings.h"

using namespace OrientView;

VideoWindow::VideoWindow(QWindow* parent) : QWindow(parent)
{
}

bool VideoWindow::initialize(Settings* settings)
{
	qDebug("Initializing VideoWindow");

	setSurfaceType(QWindow::OpenGLSurface);
	setIcon(QIcon(":/MainView/misc/orientview.ico"));
	setTitle("OrientView - Video");
	resize(settings->display.width, settings->display.height);
	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), QApplication::desktop()->availableGeometry()));

	qDebug("Creating OpenGL context");

	QSurfaceFormat surfaceFormat;
	surfaceFormat.setSamples(settings->display.multisamples);
	this->setFormat(surfaceFormat);

	context = new QOpenGLContext();
	context->setFormat(surfaceFormat);

	if (!context->create())
	{
		qWarning("Could not create OpenGL context");
		return false;
	}

	if (!context->makeCurrent(this))
	{
		qWarning("Could not make context current");
		return false;
	}

	isInitialized = true;

	return true;
}

void VideoWindow::shutdown()
{
	qDebug("Shutting down VideoWindow");

	if (context != nullptr)
	{
		delete context;
		context = nullptr;
	}

	isInitialized = false;
}

QOpenGLContext* VideoWindow::getContext() const
{
	return context;
}

bool VideoWindow::getIsInitialized() const
{
	return isInitialized;
}

bool VideoWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	return QWindow::event(event);
}
