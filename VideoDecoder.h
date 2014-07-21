// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <string>

namespace OrientView
{
	class VideoDecoder
	{
	public:

		VideoDecoder();
		~VideoDecoder();

		void LoadVideo(const std::string& fileName);

	private:

		static bool isRegistered;
	};
}
