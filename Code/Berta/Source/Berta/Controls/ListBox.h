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
#include "Berta/GUI/ScrollableView.h"
#include "Berta/GUI/SelectionController.h"
#include "Berta/GUI/LassoSelection.h"

#include <string>
#include <vector>
#include <any>
#include <unordered_set>

namespace Berta
{
	constexpr uint32_t LISTBOX_MIN_HEADER_WIDTH = 80u;
	
	namespace ReactorCore::ListBox
	{
		struct ListBoxItem;

		struct Appearance : public ControlAppearance
		{
			uint32_t HeadersHeight = 26;
			uint32_t ListItemHeight = 24;
			uint32_t ListItemIconSize = 16;
			uint32_t ListItemIconMargin = 4;
		};
		
		namespace List
		{
			struct Cell 
			{
				Cell(const std::string& text) :
					m_text(text){}
				
				std::string m_text; 
				
			};

			struct Item
			{
				std::vector<Cell> m_cells;
				Rectangle m_bounds;
				bool m_isSelected{ false };
				Image m_icon;
				std::any m_userData;
			};
		}
		
		class ItemCollection
		{
		public:
			// Callback para que la Vista sepa cuándo debe repintarse o recalcular el scroll
			using OnCollectionChangedCallback = std::function<void()>;
     				
		public:
			ItemCollection() = default;
             
			void SetOnChangedCallback(OnCollectionChangedCallback callback) { m_onChanged = std::move(callback); }
             
			size_t Append(const std::string& text);
			size_t Append(std::initializer_list<std::string> texts);
             
			void Clear();
			void RemoveAt(size_t index);
			void Erase(size_t logicalIndex);
			
			size_t GetCount() const { return m_items.size(); }
			bool IsEmpty() const { return m_items.empty(); }

			List::Item* At(size_t index);
             
			void Sort(size_t columnIndex, bool ascending);
			void ResetSort();
			void EnableImages(bool active);
             
			size_t GetLogicalIndex(size_t visualIndex) const;
			size_t GetVisualIndex(size_t logicalIndex) const;
			
			List::Item* GetItemSafely(size_t logicalIndex)
			{
				if (logicalIndex < m_items.size())
				{
					return &m_items[logicalIndex];
				}
				return nullptr;
			}

			void NotifyItemModified();
     
		private:
			void TriggerChanged();
     				
			bool m_drawImages { false };
			std::vector<List::Item> m_items;
			std::vector<size_t> m_visualMap;
			OnCollectionChangedCallback m_onChanged;
		};
		
		struct HeaderController
		{
			using OnHeaderClickedCallback = std::function<void(size_t visualColumnIndex)>;
			using OnHeadersReorderedCallback = std::function<void()>;
			using OnRequestColumnAutoWidth = std::function<uint32_t(size_t visualColumnIndex)>;

			struct ItemData
			{
				ItemData() = default;
				ItemData(const std::string& text, uint32_t width) :
					Text(text),
					Width(width)
				{
				}

				std::string Text;
				uint32_t Width;
			};

			HeaderController() = default;

			void Init(Window* owner);

			void Append(const std::string& name, uint32_t width);
			void Clear();
			void SetSortState(int visualColumnIndex, bool ascending);
			const std::vector<ItemData>& GetHeaders() const { return m_headers; }
			uint32_t GetTotalWidth() const;

			void Draw(Graphics& graphics, const Rectangle& visibleRect, int xOffset);

			bool OnDblClick(const ArgMouse& args, int scrollX);
			bool OnMouseDown(const ArgMouse& args, int scrollX);
			bool OnMouseMove(const ArgMouse& args, int scrollX);
			bool OnMouseUp(const ArgMouse& args, int scrollX);
			bool OnMouseLeave();

			bool IsResizing() const { return m_resizeInteraction.m_isResizing; }

			void SetTextPadding(int top, int bottom = 0, int left = 0, int right = 0);
			void SetOnHeaderClickedCallback(OnHeaderClickedCallback cb) { m_onHeaderClicked = cb; }
			void SetOnHeadersReorderedCallback(OnHeadersReorderedCallback cb) { m_onHeadersReordered = cb; }
			void SetOnRequestColumnAutoWidth(OnRequestColumnAutoWidth cb) { m_onRequestAutoWidth = cb; }
			
			size_t GetLogicalIndex(size_t visualIndex) const { return m_visualOrder[visualIndex]; }
			size_t GetColumnCount() const { return m_headers.size(); }
			
		private:
			uint32_t GetPositionToColumn(size_t visualIdx) const;
			void DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds, const Color& textColor);

			int GetVisualIndexAt(int mouseX, int scrollX) const;
			int GetDividerVisualIndexAt(int mouseX, int scrollX) const;
			
			Window* m_owner { nullptr };
			
			OnHeaderClickedCallback m_onHeaderClicked;
			OnHeadersReorderedCallback m_onHeadersReordered;
			OnRequestColumnAutoWidth m_onRequestAutoWidth;
			
			// --- Estados Visuales e Interacción ---
			int m_hoveredVisualIndex{ -1 };
			int m_sortLogicalIndex{ -1 };
			bool m_isSortAscending{ true };
			
