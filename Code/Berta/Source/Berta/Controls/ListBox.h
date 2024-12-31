/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LIST_BOX_HEADER
#define BT_LIST_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/ScrollBar.h"
#include "Berta/Paint/Image.h"

#include <string>
#include <vector>

namespace Berta
{
	constexpr uint32_t LISTBOX_MIN_HEADER_WIDTH = 80u;

	struct ListBoxItem;

	struct ListBoxAppearance : public ControlAppearance
	{
		uint32_t HeadersHeight = 26;
		uint32_t ListItemHeight = 24;
		uint32_t ListItemIconSize = 16;
		uint32_t ListItemIconMargin = 4;
	};

	class ListBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		void DblClick(Graphics& graphics, const ArgMouse& args) override;
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
				ItemData(const std::string& name, uint32_t width) : m_name(name)
				{
					m_bounds.Width = width;
				}

				std::string m_name;
				Rectangle m_bounds;
			};

			Graphics m_draggingBox;
			int m_mouseDownOffset{ 0 };
			int m_mouseDraggingPosition{ 0 };
			int m_draggingTargetIndex{ 0 };
			int m_selectedIndex{ -1 };
			int m_sortedHeaderIndex{ -1 };
			bool isAscendingOrdering{ true };
			bool m_isDragging{ false };
			std::vector<ItemData> m_items;

			std::vector<size_t> m_sorted;
		};

		struct Cell
		{
			Cell(const std::string& text) : m_text(text){}

			std::string m_text;
		};

		struct List
		{
			struct Item
			{
				Item(const std::string& text)
				{
					m_cells.emplace_back(text);
				}
				std::vector<Cell> m_cells;
				Rectangle m_bounds;
				bool m_isSelected{ false };
				Image m_icon;
			};

			std::vector<Item> m_items;
			std::vector<std::size_t> m_sortedIndexes;
			bool m_drawImages{ false };
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
			Rectangle m_backgroundRect{};
			bool m_needVerticalScroll{ false };
			bool m_needHorizontalScroll{ false };
			Size m_contentSize{};
			uint32_t m_innerMargin{ 0 };
			uint32_t m_itemHeight{ 0 };
			uint32_t m_itemHeightWithMargin{ 0 };

			uint32_t m_columnOffsetStartOff{ 0 };

			int m_startingVisibleIndex{ -1 };
			int m_endingVisibleIndex{ -1 };
		};

		struct MouseSelection
		{
			bool IsAlreadySelected(List::Item* item) const;
			bool IsSelected(List::Item* item) const;

			void Select(List::Item* item);
			void Deselect(List::Item* item);

			std::vector<List::Item*> m_selections;
			std::vector<List::Item*> m_alreadySelected; //TODO: cambiar por un set/map
			List::Item* m_pressedItem{ nullptr };
			List::Item* m_hoveredItem{ nullptr };
			List::Item* m_selectedItem{ nullptr };

			Point m_startPosition;
			Point m_endPosition;
			bool m_started{ false };
			bool m_inverseSelection{ false };
		};

		struct Module
		{
			void AppendHeader(const std::string& text, uint32_t width);
			void Append(const std::string& text);
			void Append(std::initializer_list<std::string> texts);
			ListBoxItem At(size_t index);

			void Clear();
			void ClearHeaders();
			void CalculateViewport(ViewportData& viewportData);
			void CalculateVisibleIndices();
			void BuildHeaderBounds(size_t startIndex = 0);
			void BuildListItemBounds(size_t startIndex = 0);

			void Erase(ListBoxItem item);
			void Erase(std::vector<ListBoxItem>& items);
			void EnableMultiselection(bool enabled);
			bool UpdateScrollBars();
			InteractionArea DetermineHoverArea(const Point& mousePosition);

			bool HandleMultiSelection(List::Item* item, const ArgMouse& args);
			void SelectItem(List::Item* index);
			void ClearSelection();
			bool EnsureVisibility(int lastSelectedIndex);
			void PerformRangeSelection(List::Item* itemIndexAtPosition);

			bool UpdateSingleSelection(List::Item* item);
			void ToggleItemSelection(List::Item* item);
			void StartSelectionRectangle(const Point& mousePosition);
			bool ClearSelectionIfNeeded();
			bool ClearSingleSelection();

			std::vector<ListBoxItem> GetSelectedItems();

			int GetHeaderAtMousePosition(const Point& mousePosition, bool splitter);

			void StartHeadersSizing(const Point& mousePosition);
			void UpdateHeadersSize(const Point& mousePosition);
			void StopHeadersSizing();

			void DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds, const Color& textColor);

			void DrawHeaders(Graphics& graphics);
			void DrawHeaderItem(Graphics& graphics, const Rectangle& rect, const std::string& name, bool isHovered, const Rectangle& textRect, const Color& textColor);
			void DrawList(Graphics& graphics);

			void CalculateSelectionBox(Point& startPoint, Point& endPoint, Size& boxSize);
			bool SetHoveredListItem(List::Item* index = nullptr);

			void SortHeader(int headerIndex, bool ascending);

			int GetListItemIndex(List::Item* item);

			Headers m_headers;
			List m_list;

			InteractionArea m_hoveredArea{ InteractionArea::None };
			InteractionArea m_pressedArea{ InteractionArea::None };

			Point m_scrollOffset{};
			std::unique_ptr<ScrollBar> m_scrollBarVert;
			std::unique_ptr<ScrollBar> m_scrollBarHoriz;
			MouseSelection m_mouseSelection;
			ViewportData m_viewport;
			Window* m_window{ nullptr };
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

	struct ListBoxItem
	{
		ListBoxItem(ListBoxReactor::List::Item* target, ListBoxReactor::Module& module) :
			m_target(target), m_module(module)
		{
		}

		void SetIcon(const Image& image);

		friend class ListBoxReactor::Module;
	private:
		ListBoxReactor::List::Item* m_target;
		ListBoxReactor::Module& m_module;
	};

	struct ArgListBox
	{
		size_t SelectedIndex{ 0 };
	};

	struct ListBoxEvents : public ControlEvents
	{
		Event<ArgListBox> Selected;
	};

	class ListBox : public Control<ListBoxReactor, ListBoxEvents, ListBoxAppearance>
	{
	public:
		ListBox() = default;
		ListBox(Window* parent, const Rectangle& rectangle);

		void AppendHeader(const std::string& name, uint32_t width = 120);
		void Append(const std::string& text);
		void Append(std::initializer_list<std::string> texts);
		ListBoxItem At(size_t index);
		void Clear();
		void ClearHeaders();
		void Erase(ListBoxItem item);
		void Erase(std::vector<ListBoxItem>& items);

		void EnableMultiselection(bool enabled);

		std::vector<ListBoxItem> GetSelected();
	};
}

#endif
