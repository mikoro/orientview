// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include "RouteData.h"

namespace OrientView
{
	struct Settings;

	// Read route and calibration data from QuickRoute JPEG files.
	class QuickRouteReader
	{

	public:

		bool initialize(Settings* settings);
		const RouteData& getRouteData() const;

	private:

		RouteData routeData;
	};
}
