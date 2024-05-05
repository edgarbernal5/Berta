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
	struct BasicWindow;

	class WidgetBase
	{
	public:
		WidgetBase() = default;
		virtual ~WidgetBase() = default;

		BasicWindow* Handle() const { return m_handle; }

		void Caption(const std::wstring& caption);
		void Show();

	protected:
		BasicWindow* m_handle{ nullptr };
	};

	template<typename Renderer>
	class Widget : public WidgetBase
	{
	public:
		using RendererType = Renderer;

		Widget() = default;
		virtual ~Widget()
		{
			GUI::DestroyWindow(m_handle);
		}

		Widget(const Widget&) = delete;
		Widget& operator=(const Widget&) = delete;

		Widget(Widget&&) = delete;
		Widget& operator=(Widget&&) = delete;

	protected:
		void Create(const Rectangle& rectangle)
		{
			m_handle = GUI::CreateWidget(rectangle);
		}

		void Create(const Rectangle& rectangle, const WindowStyle& windowStyle)
		{
			m_handle = GUI::CreateNativeWindow(rectangle, windowStyle);
			GUI::InitRenderer(this, m_renderer);
		}

		RendererType m_renderer;
	};
}

#endif