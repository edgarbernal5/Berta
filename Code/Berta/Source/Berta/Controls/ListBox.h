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
#include "Berta/Controls/ScrollBar.h"

#include <string>
#include <vector>

namespace Berta
{
	constexpr uint32_t LISTBOX_MIN_HEADER_WIDTH = 80u;

	class ListBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;

		struct Headers
		{
			struct ItemData
			{
				ItemData() {}
				ItemData(const std::string& text, uint32_t width) : Name(text), Width(width) {}

				std::string Name;
				uint32_t Width{ 120 };
			};

			std::vector<ItemData> Items;
		};

		struct Cell
		{
			Cell(const std::string& text) : Text(text){}

			std::string Text;
		};

		struct List
		{
			struct Item
			{
				Item(const std::string& text) {
					Cells.emplace_back(text);
				}
				std::vector<Cell> Cells;
			};

			std::vector<Item> Items;
			std::vector<std::size_t> SortedIndexes;
		};
		
		struct Module
		{
			Headers Headers;
			List List;

			void AppendHeader(const std::string& text, uint32_t width);
			void Append(const std::string& text);
			void Append(std::initializer_list<std::string> texts);
			
			void Clear();


			Point ScrollOffset{};
			std::unique_ptr<ScrollBar> m_scrollBarVert;
			std::unique_ptr<ScrollBar> m_scrollBarHoriz;
		};

		Module& GetModule() { return m_module; }
	private:
		void CalculateViewport(Rectangle& backgroundRect, bool& needVerticalScroll, bool& needHorizontalScroll, Size& contentSize);
		void DrawStringInBox(Graphics& graphics, const std::string& str, Rectangle boxBounds);
		bool UpdateScrollBars(const Rectangle& backgroundRect, bool needVerticalScroll, bool needHorizontalScroll, const Size& contentSize);

		void DrawHeaders(Graphics& graphics);
		void DrawList(Graphics& graphics);

		Module m_module;

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
