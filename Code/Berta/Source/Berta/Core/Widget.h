/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WIDGET_HEADER
#define BT_WIDGET_HEADER

#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	class BasicWindow;

	class Widget
	{
	public:
		void Show();

		BasicWindow* Handle() const { return m_handle; }

		void Caption(const std::wstring& caption);
	protected:
		void Create(const Rectangle& rectangle);
		void Create(const Rectangle& rectangle, const WindowStyle& appearance);

		BasicWindow* m_handle{ nullptr };
	};
}

#endif