// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

namespace OrientView
{
	class VideoDecoderThread : public QThread
	{
		Q_OBJECT

	public:

		VideoDecoderThread();

		void initialize();

	protected:

		void run();

	private:

	};
}
