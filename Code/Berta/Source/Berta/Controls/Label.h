/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LABEL_HEADER
#define BT_LABEL_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>

namespace Berta
{
	class LabelReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control, Graphics* graphics) override;
		void Update(Graphics& graphics) override;

		struct Module
		{
			void Update();
			
			bool m_isWordWrap{ false };
			HorizontalAlign m_horizontalAlign{ HorizontalAlign::Left };
			VerticalAlign m_verticalAlignment{ VerticalAlign::Top };
			Window* m_owner{ nullptr };
		};
		
		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }
		
	private:
		Module m_module;
	};

	class Label : public Control<LabelReactor>
	{
	public:
		Label() = default;
		Label(Window* parent, const Rectangle& rectangle, const std::wstring& text);
		Label(Window* parent, const Rectangle& rectangle, const std::string& text);
		
		bool IsWordWrap() const;
		void SetWordWrap(bool wordWrap);
		
		void SetHorizontalAlignment(HorizontalAlign horizontalAlignment);
		void SetVerticalAlignment(VerticalAlign verticalAlignment);
	};
}

#endif