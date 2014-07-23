// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>

#include "VideoWindow.h"

using namespace OrientView;

VideoWindow::VideoWindow(QWindow* parent) : QWindow(parent)
{
}

VideoWindow::~VideoWindow()
{
	shutdown();
}

bool VideoWindow::initialize()
{
	qDebug("Initializing VideoWindow");

	setSurfaceType(QWindow::OpenGLSurface);
	setIcon(QIcon(":/MainView/misc/orientview.ico"));
	setTitle("OrientView - Video");
	resize(1280, 720);
	setModality(Qt::ApplicationModal);
	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), QApplication::desktop()->availableGeometry()));

	qDebug("Creating OpenGL context");

	context = std::unique_ptr<QOpenGLContext>(new QOpenGLContext());
	context->setFormat(format());

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

	return true;
}

void VideoWindow::shutdown()
{
	qDebug("Shutting down VideoWindow");

	if (context != nullptr)
		context.reset(nullptr);
}

QOpenGLContext* VideoWindow::getContext() const
{
	return context.get();
}

bool VideoWindow::event(QEvent* event)
{
	if (event->type() == QEvent::Close)
		emit closing();

	return QWindow::event(event);
}
