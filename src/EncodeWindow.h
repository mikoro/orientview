// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QDialog>
#include <QTime>

namespace Ui
{
	class EncodeWindow;
}

namespace OrientView
{
	class EncodeWindow : public QDialog
	{
		Q_OBJECT

	public:

		explicit EncodeWindow(QWidget *parent = 0);
		~EncodeWindow();

		bool initialize();
		void shutdown();

	signals:

		void closing();

	public slots:

		void progressUpdate(int currentFrame, int totalFrames);
		void encodingFinished();

	private slots:
		
		void on_pushButtonStop_clicked();

	private:

		bool event(QEvent* event);

		Ui::EncodeWindow* ui = nullptr;
		QTime startTime;
	};
}
