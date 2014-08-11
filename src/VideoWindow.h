// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QWindow>
#include <QOpenGLContext>

namespace OrientView
{
	struct Settings;

	// Display the live video in a window.
	class VideoWindow : public QWindow
	{
		Q_OBJECT

	public:

		explicit VideoWindow(QWindow* parent = 0);
		~VideoWindow();

		bool initialize(Settings* settings);

		QOpenGLContext* getContext() const;
		bool getIsInitialized() const;
		bool keyIsDown(int key);
		bool keyIsDownOnce(int key);

	signals:

		void closing();
		void resizing(int newWidth, int newHeight);

	protected:

		bool event(QEvent* event);

	private:

		QOpenGLContext* context = nullptr;

		bool isInitialized = false;

		std::map<int, bool> keyMap;
		std::map<int, bool> keyMapOnce;
	};
}
