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

		if (rendererEventPtr)
		{
			(window->Renderer.*rendererEventPtr)(args);
		}
		
		if (m_windowManager.Exists(window) && eventPtr)
		{
			(*window->Events.*eventPtr).Emit(args);
		}

		bool isResizing = std::is_same_v<TArgument, ArgResize>;
		if (m_windowManager.Exists(window) && (window->Status == WindowStatus::Updated || isResizing))
		{
			if (!m_windowManager.TryDeferredUpdate(window))
			{
				if (window->Visible)
				{
					if (window->Status == WindowStatus::Updated)
					{
						if (window->Type != WindowType::Panel)
						{
							m_windowManager.Paint(window, isResizing);
							m_windowManager.Map(window, nullptr); // Copy from root graphics to native hwnd window.
						}
					}
				}
			}
		}

		window->Status = WindowStatus::None;
	}
}

#endif