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
	class ThumbListBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyReleased(Graphics& graphics, const ArgKeyboard& args) override;

		struct Module
		{
			struct ItemType
			{
				ItemType() = default;

				std::wstring Text;
				Image Thumbnail;
				bool IsSelected{ false };
				Rectangle PosSize;
			};

			struct GridCardType
			{
				Rectangle PosSize{};
			};

			struct State
			{
				int m_offset{ 0 };
			};

			void AddItem(const std::wstring& text, const Image& thumbnail);
			void BuildGridCards();
			void BuildItems();
			void Clear();
			void SetThumbnailSize(uint32_t size);
			void UpdateScrollBar();
			void OnMouseDown(const ArgMouse& args);
			void EnableMultiselection(bool enabled);
			int GetItemIndexAtMousePosition(const Point& position);

			std::vector<size_t> GetSelectedItems() const;

			std::vector<ItemType> Items;
			uint32_t ThumbnailSize{ 96u };
			std::unique_ptr<ScrollBar> m_scrollBar;
			ThumbListBoxAppearance* Appearance{ nullptr };
			State m_state;
			Window* m_window{ nullptr };
			Point m_mouseDownPosition;
			bool m_multiselection{ true };

			struct Selection
			{
				std::vector<size_t> m_indexes;
				int m_pressedIndex{ -1 };
				int m_selectedIndex{ -1 };
				int m_pivotIndex{ -1 };
				Point m_startPosition;
			};
			bool m_shiftPressed = false;
			bool m_ctrlPressed = false;
			Selection m_selection;
			std::vector<GridCardType> m_gridCards;
		private:
			bool NeedsScrollBar() const;

		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	class ThumbListBox : public Control<ThumbListBoxReactor, ThumbListBoxEvents, ThumbListBoxAppearance>
	{
	public:
		ThumbListBox() = default;
		ThumbListBox(Window* parent, const Rectangle& rectangle);

		void AddItem(const std::wstring& text, const Image& thumbnail);
		void Clear();
		void SetThumbnailSize(uint32_t size);

		void EnableMultiselection(bool enabled);

		std::vector<size_t> GetSelected() const;
	};
}

#endif
