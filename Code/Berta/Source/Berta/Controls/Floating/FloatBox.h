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
	private:
		FloatBox* m_control{ nullptr };
	};

	class FloatBox : public Control<FloatBoxReactor, RootEvents>
	{
	public:
		FloatBox(Window* parent, const Rectangle& rectangle);
		void SetItems(std::vector<std::wstring>& items)
		{
			m_items = &items;
		}

		friend class FloatBoxReactor;
	private:
		std::vector<std::wstring>* m_items{ nullptr };
	};
}

#endif