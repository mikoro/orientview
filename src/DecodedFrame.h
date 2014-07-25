// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <stdint.h>

namespace OrientView
{
	struct DecodedFrame
	{
		uint8_t* data = nullptr;
		int dataLength = 0;
		int stride = 0;
		int width = 0;
		int height = 0;
		int64_t duration = 0;
	};
}
