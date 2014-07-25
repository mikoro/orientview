// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "RenderOffScreenThread.h"

using namespace OrientView;

RenderOffScreenThread::RenderOffScreenThread()
{
}

bool RenderOffScreenThread::initialize()
{
	qDebug("Initializing RenderOffScreenThread");

	return true;
}

void RenderOffScreenThread::shutdown()
{
	qDebug("Shutting down RenderOffScreenThread");
}

void RenderOffScreenThread::run()
{
}
