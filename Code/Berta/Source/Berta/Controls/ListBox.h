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

			Graphics DraggingBox;
			int MouseDownOffset{ 0 };
			int MouseDraggingPosition{ 0 };
			int DraggingTargetIndex{ 0 };
			int SelectedIndex{ -1 };
			bool IsDragging{ false };
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
				bool IsSelected{ false };
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
			uint32_t ItemHeight{ 0 };
			uint32_t ItemHeightWithMargin{ 0 };

			uint32_t ColumnOffsetStartOff{ 0 };

			int StartingVisibleIndex{ -1 };
			int EndingVisibleIndex{ -1 };
		};

		struct MouseSelection
		{
			bool IsSelected(size_t index) const;
			void Deselect(size_t index);

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
			void CalculateVisibleIndices();
			void BuildHeaderBounds(size_t startIndex = 0);
			void BuildListItemBounds(size_t startIndex = 0);

			void Erase(size_t index);
			void EnableMultiselection(bool enabled);
			bool UpdateScrollBars();
			InteractionArea DetermineHoverArea(const Point& mousePosition);

			bool HandleMultiSelection(int itemIndexAtPosition, const ArgMouse& args);
			void SelectItem(int index);
			void ClearSelection();
			void EnsureVisibility(int lastSelectedIndex);
			void PerformRangeSelection(int itemIndexAtPosition);

			void ToggleItemSelection(int itemIndexAtPosition);
			void StartSelectionRectangle(const Point& mousePosition);
			bool ClearSelectionIfNeeded();
			bool ClearSingleSelection();

			std::vector<size_t> GetSelectedItems() const;

			int GetHeaderAtMousePosition(const Point& mousePosition, bool splitter);

			void StartHeadersSizing(const Point& mousePosition);
			void UpdateHeadersSize(const Point& mousePosition);
			void StopHeadersSizing();

			void DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds);

			void DrawHeaders(Graphics& graphics);
			void DrawHeaderItem(Graphics& graphics, const Rectangle& rect, const std::string& name, bool isHovered, uint32_t leftTextMargin);
			void DrawList(Graphics& graphics);

			InteractionArea m_hoveredArea{ InteractionArea::None };
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
		const Module& GetModule() const { return m_module; }
	private:

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
		void Erase(uint32_t index);

		void EnableMultiselection(bool enabled);

		std::vector<size_t> GetSelected() const;
	};
}

#endif
