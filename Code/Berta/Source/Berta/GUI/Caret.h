/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CARET_HEADER
#define BT_CARET_HEADER

#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	class Caret
	{
	public:
		Caret() = default;
		Caret(const Size& size);

	private:
		Size m_size{};
	};
}

#endif