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
		void ProcessEvents(Window* window, void(Renderer::*rendererPtr)(const TArgument&), Event<TArgument> ControlEvents::*eventPtr, TArgument& args);

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
	inline void Foundation::ProcessEvents(Window* window, void(Renderer::*rendererPtr)(const TArgument&), Event<TArgument> ControlEvents::*eventPtr, TArgument& args)
	{
		if (!m_windowManager.Exists(window))
		{
			return;
		}
		if (rendererPtr)
		{
			(window->Renderer.*rendererPtr)(args);
		}
		
		if (eventPtr)
		{
			(*window->Events.*eventPtr).Emit(args);
		}

		if (m_windowManager.Exists(window) && (window->Status == WindowStatus::Updated || (std::is_same_v<TArgument, ArgResize>)))
		{
			m_windowManager.DeferredUpdate(window);
		}
		window->Status = WindowStatus::None;
	}

}

#endif