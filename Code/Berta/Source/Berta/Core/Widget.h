/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WIDGET_HEADER
#define BT_WIDGET_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/WidgetRenderer.h"
#include "Berta/GUI/Interface.h"

namespace Berta
{
	class BasicWindow;

	class WidgetBase
	{
	public:
		WidgetBase() {}
		virtual ~WidgetBase() = default;

		BasicWindow* Handle() const { return m_handle; }

		void Caption(const std::wstring& caption);
		void Show();

	protected:
		void Create(const Rectangle& rectangle);
		void Create(const Rectangle& rectangle, const WindowStyle& appearance);

		BasicWindow* m_handle{ nullptr };
	};

	template<typename Renderer>
	class Widget : public WidgetBase
	{
	public:
		Widget() = default;
		virtual ~Widget()
		{
			GUI::DestroyWindow(m_handle);
		}

		Widget(const Widget&) = delete;
		Widget& operator=(const Widget&) = delete;

		Widget(Widget&&) = delete;
		Widget& operator=(Widget&&) = delete;

	private:
		Renderer m_renderer;
	};
}

#endif