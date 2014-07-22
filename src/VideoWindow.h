// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <memory>

#include <QWindow>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

namespace OrientView
{
	class VideoWindow : public QWindow, protected QOpenGLFunctions
	{
		Q_OBJECT

	public:

		explicit VideoWindow(QWindow* parent = 0);

		bool initialize();
		void start();

		void renderOnScreen();
		void renderOffScreen();

	private slots:

		void timerUpdate();

	private:

		std::unique_ptr<QOpenGLContext> context = nullptr;
		std::unique_ptr<QOpenGLShaderProgram> shaderProgram = nullptr;

		GLuint positionAttribute = 0;
		GLuint colorAttribute = 0;
		GLuint matrixUniform = 0;
	};
}
