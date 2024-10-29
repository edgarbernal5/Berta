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
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyReleased(Graphics& graphics, const ArgKeyboard& args) override;

		struct Headers
		{
			struct ItemData
			{
				ItemData() {}
				ItemData(const std::string& text, uint32_t width) : Name(text)
				{
					Bounds.Width = width;
				}

				std::string Name;
				Rectangle Bounds;
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
				Item(const std::string& text)
				{
					Cells.emplace_back(text);
				}
				std::vector<Cell> Cells;
				Rectangle Bounds;
			};

			std::vector<Item> Items;
			std::vector<std::size_t> SortedIndexes;
		};

		enum class InteractionArea
		{
			None,
			Header,
			HeaderSplitter,
			List,
			ListBlank
		};

		struct ViewportData
		{
			Rectangle BackgroundRect{};
			bool NeedVerticalScroll{ false };
			bool NeedHorizontalScroll{ false };
			Size ContentSize{};
			uint32_t InnerMargin{ 0 };
		};

		struct MouseSelection
		{
			std::vector<size_t> m_selections;
			std::vector<size_t> m_alreadySelected;
			int m_pressedIndex{ -1 };
			int m_hoveredIndex{ -1 };
			int m_selectedIndex{ -1 };
			Point m_startPosition;
			Point m_endPosition;
			bool m_started{ false };
			bool m_inverseSelection{ false };
		};

		struct Module
		{
			Headers Headers;
			List List;

			void AppendHeader(const std::string& text, uint32_t width);
			void Append(const std::string& text);
			void Append(std::initializer_list<std::string> texts);
			
			void Clear();
			void CalculateViewport(ViewportData& viewportData);
			void BuildHeaderBounds(uint32_t startIndex);
			void BuildListItemBounds(uint32_t startIndex);

			void EnableMultiselection(bool enabled);
			bool UpdateScrollBars();
			InteractionArea DetermineHoverArea(const Point& mousePosition);

			InteractionArea m_hoverArea{ InteractionArea::None };
			InteractionArea m_pressedArea{ InteractionArea::None };
			Point m_mouseDownPosition{};

			Point ScrollOffset{};
			std::unique_ptr<ScrollBar> m_scrollBarVert;
			std::unique_ptr<ScrollBar> m_scrollBarHoriz;
			MouseSelection m_mouseSelection;
			ViewportData m_viewport;
			Window* m_window;
			ListBoxAppearance* m_appearance{ nullptr };
			bool m_multiselection{ true };
			bool m_shiftPressed{ false };
			bool m_ctrlPressed{ false };
		};

		Module& GetModule() { return m_module; }
	private:

		void DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds);

		void DrawHeaders(Graphics& graphics);
		void DrawList(Graphics& graphics);


		Module m_module;

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

		void EnableMultiselection(bool enabled);
	};
}

#endif
