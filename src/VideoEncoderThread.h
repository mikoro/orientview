// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

namespace OrientView
{
	class VideoEncoderThread : public QThread
	{
		Q_OBJECT

	public:

		VideoEncoderThread();

		bool initialize();
		void shutdown();

	protected:

		void run();

	private:

	};
}
