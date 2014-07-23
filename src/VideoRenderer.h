// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <memory>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

namespace OrientView
{
	class VideoRenderer : protected QOpenGLFunctions
	{

	public:

		VideoRenderer();
		~VideoRenderer();

		bool initialize();
		void shutdown();

		void render();

	private:

		std::unique_ptr<QOpenGLShaderProgram> shaderProgram = nullptr;

		GLuint positionAttribute = 0;
		GLuint colorAttribute = 0;
		GLuint matrixUniform = 0;
	};
}
