// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "OpenGLWindow.h"

using namespace OrientView;

OpenGLWindow::OpenGLWindow(QWindow* parent) : QWindow(parent)
{
	setSurfaceType(QWindow::OpenGLSurface);
}

OpenGLWindow::~OpenGLWindow()
{
}

void OpenGLWindow::renderNow()
{
	bool needsInitialize = false;

	if (!context)
	{
		context = new QOpenGLContext(this);
		context->setFormat(requestedFormat());
		context->create();

		needsInitialize = true;
	}

	context->makeCurrent(this);

	if (needsInitialize)
	{
		initializeOpenGLFunctions();
		initialize();
	}

	render();

	context->swapBuffers(this);
}
