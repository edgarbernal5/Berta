/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Image.h"

namespace Berta
{
	Image::Image(const std::filesystem::path& filepath)
	{
		Open(filepath);
	}

	Image::~Image()
	{
	}

	void Image::Open(const std::filesystem::path& filepath)
	{
	}

	void Image::Paste(Graphics& destination, const Point& positionDestination)
	{
	}
}