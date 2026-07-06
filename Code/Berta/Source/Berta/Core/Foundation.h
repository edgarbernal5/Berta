/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FOUNDATION_HEADER
#define BT_FOUNDATION_HEADER

#include "Berta/GUI/WindowManager.h"
#include "Berta/GUI/MenuManager.h"
#include "Berta/GUI/Renderer.h"
#include "Berta/Core/Event.h"
#include "Berta/GUI/Window.h"
#include "Berta/GUI/ControlEvents.h"
#include "Berta/GUI/UIRendererCoordinator.h"

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

		static Foundation& GetInstance();

		WindowManager& GetWindowManager() { return m_windowManager; }
		MenuManager& GetMenuManager() { return m_menuManager; }
		void ProcessMessages(const std::function<bool()>& keepRunning = nullptr);

		template <typename TArgument>
		void ProcessEvents(Window* window, void(Renderer::* rendererEventPtr)(const TArgument&), Event<TArgument> ControlEvents::*eventPtr, TArgument& args);

		void EventEnterSizeMove(Window* window);
		void EventExitSizeMove(Window* window);

		class RootGuard
		{
		public:
			RootGuard(Window* window);
			~RootGuard();

			Window* m_window;
		};
	private:

		static Foundation g_foundation;
		WindowManager m_windowManager;
		MenuManager m_menuManager;
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
		
		if (eventPtr)
		{
			(*window->Events.*eventPtr).Emit(args);
		}
		
		// Nota: El redibujado ahora sucede porque el Reactor llamó a GUI::MarkAsNeedUpdate()
		// internamente durante los eventos, lo que ya le avisó a Win32.
	}
}

#endif