			uint32_t m_startOffPos{ 4 };
			Padding m_textPadding;
			
			// Estado de Redimensionamiento
			struct ResizeState
			{
				bool m_isResizing{ false };
				int m_visualColumnIndex{ -1 };
				int m_startX{ 0 };
				uint32_t m_startWidth{ 0 };
				bool m_isHoveringDivider{ false };
			} m_resizeInteraction;
			
			// Estado de Arrastre (Drag & Drop)
			int m_draggedVisualIndex{ -1 };
			int m_dragStartX{ 0 };
			int m_currentMouseX{ 0 };
			bool m_isDraggingConfirmed{ false };
			
			std::vector<ItemData> m_headers;
			std::vector<size_t> m_visualOrder;
			Graphics m_draggingBox;
		};

		class Reactor : public ControlReactor
		{
		public:
			void Init(ControlBase& control, Graphics* graphics) override;
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

			/*struct Cell
			{
				Cell(const std::string& text) : m_text(text){}

				std::string m_text;
			};

			struct List
			{
				struct Item
				{
					Item() = default;
					Item(const std::string& text)
					{
						m_cells.emplace_back(text);
					}
					std::vector<Cell> m_cells;
					Rectangle m_bounds;
					bool m_isSelected{ false };
					Image m_icon;
					std::any m_userData;
				};

				std::vector<Item> m_items;
				std::vector<std::size_t> m_sortedIndexes;
				bool m_drawImages{ false };
			};*/
			

			struct Module
			{
				void AppendHeader(const std::string& text, uint32_t width);
				ListBoxItem Append(const std::string& text);
				ListBoxItem Append(std::initializer_list<std::string> texts);
				ListBoxItem At(size_t index);

				void Clear();
				void ClearHeaders();

				void Erase(ListBoxItem item);
				void Erase(std::vector<ListBoxItem>& items);
				void EnableMultiselection(bool enabled);
				void UpdateScrollData();
				
				bool EnsureVisibility(int lastSelectedIndex);

				std::vector<ListBoxItem> GetSelectedItems();

				void DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds, const Color& textColor);

				void DrawHeaders(Graphics& graphics);
				void DrawHeaderItem(Graphics& graphics, const Rectangle& rect, const std::string& name, bool isHovered, const Rectangle& textRect, const Color& textColor);
				void DrawList(Graphics& graphics);

				void SortHeader(size_t headerIndex, bool ascending);

				void InitScrollableView();

				void ProcessLassoIntersection();
				void UpdateSelectionRange(int startIndex, int endIndex);
				void SetItemSelected(size_t index, bool selected);
				
				void DrawRowBackground(Graphics& graphics, int visualIndex, const Rectangle& rowRect);
				void DrawRowContent(Graphics& graphics, int visualRowIndex, const Rectangle& rect);
				void DrawCell(Graphics& graphics, const Rectangle& rect, const std::string& text, bool isRowSelected);
    
				bool IsSelected(int index) const;
				
				HeaderController m_headers;
				ItemCollection m_items;
				
				std::unique_ptr<ScrollableView> m_scrollableView;
				LassoSelection m_lassoSelection;
				SelectionController<size_t> m_selectionController;
				std::unordered_set<int> m_preLassoSelection;
				
				bool m_isSortAscending {false};
				std::optional<size_t> m_currentSortColumn = std::nullopt;
				int m_hoveredIndex{-1};
				bool m_drawImages {false};
				Window* m_window{ nullptr };
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
			ListBoxItem(size_t logicalIndex, ItemCollection* itemCollection) :
				m_logicalIndex(logicalIndex), m_collection(itemCollection)
			{
			}

			void SetIcon(const Image& image) const;
			void SetText(size_t columnIndex, const std::string& text) const;
			std::string GetText(size_t columnIndex) const;

			template<typename T>
			bool HasUserData() const
			{
				return std::any_cast<T>(&UserData());
			}

			template<typename T>
			const T& GetUserData() const
			{
				auto p = std::any_cast<T>(&UserData());
				return *p;
			}

			template<typename T>
			ListBoxItem& SetUserData(T&& userData)
			{
				UserData() = std::forward<T>(userData);
				return *this;
			}

			explicit operator bool() const
			{
				return m_collection;
			}

			friend struct Reactor::Module;
		private:
			std::any& UserData();
			const std::any& UserData() const;

			size_t m_logicalIndex{ static_cast<size_t>(-1) };
			ItemCollection* m_collection{ nullptr };
		};
	}

	struct ArgListBox
	{
		size_t SelectedIndex{ 0 };
	};

	namespace ReactorCore::ListBox
	{
		struct Events : public ControlEvents
		{
			Event<ArgListBox> Selected;
		};
	}
	
	class ListBox : public Control<ReactorCore::ListBox::Reactor, ReactorCore::ListBox::Events, ReactorCore::ListBox::Appearance>
	{
	public:
		using ListBoxItem = ReactorCore::ListBox::ListBoxItem;
		
	public:
		ListBox() = default;
		ListBox(Window* parent, const Rectangle& rectangle = {});

		void AppendHeader(const std::string& name, uint32_t width = 120);
		ListBoxItem Append(const std::string& text);
		ListBoxItem Append(std::initializer_list<std::string> texts);
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
