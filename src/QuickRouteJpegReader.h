// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QImage>

namespace OrientView
{
	class QuickRouteJpegReader
	{

	public:

		QuickRouteJpegReader();

		bool initialize(const QString& fileName);
		void shutdown();

		QImage getMapImage() const;

	private:

		QImage mapImage;
	};
}
