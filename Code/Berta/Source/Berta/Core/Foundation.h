/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FOUNDATION_HEADER
#define BT_FOUNDATION_HEADER

#include "Berta/GUI/WindowManager.h"
#include "Berta/GUI/Renderer.h"
#include "Berta/Core/Event.h"
#include "Berta/GUI/Window.h"
#include "Berta/GUI/ControlEvents.h"

#include <functional>

namespace Berta
{
	class Logger;
	struct Menu;

	class Foundation
	{
	public:
		struct MenuData
		{
			Window* m_menuBar{ nullptr };
			Menu* m_activeMenu{ nullptr };
		};

		Foundation();
		~Foundation();

		Foundation(const Foundation&) = delete;
		Foundation& operator=(const Foundation&) = delete;

		WindowManager& GetWindowManager() { return m_windowManager; }
		void ProcessMessages();

		template <typename TArgument>
		void ProcessEvents(Window* window, void(Renderer::* rendererEventPtr)(const TArgument&), Event<TArgument> ControlEvents::*eventPtr, TArgument& args);

		static Foundation& GetInstance();

		class RootGuard
		{
		public:
			RootGuard(Window* window);
			~RootGuard();

			Window* m_window;
		};
	private:
		static Foundation g_foundation;
		std::shared_ptr<Logger> m_logger;
		WindowManager m_windowManager;
	};

	template<typename TArgument>
	inline void Foundation::ProcessEvents(Window* window, void(Renderer::* rendererEventPtr)(const TArgument&), Event<TArgument> ControlEvents::*eventPtr, TArgument& args)
	{
		if (!m_windowManager.Exists(window))
		{
			return;
		}

		window->Status = WindowStatus::None;
		if (rendererEventPtr)
		{
			(window->Renderer.*rendererEventPtr)(args);
		}
		
		if (eventPtr)
		{
			(*window->Events.*eventPtr).Emit(args);
		}

		if (!m_windowManager.Exists(window))
		{
			return;
		}

		bool isResizing = std::is_same_v<TArgument, ArgResize>;
		if (window->Visible && (window->Status == WindowStatus::Updated || isResizing))
		{
			if (window->Type != WindowType::Panel && window->IsBatchActive())
			{
				m_windowManager.TryAddWindowToBatch
				(
					window, 
					window->Status == WindowStatus::Updated ? DrawOperation::NeedMap : DrawOperation::NeedUpdate | DrawOperation::NeedMap
				);
			}
		}
	}
}

#endif