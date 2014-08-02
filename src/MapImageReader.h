// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QImage>

namespace OrientView
{
	struct Settings;

	class MapImageReader
	{

	public:

		bool initialize(const QString& fileName, Settings* settings);

		QImage getMapImage() const;

	private:

		QImage mapImage;
	};
}
