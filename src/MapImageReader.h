// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QImage>

namespace OrientView
{
	struct Settings;

	class MapImageReader
	{

	public:

		bool initialize(Settings* settings);
		QImage getMapImage() const;

	private:

		QImage mapImage;
	};
}
