/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_COLOR_BUFFER_HEADER
#define BT_COLOR_BUFFER_HEADER

#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	class ColorBuffer
	{
	public:


		struct Storage
		{
			unsigned char* m_imageData{ nullptr };
			Size m_size{};
		};
	};
}

#endif