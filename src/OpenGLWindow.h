// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QWindow>
#include <QOpenGLFunctions>

namespace OrientView
{
	class OpenGLWindow : public QWindow, protected QOpenGLFunctions
	{
		Q_OBJECT

	public:

		explicit OpenGLWindow(QWindow* parent = 0);
		virtual ~OpenGLWindow();

		virtual void render() = 0;
		virtual void initialize() = 0;

	public slots:

		void renderNow();

	private:
		
		QOpenGLContext* context = nullptr;
	};
}
