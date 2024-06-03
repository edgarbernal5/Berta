/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FLOAT_BOX_HEADER
#define BT_FLOAT_BOX_HEADER

#include <string>
#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"

namespace Berta
{
	class FloatBox;

	class FloatBoxReactor : public ControlReactor
	{
	public:
		~FloatBoxReactor();

		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;

		void SetIndex(int index);
	private:
		FloatBox* m_control{ nullptr };
		int m_index{ -1 };
	};

	class FloatBox : public Control<FloatBoxReactor, RootEvents>
	{
	public:
		FloatBox(Window* parent, const Rectangle& rectangle);
		~FloatBox();

		void SetItems(std::vector<std::wstring>& items)
		{
			m_items = &items;
		}

		void SetSelectedIndex(int& index)
		{
			m_selectedIndex = &index;
			m_reactor.SetIndex(index);
		}

		friend class FloatBoxReactor;
	private:
		std::vector<std::wstring>* m_items{ nullptr };
		int* m_selectedIndex{ nullptr };
		bool m_ignoreFirstMouseUp{ false };
	};
}

#endif