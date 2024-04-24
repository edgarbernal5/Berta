
#ifndef BERTA_BASIC_TYPES_HPP
#define BERTA_BASIC_TYPES_HPP

#include <cstdint>

namespace Berta
{
	struct Rectangle
	{
		int x;
		int y;
		uint32_t width;
		uint32_t height;
	};
}

#endif