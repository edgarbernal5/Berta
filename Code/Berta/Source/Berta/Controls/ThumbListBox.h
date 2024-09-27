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
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;

		struct Module
		{
			struct ItemType
			{
				ItemType() = default;

				std::wstring Text;
				Image Thumbnail;
			};

			struct State
			{
				int m_offset{ 0 };
			};

			void AddItem(const std::wstring& text, const Image& thumbnail);
			void Clear();
			void SetThumbnailSize(uint32_t size);
			void UpdateScrollBar();

			std::vector<ItemType> Items;
			uint32_t ThumbnailSize{ 96u };
			std::unique_ptr<ScrollBar> m_scrollBar;
			ThumbListBoxAppearance* Appearance{ nullptr };
			State m_state;
			Window* m_window{ nullptr };
		private:
			void BuildItems();
			bool NeedsScrollBar() const;

		};
		Module& GetModule() { return m_module; }

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
	};
}

#endif
