/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LIST_BOX_HEADER
#define BT_LIST_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/Panel.h"

#include <string>
#include <vector>

namespace Berta
{
	class ListBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		struct Headers
		{
			struct Item
			{
				Item() {}
				Item(const std::string& text, uint32_t width) : Name(text), Width(width) {}

				std::string Name;
				uint32_t Width{ 120 };
			};

			void Append(const std::string& text, uint32_t width);

			std::vector<Item> Items;
		};

		struct Cell
		{
			std::string Text;
		};

		struct List
		{
			std::vector<Cell> Cells;
		};

		Headers& GetHeaders() { return m_headers; }
	private:
		void CalculateViewport(Rectangle& backgroundRect);

		Headers m_headers;
		Window* m_window;
		ListBoxAppearance* m_appearance{ nullptr };
	};

	class ListBox : public Control<ListBoxReactor, ListBoxEvents, ListBoxAppearance>
	{
	public:
		ListBox() = default;
		ListBox(Window* parent, const Rectangle& rectangle);

		void AppendHeader(const std::string& name, uint32_t width = 120);
		void Append(const std::string& text);
		void Append(std::initializer_list<std::string> texts);
		void Clear();
		void ClearHeaders();
	};
}

#endif
