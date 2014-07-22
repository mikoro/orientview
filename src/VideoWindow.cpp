// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "VideoWindow.h"

using namespace OrientView;

VideoWindow::VideoWindow(QWindow* parent) : OpenGLWindow(parent)
{
	setTitle("OrientView - Video");
	resize(1280, 720);
	setModality(Qt::ApplicationModal);
	setIcon(QIcon(":/MainView/misc/orientview.ico"));
}

VideoWindow::~VideoWindow()
{
}

void VideoWindow::initialize()
{
}

void VideoWindow::render()
{
}
