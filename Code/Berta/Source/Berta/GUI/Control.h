/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CONTROL_HEADER
#define BT_CONTROL_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Interface.h"
#include "Berta/GUI/ControlRenderer.h"
#include "Berta/GUI/ControlAppearance.h"
#include "Berta/GUI/CommonEvents.h"

namespace Berta
{
	class ControlBase
	{
	public:
		ControlBase() = default;
		virtual ~ControlBase() = default;

		Window* Handle() const { return m_handle; }

		void Caption(const std::wstring& caption);
		std::wstring Caption();
		void Show();

		operator Window* () const { return m_handle; }
	protected:
		Window* m_handle{ nullptr };
	};

	template <typename Renderer, typename Events = CommonEvents>
	class Control : public ControlBase
	{
	public:
		using RendererType = Renderer;
		using EventsType = Events;

		Control()
		{
			static_assert(std::is_base_of<ControlRenderer, Renderer>::value, "Renderer must be derived from ControlRenderer");
			static_assert(std::is_base_of<CommonEvents, Events>::value, "Events must be derived from CommonEvents");
		}

		virtual ~Control()
		{
			GUI::DisposeWindow(m_handle);
		}

		Control(const Control&) = delete;
		Control& operator=(const Control&) = delete;

		Control(Control&&) = delete;
		Control& operator=(Control&&) = delete;

		ControlAppearance& GetAppearance() { return *m_appearance; }
		EventsType& GetEvents() { return *m_events; }

	protected:
		void Create(Window* parent, const Rectangle& rectangle, const FormStyle& formStyle)
		{
			m_handle = GUI::CreateForm(rectangle, formStyle);
			m_appearance = new ControlAppearance();
			m_events = std::make_shared<Events>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRenderer(this, m_renderer);
		}

		void Create(Window* parent, const Rectangle& rectangle, bool visible = true)
		{
			m_handle = GUI::CreateControl(parent, rectangle);
			m_appearance = new ControlAppearance();
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
		ControlAppearance* m_appearance{ nullptr };
	};
}

#endif