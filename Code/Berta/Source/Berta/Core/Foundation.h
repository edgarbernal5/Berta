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

	private:

		static Foundation g_foundation;
		std::shared_ptr<Logger> m_logger;
		WindowManager m_windowManager;
	};

	template<typename TArgument>
	inline void Foundation::ProcessEvents(Window* window, void(Renderer::*rendererPtr)(const TArgument&), Event<TArgument> ControlEvents::*eventPtr, TArgument& args)
	{
		auto& ev = *window->Events.get();

		(window->Renderer.*rendererPtr)(args);
		
		(ev.*eventPtr).Emit(args);
	}
}

#endif