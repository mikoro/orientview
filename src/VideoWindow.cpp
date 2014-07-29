// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>

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
	setWindowState(settings->display.fullscreen ? Qt::WindowFullScreen : Qt::WindowNoState);
	setCursor(settings->display.hideCursor ? Qt::BlankCursor : Qt::ArrowCursor);
	
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

	initialized = true;

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

	initialized = false;
}

QOpenGLContext* VideoWindow::getContext() const
{
	return context;
}

bool VideoWindow::isInitialized() const
{
	return initialized;
}

bool VideoWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = (QKeyEvent*)event;

		if (ke->key() == Qt::Key_Escape)
		{
			emit closing();
			this->close();
		}
	}

	return QWindow::event(event);
}
