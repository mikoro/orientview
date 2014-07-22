// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include "OpenGLWindow.h"

#include <QOpenGLShaderProgram>

namespace OrientView
{
	class VideoWindow : public OpenGLWindow
	{

	public:

		VideoWindow();
		~VideoWindow();

		void initialize();
		void render();

	private:

	};
}
