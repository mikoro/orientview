// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QWindow>
#include <QOpenGLContext>

namespace OrientView
{
	class Settings;

	class VideoWindow : public QWindow
	{
		Q_OBJECT

	public:

		explicit VideoWindow(QWindow* parent = 0);

		bool initialize(Settings* settings);
		void shutdown();

		QOpenGLContext* getContext() const;
		bool isInitialized() const;
		bool keyIsDown(int key);
		bool keyIsDownOnce(int key);

	signals:

		void closing();

	protected:

		bool event(QEvent* event);

	private:

		QOpenGLContext* context = nullptr;
		bool initialized = false;
		std::map<int, bool> keyMap;
		std::map<int, bool> keyMapOnce;
	};
}
