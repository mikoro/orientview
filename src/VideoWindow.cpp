// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>

#include "VideoWindow.h"
#include "RenderOnScreenThread.h"
#include "Settings.h"

using namespace OrientView;

VideoWindow::VideoWindow(QWindow* parent) : QWindow(parent)
{
}

bool VideoWindow::initialize(RenderOnScreenThread* renderOnScreenThread, Settings* settings)
{
	qDebug("Initializing VideoWindow");

	this->renderOnScreenThread = renderOnScreenThread;

	setSurfaceType(QWindow::OpenGLSurface);
	setIcon(QIcon(":/MainView/misc/orientview.ico"));
	setTitle("OrientView - Video");
	resize(settings->window.width, settings->window.height);
	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), QApplication::desktop()->availableGeometry()));
	setWindowState(settings->window.fullscreen ? Qt::WindowFullScreen : Qt::WindowNoState);
	setCursor(settings->window.hideCursor ? Qt::BlankCursor : Qt::ArrowCursor);
	
	qDebug("Creating OpenGL context");

	QSurfaceFormat surfaceFormat;
	surfaceFormat.setSamples(settings->window.multisamples);
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

bool VideoWindow::keyIsDown(int key)
{
	if (keyMap.count(key) == 0)
		return false;

	return keyMap[key];
}

bool VideoWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = (QKeyEvent*)event;
		keyMap[ke->key()] = true;

		if (ke->key() == Qt::Key_Escape)
		{
			emit closing();
			this->close();
		}

		if (ke->key() == Qt::Key_F1)
			renderOnScreenThread->toggleRenderInfoPanel();
	}

	if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ke = (QKeyEvent*)event;
		keyMap[ke->key()] = false;
	}

	return QWindow::event(event);
}
