/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_IMAGE_PROCESSOR_HEADER
#define BT_IMAGE_PROCESSOR_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Paint/ColorBuffer.h"

namespace Berta::ImageProcessor
{
	void ScaleBilinearWithAlphaBlend(ColorBuffer& sourceBuffer, const Rectangle& sourceRect, ColorBuffer& destBuffer, const Rectangle& destRect);
	void AlphaBlend(ColorBuffer& sourceBuffer, const Rectangle& sourceRect, ColorBuffer& destBuffer, const Point& destPos, double alpha);
	void ScaleNearestAlphaBlend(ColorBuffer& sourceBuffer, const Rectangle& sourceRect, ColorBuffer& destBuffer, const Rectangle& destRect);
}

#endif