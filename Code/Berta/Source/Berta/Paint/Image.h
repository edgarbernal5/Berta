/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_IMAGE_HEADER
#define BT_IMAGE_HEADER

#include "Berta/Core/BasicTypes.h"
#include <filesystem>

namespace Berta
{
	class Graphics;

	class Image
	{
	public:
		Image(const std::filesystem::path& filepath);
		~Image();

		void Open(const std::filesystem::path& filepath);
		void Paste(Graphics& destination, const Point& positionDestination);
	private:
	};
}

#endif