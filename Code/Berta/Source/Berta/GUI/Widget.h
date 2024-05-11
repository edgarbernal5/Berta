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
#include "Berta/GUI/CommonEvents.h"

namespace Berta
{

	class WidgetBase
	{
	public:
		WidgetBase() = default;
		virtual ~WidgetBase() = default;

		Window* Handle() const { return m_handle; }

		void Caption(const std::wstring& caption);
		std::wstring Caption();
		void Show();

		operator Window* () const { return m_handle; }
	protected:
		Window* m_handle{ nullptr };
	};

	template <typename Renderer, typename Events = CommonEvents>
	class Widget : public WidgetBase
	{
	public:
		using RendererType = Renderer;

		Widget() = default;
		virtual ~Widget()
		{
			GUI::DisposeWindow(m_handle);
		}

		Widget(const Widget&) = delete;
		Widget& operator=(const Widget&) = delete;

		Widget(Widget&&) = delete;
		Widget& operator=(Widget&&) = delete;

		WidgetAppearance& GetAppearance() { return *m_appearance; }
		Events& GetEvents() { return *m_events; }

	protected:
		void Create(Window* parent, const Rectangle& rectangle, const FormStyle& formStyle)
		{
			m_handle = GUI::CreateForm(rectangle, formStyle);
			m_appearance = new WidgetAppearance();
			m_events = std::make_shared<Events>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRenderer(this, m_renderer);
		}

		void Create(Window* parent, const Rectangle& rectangle, bool visible = true)
		{
			m_handle = GUI::CreateWidget(parent, rectangle);
			m_appearance = new WidgetAppearance();
			m_events = std::make_shared<Events>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRenderer(this, m_renderer);
			if (visible)
			{
				GUI::ShowWindow(m_handle, true);
			}
		}

		RendererType m_renderer;
		std::shared_ptr<Events> m_events;
		WidgetAppearance* m_appearance{ nullptr };
	};
}

#endif