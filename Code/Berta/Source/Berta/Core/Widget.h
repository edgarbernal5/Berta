/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WIDGET_HEADER
#define BT_WIDGET_HEADER

namespace Berta
{
	class BasicWindow;

	class Widget
	{
	public:
		void Show();

		BasicWindow* Handle() const;

	protected:
		BasicWindow* m_handle{ nullptr };
	};
}

#endif