/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CONTROL_HEADER
#define BT_CONTROL_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Interface.h"
#include "Berta/GUI/ControlReactor.h"
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
		void Hide();
		void Dispose();

		operator Window* () const { return m_handle; }
	protected:
		virtual void DoOnCaption(const std::wstring& caption);
		virtual std::wstring DoOnCaption();

		Window* m_handle{ nullptr };
	};

	template <typename Reactor, typename Events = CommonEvents>
	class Control : public ControlBase
	{
	public:
		using ReactorType = Reactor;
		using EventsType = Events;

		Control()
		{
			static_assert(std::is_base_of<ControlReactor, Reactor>::value, "Reactor must be derived from ControlReactor");
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
			m_handle = GUI::CreateForm(parent, rectangle, formStyle);
			m_appearance = new ControlAppearance();
			m_events = std::make_shared<Events>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRendererReactor(this, m_reactor);
		}

		void Create(Window* parent, const Rectangle& rectangle, bool visible = true)
		{
			m_handle = GUI::CreateControl(parent, rectangle);
			m_appearance = new ControlAppearance();
			m_events = std::make_shared<Events>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRendererReactor(this, m_reactor);
			if (visible)
			{
				GUI::ShowWindow(m_handle, true);
			}
		}

		ReactorType m_reactor;
		std::shared_ptr<Events> m_events;
		ControlAppearance* m_appearance{ nullptr };
	};
}

#endif