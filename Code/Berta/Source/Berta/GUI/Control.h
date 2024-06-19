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

		void SetCaption(const std::wstring& caption);
		std::wstring GetCaption() const;

		bool GetEnabled() const;
		void SetEnabled(bool enabled);
		
		Size GetSize() const;
		void SetSize(const Size& newSize);
		
		void Show();
		void Hide();
		void Dispose();

#if BT_DEBUG
		void SetDebugName(const std::string& name)
		{
			m_handle->Name = name;
		}
#endif
		operator Window* () const { return m_handle; }
	protected:
		virtual void DoOnCaption(const std::wstring& caption);
		virtual std::wstring DoOnCaption() const;
		virtual void DoOnEnabled(bool enabled);
		virtual bool DoOnEnabled() const;

		virtual void DoOnSize(const Size& newSize);
		virtual Size DoOnSize() const;

		Window* m_handle{ nullptr };
	};

	template <typename Reactor, typename Events = CommonEvents, typename Appearance = ControlAppearance>
	class Control : public ControlBase
	{
	public:
		using ReactorType = Reactor;
		using EventsType = Events;
		using AppearanceType = Appearance;

		Control()
		{
			static_assert(std::is_base_of<ControlReactor, Reactor>::value, "Reactor must be derived from ControlReactor");
			static_assert(std::is_base_of<CommonEvents, Events>::value, "Events must be derived from CommonEvents");
			static_assert(std::is_base_of<ControlAppearance, Appearance>::value, "Appearance must be derived from ControlAppearance");
		}

		virtual ~Control()
		{
			GUI::DisposeWindow(m_handle);
		}

		Control(const Control&) = delete;
		Control& operator=(const Control&) = delete;

		Control(Control&&) = delete;
		Control& operator=(Control&&) = delete;

		AppearanceType& GetAppearance() const { return *m_appearance; }
		EventsType& GetEvents() const { return *m_events; }

	protected:
		void Create(Window* parent, const Rectangle& rectangle, const FormStyle& formStyle)
		{
			m_handle = GUI::CreateForm(parent, rectangle, formStyle);
			m_appearance = std::make_shared<Appearance>();
			m_events = std::make_shared<Events>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRendererReactor(this, m_reactor);
		}

		void Create(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool visible = true)
		{
			m_handle = GUI::CreateControl(parent, isUnscaleRect, rectangle);
			m_appearance = std::make_shared<Appearance>();
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
		std::shared_ptr<ControlAppearance> m_appearance;
	};
}

#endif