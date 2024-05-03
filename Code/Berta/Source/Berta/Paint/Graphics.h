/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_GRAPHICS_HEADER
#define BT_GRAPHICS_HEADER

#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	/*
	* Wrapper for GDI functions.
	*/
	class Graphics
	{
	public:
		Graphics();

		void DrawRectangle(const Rectangle& rectangle, bool solid);

		HDC m_hdc;
	};
}

#endif