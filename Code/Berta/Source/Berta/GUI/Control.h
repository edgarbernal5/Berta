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
	class ControlWindowInterface;

	class ControlBase
	{
	public:
		class ControlWindow;

		friend class ControlWindowInterface;
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

		class ControlWindow : public ControlWindowInterface
		{
		public:
			ControlWindow(ControlBase& control) : m_control(control)
			{
			}

			virtual ControlBase* ControlPtr() const override
			{
				return &m_control;
			}

			virtual void Destroy() override
			{
				if (m_isDestroyed)
				{
					return;
				}

				m_isDestroyed = true;
				m_control.NotifyDestroy();
			}

		private:
			ControlBase& m_control;
			bool m_isDestroyed{ false };
		};
	protected:
		virtual void DoOnCaption(const std::wstring& caption);
		virtual std::wstring DoOnCaption() const;
		virtual void DoOnEnabled(bool enabled);
		virtual bool DoOnEnabled() const;

		virtual void DoOnSize(const Size& newSize);
		virtual Size DoOnSize() const;

		void NotifyDestroy()
		{
			m_handle = nullptr;
			DoOnNotifyDestroy();
		}
		virtual void DoOnNotifyDestroy(){}

		Window* m_handle{ nullptr };
	};

	template <typename Reactor, typename Events = CommonEvents, typename Appearance = ControlAppearance>
	class Control : public ControlBase
	{
	public:
		using ReactorType = Reactor;
		using EventsType = Events;
		using AppearanceType = Appearance;

	public:
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

		void Create(Window* parent, const Rectangle& rectangle, const FormStyle& formStyle)
		{
			m_handle = GUI::CreateForm(parent, rectangle, formStyle, this);
			m_appearance = std::make_shared<Appearance>();
			m_events = std::make_shared<Events>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			GUI::InitRendererReactor(this, m_reactor);
		}

		//TODO: hacer este método polimórfico para que la clase Panel no inicie renderer ni reactor
		void Create(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool visible = true, bool isPanel = false)
		{
			m_handle = GUI::CreateControl(parent, isUnscaleRect, rectangle, this, isPanel);
			m_appearance = std::make_shared<Appearance>();
			m_events = std::make_shared<Events>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			if (!isPanel)
			{
				GUI::InitRendererReactor(this, m_reactor);
			}

			if (visible)
			{
				GUI::ShowWindow(m_handle, true);
			}
		}

	protected:

		virtual void DoOnNotifyDestroy() override
		{
			m_events = std::make_shared<Events>();
		}

		ReactorType m_reactor;
		std::shared_ptr<Events> m_events;
		std::shared_ptr<ControlAppearance> m_appearance;
	};
}

#endif