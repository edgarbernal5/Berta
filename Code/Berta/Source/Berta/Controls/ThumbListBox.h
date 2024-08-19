/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_THUMB_LIST_BOX_HEADER
#define BT_THUMB_LIST_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
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

		struct Module;
		Module& GetModule() { return m_module; }
	private:
		struct Module
		{
			struct ItemType
			{
				std::wstring Text;
				Image Image;
			};

			void AddItem(const std::wstring& text, const Image& thumbnail);
			void Clear();

			std::vector<ItemType> Items;
		};

		Module m_module;
	};

	class ThumbListBox : public Control<ThumbListBoxReactor>
	{
	public:
		ThumbListBox() = default;
		ThumbListBox(Window* parent, const Rectangle& rectangle);

		void AddItem(const std::wstring& text, const Image& thumbnail);
		void Clear();
	};
}

#endif
