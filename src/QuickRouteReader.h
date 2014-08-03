// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include "RouteData.h"

namespace OrientView
{
	class Settings;

	class QuickRouteReader
	{

	public:

		bool initialize(Settings* settings);
		const RouteData& getRouteData() const;

	private:

		RouteData routeData;
	};
}
