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
#include "Berta/GUI/ControlEvents.h"

namespace Berta
{
	
	class ControlWindowInterface;

	//Type Erasure
	class ControlBase
	{
	public:
		class ControlWindow;

		friend class ControlWindowInterface;
	public:
		ControlBase() = default;
		virtual ~ControlBase() = default;

		Window* Handle() const { return m_handle; }

		std::string GetCaption() const;
		std::wstring GetCaptionW() const;
		void SetCaption(const std::wstring& caption);
		void SetCaption(const std::string& caption);

		bool GetEnabled() const;
		void SetEnabled(bool enabled);

		Window* GetParent() const;
		void SetParent(Window* newParent) const;

		Window* GetOwner() const;

		Point GetPosition() const;
		void SetPosition(const Point& newPosition);

		Rectangle GetArea() const;
		void SetArea(const Rectangle& area);

		Size GetSize() const;
		void SetSize(const Size& newSize);

		bool IsVisible() const;

		void Show() const;
		void Hide() const;
		void Dispose() const;

		void Capture(bool redirectToChildren);
		void ReleaseCapture();
		
		bool IsBorderless() const;
		Rectangle GetClientArea() const;
		
		bool IsAutoDraw() const;
		void SetAutoDraw(bool autoDraw) const;

		void SetBackgroundColor(const Color& newColor);

		void MakeActive(bool activated, Window* makeTargetWhenInactive);
		void Focus();
		
#if BT_DEBUG
		void SetDebugName(const std::string& name) const
		{
			m_handle->Name = name;
		}
#endif
		operator Window* () const { return m_handle; }

		class ControlWindow : public ControlWindowInterface
		{
		public:
			ControlWindow(ControlBase& control) : 
				m_control(control)
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
		virtual std::wstring DoOnCaption() const;
		virtual void DoOnCaption(const std::wstring& caption);

		virtual bool DoOnEnabled() const;
		virtual void DoOnEnabled(bool enabled);

		virtual Size DoOnSize() const;
		virtual void DoOnSize(const Size& newSize);

		virtual void DoOnMove(const Point& newPoint);
		virtual void DoOnMove(const Rectangle& newArea);

		void NotifyDestroy()
		{
			m_handle = nullptr;
			DoOnNotifyDestroy();
		}
		virtual void DoOnNotifyDestroy(){}

		Window* m_handle{ nullptr };
	};
	
	//Control
	template <typename CategoryTag, typename Reactor, typename Events = ControlEvents, typename Appearance = ControlAppearance>
	class Control : public ControlBase
	{
	public:
		using ReactorType = Reactor;
		using EventsType = Events;
		using AppearanceType = Appearance;

		Control()
		{
			// Validaciones en tiempo de compilación para garantizar el uso correcto de Berta
			static_assert(std::is_same<CategoryTag, Category::ControlTag>::value, 
				"Esta plantilla es exclusiva para controles hijos (WidgetTag)");
			
			static_assert(std::is_base_of<ControlReactor, Reactor>::value, "Reactor must be derived from ControlReactor");
			static_assert(std::is_base_of<ControlEvents, Events>::value, "Events must be derived from ControlEvents");
			static_assert(std::is_base_of<ControlAppearance, Appearance>::value, "Appearance must be derived from ControlAppearance");
		}

		~Control() override
		{
			GUI::DisposeWindow(m_handle);
		}

		AppearanceType& GetAppearance() const { return *m_appearance; }
		EventsType& GetEvents() const { return *m_events; }
		
		void Create(Window* parent, bool isUnscaleRect = false, const Rectangle& rectangle = {}, bool visible = true)
		{
			m_handle = GUI::CreateControl(parent, isUnscaleRect, rectangle, this, false);
			m_appearance = std::make_shared<AppearanceType>();
			m_events = std::make_shared<EventsType>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);

			GUI::InitRendererReactor(this, m_reactor);
			if (visible)
			{
				GUI::ShowWindow(m_handle, true);
			}
		}
	protected:
		void DoOnNotifyDestroy() override
		{
			m_events = std::make_shared<EventsType>();
		}
		
		ReactorType& GetReactor() { return m_reactor; }
		const ReactorType& GetReactor() const { return m_reactor; }
		
	private:
		ReactorType m_reactor;
		std::shared_ptr<EventsType> m_events;
		std::shared_ptr<AppearanceType> m_appearance;
	};
	
	//Panel
	template <typename Reactor, typename Events, typename Appearance>
	class Control<Category::PanelTag, Reactor, Events, Appearance> : public ControlBase
	{
	public:
		using ReactorType = Reactor;
		using EventsType = Events;
		using AppearanceType = Appearance;

		Control()
		{			
			static_assert(std::is_base_of<ControlReactor, Reactor>::value, "Reactor must be derived from ControlReactor");
			static_assert(std::is_base_of<ControlEvents, Events>::value, "Events must be derived from ControlEvents");
			static_assert(std::is_base_of<ControlAppearance, Appearance>::value, "Appearance must be derived from ControlAppearance");
		}

		~Control() override
		{
			GUI::DisposeWindow(m_handle);
		}

		AppearanceType& GetAppearance() const { return *m_appearance; }
		EventsType& GetEvents() const { return *m_events; }
		
		void Create(Window* parent, bool isUnscaleRect = false, const Rectangle& rectangle = {}, bool visible = true)
		{
			m_handle = GUI::CreateControl(parent, isUnscaleRect, rectangle, this, true);
			m_appearance = std::make_shared<AppearanceType>();
			m_events = std::make_shared<EventsType>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);
			
			if (visible)
			{
				GUI::ShowWindow(m_handle, true);
			}
		}
	protected:
		void DoOnNotifyDestroy() override
		{
			m_events = std::make_shared<EventsType>();
		}
		
		ReactorType& GetReactor() { return m_reactor; }
		const ReactorType& GetReactor() const { return m_reactor; }
		
	private:
		ReactorType m_reactor;
		std::shared_ptr<EventsType> m_events;
		std::shared_ptr<AppearanceType> m_appearance;
	};
	
	//Root
	template <typename Reactor, typename Events, typename Appearance>
	class Control<Category::RootTag, Reactor, Events, Appearance> : public ControlBase
	{
	public:
		using ReactorType = Reactor;
		using EventsType = Events;
		using AppearanceType = Appearance;

		Control()
		{
			static_assert(std::is_base_of<ControlReactor, Reactor>::value, "Reactor debe heredar de ControlReactor");
		}

		~Control() override { GUI::DisposeWindow(m_handle); }

		AppearanceType& GetAppearance() const { return *m_appearance; }
		EventsType& GetEvents() const { return *m_events; }
		
		void Create(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, const FormStyle& formStyle, bool isNested, bool isRenderForm = false)
		{
			m_handle = GUI::CreateForm(parent, isUnscaleRect, rectangle, formStyle, isNested, this, isRenderForm);
			m_appearance = std::make_shared<AppearanceType>();
			m_events = std::make_shared<EventsType>();
			GUI::SetEvents(m_handle, m_events);
			GUI::SetAppearance(m_handle, m_appearance);

			GUI::InitRendererReactor(this, m_reactor);
		}
	protected:
		void DoOnNotifyDestroy() override
		{
			m_events = std::make_shared<EventsType>();
		}
		
		ReactorType& GetReactor() { return m_reactor; }
		const ReactorType& GetReactor() const { return m_reactor; }
		
	private:
		ReactorType m_reactor;
		std::shared_ptr<EventsType> m_events;
		std::shared_ptr<AppearanceType> m_appearance;
	};
}

#endif