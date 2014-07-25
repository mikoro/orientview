// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QDialog>

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

	signals:

	private:

		void closeEvent(QCloseEvent* event);

		Ui::EncodeWindow* ui = nullptr;
	};
}
