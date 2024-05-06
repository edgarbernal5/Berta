/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WIDGET_HEADER
#define BT_WIDGET_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Interface.h"
#include "Berta/GUI/WidgetRenderer.h"
#include "Berta/GUI/WidgetAppearance.h"

namespace Berta
{

	class WidgetBase
	{
	public:
		WidgetBase() = default;
		virtual ~WidgetBase() = default;

		BasicWindow* Handle() const { return m_handle; }

		void Caption(const std::wstring& caption);
		std::wstring Caption();
		void Show();

		operator BasicWindow* () const { return m_handle; }
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

		WidgetAppearance& GetAppearance() { return *m_appearance; }

	protected:
		void Create(BasicWindow* parent, const Rectangle& rectangle)
		{
			m_handle = GUI::CreateWidget(parent, rectangle);
			m_appearance = new WidgetAppearance();
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRenderer(this, m_renderer);
		}

		void Create(BasicWindow* parent, const Rectangle& rectangle, const WindowStyle& windowStyle)
		{
			m_handle = GUI::CreateNativeWindow(rectangle, windowStyle);
			m_appearance = new WidgetAppearance();
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRenderer(this, m_renderer);
		}

		RendererType m_renderer;
		WidgetAppearance* m_appearance{ nullptr };
	};
}

#endif