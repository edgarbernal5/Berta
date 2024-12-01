/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_THUMB_LIST_BOX_HEADER
#define BT_THUMB_LIST_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/ScrollBar.h"
#include "Berta/Paint/Image.h"

#include <string>
#include <vector>

namespace Berta
{
	struct ThumbListBoxItem;

	struct ThumbListBoxAppearance : public ControlAppearance
	{
		uint32_t ThumbnailCardHeight = 45;
	};

	class ThumbListBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyReleased(Graphics& graphics, const ArgKeyboard& args) override;

		struct Module
		{
			struct ItemType
			{
				ItemType() = default;

				std::wstring m_text;
				Image m_thumbnail;
				bool m_isSelected{ false };
				Rectangle m_bounds;
			};

			struct GridCardType
			{
				Rectangle m_positionSize{};
			};

			struct State
			{
				int m_offset{ 0 };
			};

			struct ViewportData
			{
				Rectangle m_backgroundRect{};
				uint32_t m_totalRows{ 0 };
				uint32_t m_totalCardsInRow{ 0 };
				Size m_cardSize{};
				Size m_cardSizeWithMargin{};
				int m_contentSize{ 0 };
				uint32_t m_innerMargin{ 0 };
				uint32_t m_cardMargin{ 0 };
				uint32_t m_cardMarginHalf{ 0 };

				int m_startingVisibleIndex{ -1 };
				int m_endingVisibleIndex{ -1 };
			};

			struct MouseSelection
			{
				bool IsAlreadySelected(size_t index) const;
				bool IsSelected(size_t index) const;
				void Select(size_t index);
				void Deselect(size_t index);

				std::vector<size_t> m_selections; //TODO: cambiar a set/map
				std::vector<size_t> m_alreadySelected;  //TODO: cambiar a set/map
				int m_pressedIndex{ -1 };
				int m_selectedIndex{ -1 };
				int m_pivotIndex{ -1 };
				Point m_startPosition;
				Point m_endPosition;
				bool m_started{ false };
				bool m_inverseSelection{ false };
			};

			void AddItem(const std::wstring& text, const Image& thumbnail);
			ThumbListBoxItem At(size_t index);
			void CalculateViewport(ViewportData& viewportData);
			void CalculateVisibleIndices();
			void CalculateSelectionBox(Point& startPoint, Point& endPoint, Size& boxSize);
			void BuildItems();
			void Clear();
			void Erase(size_t index);
			void SetThumbnailSize(uint32_t size);
			void UpdateScrollBar();
			void EnableMultiselection(bool enabled);
			int GetItemIndexAtMousePosition(const Point& position);
			void ClearSelection();
			bool ClearSingleSelection();
			bool UpdateSingleSelection(int newItemIndex);
			void SelectItem(int index);
			void StartSelectionRectangle(const Point& mousePosition);
			bool ClearSelectionIfNeeded();
			void ToggleItemSelection(int itemIndexAtPosition);
			void PerformRangeSelection(int itemIndexAtPosition);
			bool HandleMultiSelection(int itemIndexAtPosition, const ArgMouse& args);

			std::vector<size_t> GetSelectedItems() const;
			bool EnsureVisibility(int lastSelectedIndex);

			void UpdatedThumbnail(ItemType& item);

			std::vector<ItemType> m_items;
			uint32_t m_thumbnailSize{ 96u };
			std::unique_ptr<ScrollBar> m_scrollBar;
			ThumbListBoxAppearance* m_appearance{ nullptr };
			State m_state;
			Window* m_window{ nullptr };
			ControlBase* m_control{ nullptr };
			bool m_multiselection{ true };
			bool m_shiftPressed{ false };
			bool m_ctrlPressed{ false };
			MouseSelection m_mouseSelection;
			std::vector<GridCardType> m_gridCards;
			ViewportData m_viewport;
		private:

		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	struct ThumbListBoxItem
	{
		ThumbListBoxItem(ThumbListBoxReactor::Module::ItemType& target, ThumbListBoxReactor::Module& module) :
			m_target(target), m_module(module)
		{
		}

		void SetIcon(const Image& image);
	private:
		ThumbListBoxReactor::Module::ItemType& m_target;
		ThumbListBoxReactor::Module& m_module;
	};

	struct ArgThumbListBox
	{
		size_t SelectedIndex{ 0 };
	};

	struct ArgThumbListBoxItemVisibility
	{
		size_t SelectedIndex{ 0 };
		bool Visible{ false };
	};

	struct ThumbListBoxEvents : public ControlEvents
	{
		Event<ArgThumbListBox>	Selected;
		Event<ArgThumbListBoxItemVisibility>	ItemVisibility;
	};

	class ThumbListBox : public Control<ThumbListBoxReactor, ThumbListBoxEvents, ThumbListBoxAppearance>
	{
	public:
		ThumbListBox() = default;
		ThumbListBox(Window* parent, const Rectangle& rectangle);

		void AddItem(const std::wstring& text, const Image& thumbnail);
		ThumbListBoxItem At(size_t index);
		void Clear();
		void Erase(size_t index);
		void SetThumbnailSize(uint32_t size);

		void EnableMultiselection(bool enabled);

		std::vector<size_t> GetSelected() const;
	};
}

#endif
