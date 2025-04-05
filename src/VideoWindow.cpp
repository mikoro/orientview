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
	qDebug("Initializing video window");

	setSurfaceType(QWindow::OpenGLSurface);
	setIcon(QIcon(":/icons/icons/orientview.ico"));
	setTitle("OrientView - Video");
	resize(settings->window.width, settings->window.height);
	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), QApplication::desktop()->availableGeometry()));
	setWindowState(settings->window.fullscreen ? Qt::WindowFullScreen : Qt::WindowNoState);
	setCursor(settings->window.hideCursor ? Qt::BlankCursor : Qt::ArrowCursor);

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

	isInitialized = true;

	return true;
}

VideoWindow::~VideoWindow()
{
	if (context != nullptr)
	{
		delete context;
		context = nullptr;
	}
}

QOpenGLContext* VideoWindow::getContext() const
{
	return context;
}

bool VideoWindow::getIsInitialized() const
{
	return isInitialized;
}

bool VideoWindow::keyIsDown(int key)
{
	if (keyMap.count(key) == 0)
		return false;

	return keyMap[key];
}

bool VideoWindow::keyIsDownOnce(int key)
{
	if (keyMap.count(key) == 0 || keyMapOnce[key])
		return false;

	if (keyMap[key])
	{
		keyMapOnce[key] = true;
		return true;
	}

	return false;
}

bool VideoWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	if (event->type() == QEvent::Resize)
	{
		QResizeEvent* re = (QResizeEvent*)event;
		emit resizing(re->size().width(), re->size().height());
	}

	if (event->type() == QEvent::FocusIn)
	{
		emit resizing(width(), height());
	}

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = (QKeyEvent*)event;

		if (!ke->isAutoRepeat())
		{
			keyMap[ke->key()] = true;

			if (ke->key() == Qt::Key_Escape)
			{
				emit closing();
				this->close();
			}
		}
	}

	if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ke = (QKeyEvent*)event;

		if (!ke->isAutoRepeat())
		{
			keyMap[ke->key()] = false;
			keyMapOnce[ke->key()] = false;
		}
	}

	return QWindow::event(event);
}
