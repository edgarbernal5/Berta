/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "MenuBar.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void MenuBarReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void MenuBarReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();

		bool enabled = m_control->GetEnabled();
		
		if (!enabled)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ButtonDisabledBackground, true);
		}
		else if (m_status == State::Normal)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ButtonBackground, true);
		}
		auto& items = m_menuCollectionData.m_items;
		auto itemMargin = static_cast<uint32_t>(4u * window->DPIScaleFactor);
		Point offset{ 0, (int)itemMargin };
		for (size_t i = 0; i < items.size(); i++)
		{
			auto& itemData = m_menuCollectionData.m_items[i];

			auto textSize = graphics.GetTextExtent(itemData.name);
			Size itemSize{ textSize.Width + itemMargin * 2, window->Size.Height - itemMargin * 2 };

			auto center = itemSize - textSize;
			center = center * 0.5f;

			if (m_menuCollectionData.m_hoveredItemIndex == i)
			{
				graphics.DrawString({ offset.X + (int)center.Width , offset.Y + (int)center.Height }, itemData.name, window->Appereance->Foreground);
			}
			else
			{
				graphics.DrawString({ offset.X + (int)center.Width , offset.Y + (int)center.Height }, itemData.name, enabled ? window->Appereance->Foreground : window->Appereance->BoxBorderDisabledColor);
			}
			offset.X += (int)itemSize.Width;
		}

		/*else if (m_status == State::Hovered)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ButtonHighlightBackground, true);
		}
		else if (m_status == State::Pressed)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->ButtonPressedBackground, true);
		}
		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);

		auto caption = m_control->GetCaption();
		auto center = window->Size - graphics.GetTextExtent(caption);
		center = center * 0.5f;
		graphics.DrawString({ (int)center.Width,(int)center.Height }, caption, enabled ? window->Appereance->Foreground : window->Appereance->BoxBorderDisabledColor);*/
	}

	void MenuBarReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		/*m_status = State::Hovered;
		Update(graphics);
		GUI::UpdateDeferred(*m_control);*/
	}

	void MenuBarReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		/*m_status = State::Normal;
		Update(graphics);
		GUI::UpdateDeferred(*m_control);*/
	}

	void MenuBarReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		/*m_status = State::Pressed;
		Update(graphics);
		GUI::UpdateDeferred(*m_control);

		GUI::Capture(*m_control);*/
	}

	void MenuBarReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{

		Update(graphics);
		GUI::UpdateDeferred(*m_control);
	}

	void MenuBarReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		//if (m_control->Handle()->Size.IsInside(args.Position))
		//{
		//	m_status = State::Hovered;
		//}
		//else
		//{
		//	m_status = State::Normal;
		//}

		//Update(graphics);
		//GUI::UpdateDeferred(*m_control);
		//GUI::ReleaseCapture(*m_control);
	}

	MenuBar::MenuBar(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	void MenuBar::PushBack(const std::wstring& itemName)
	{
		m_reactor.GetCollectionData().m_items.emplace_back(MenuBarReactor::MenuItemData{ itemName });
	}
}