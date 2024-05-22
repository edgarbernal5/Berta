/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CARET_HEADER
#define BT_CARET_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/Timer.h"

namespace Berta
{
	struct Window;

	class Caret
	{
	public:
		Caret(Window* owner, const Size& size);
		~Caret();

		void Activate();
		void Show(bool visible);
		void Deactivate();

		bool IsVisible() { return m_timer.IsRunning() && m_visible; }
	private:
		bool m_visible{ false };
		Window* m_owner{ nullptr };
		Size m_size{};
		Timer m_timer;
	};
}

#endif