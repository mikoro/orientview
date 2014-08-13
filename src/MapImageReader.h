// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QImage>

namespace OrientView
{
	class Settings;

	// Read image data from normal image files.
	class MapImageReader
	{

	public:

		bool initialize(Settings* settings);

		QImage getMapImage() const;

	private:

		QImage mapImage;
	};
}
