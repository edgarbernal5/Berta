/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Button.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	namespace Internal::Button
	{
		void Reactor::Update(Graphics& graphics)
		{
			auto window = m_control->Handle();
			bool enabled = m_control->GetEnabled();
			auto backgroundRect = window->ClientSize.ToRectangle();
			auto buttonAppearance = static_cast<Buttons::Appearance*>(window->Appearance.get());
			
			auto color = window->Appearance->Background;
			if (!enabled) color = window->Appearance->ButtonDisabledBackground;
			else if (m_module.m_status == State::Normal) color = window->Appearance->ButtonBackground;
			else if (m_module.m_status == State::Hovered) color = window->Appearance->ButtonHighlightBackground;
			else if (m_module.m_status == State::Pressed) color = window->Appearance->ButtonPressedBackground;
			
			graphics.DrawRoundRectBox(backgroundRect, color, enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, true);

			auto caption = m_control->GetCaption();
			bool hasText = !caption.empty();
			bool hasIcon = m_module.Icon.has_value();

			if (!hasText && !hasIcon)
			{
				return;
			}
			
			const Size windowSize = window->ClientSize;
			Point textExtent{ 0, 0 };
			if (hasText)
			{
				textExtent = graphics.GetTextExtent(caption);
			}

			int iconSize = hasIcon ? static_cast<int>(window->ToScale(window->Appearance->SmallIconSize)) : 0;
			int iconSpacing = (hasIcon && hasText) ? static_cast<int>(window->ToScale(buttonAppearance->IconTextSpacing)) : 0;

			int totalContentWidth = iconSize + iconSpacing + textExtent.X;
			int totalContentHeight = std::max<int>(iconSize, textExtent.Y);

			int currentX = 0;
			switch (m_module.ContentHAlign)
			{
			case HorizontalAlign::Center:
				currentX = (static_cast<int>(windowSize.Width) - totalContentWidth) / 2;
				break;
			case HorizontalAlign::Left:
				currentX = window->ToScale(5);
				break;
			case HorizontalAlign::Right:
				currentX = static_cast<int>(windowSize.Width) - totalContentWidth - window->ToScale(5);
				break;
			}
			
			int contentY = 0;
			switch (m_module.ContentVAlign)
			{
			case VerticalAlign::Center:
				contentY = (static_cast<int>(windowSize.Height) - totalContentHeight) / 2;
				break;
			case VerticalAlign::Top:
				contentY = window->ToScale(5);
				break;
			case VerticalAlign::Bottom:
				contentY = static_cast<int>(windowSize.Height) - totalContentHeight - window->ToScale(5);
				break;
			}

			if (hasIcon)
			{
				int iconY = contentY + (totalContentHeight - iconSize) / 2;
				m_module.Icon->Paste(graphics, { currentX, iconY, static_cast<uint32_t>(iconSize), static_cast<uint32_t>(iconSize) });
				
				currentX += iconSize + iconSpacing;
			}

			if (hasText)
			{
				int textY = contentY + (totalContentHeight - textExtent.Y) / 2;
				graphics.DrawString({ currentX, textY }, caption, enabled ? window->Appearance->Foreground : window->Appearance->BoxBorderDisabledColor);
			}
		}

		void Reactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
		{
			m_module.m_status = State::Hovered;

			GUI::MarkAsNeedUpdate(*m_control);
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			m_module.m_status = State::Normal;

			GUI::MarkAsNeedUpdate(*m_control);
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			m_module.m_status = State::Pressed;

			GUI::Capture(*m_control);
			GUI::MarkAsNeedUpdate(*m_control);
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			GUI::ReleaseCapture(*m_control);
			
			if (m_control->Handle()->ClientSize.IsInside(args.Position))
			{
				m_module.m_status = State::Hovered;
			}
			else
			{
				m_module.m_status = State::Normal;
			}

			GUI::MarkAsNeedUpdate(*m_control);
		}

		void Reactor::Module::SetContentHorizontalAlign(HorizontalAlign halign)
		{
			ContentHAlign = halign;
		}

		void Reactor::Module::SetContentVerticalAlign(VerticalAlign valign)
		{
			ContentVAlign = valign;
		}

		void Reactor::Module::SetIcon(const Image& icon)
		{
			Icon = icon;
		}
	}
	
	Button::Button(Window* parent, const Rectangle& rectangle, const std::string& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "Button";
#endif
	}

	Button::Button(Window* parent, const Rectangle& rectangle, const std::wstring& text)
	{
		Create(parent, true, rectangle);
		SetCaption(text);

#if BT_DEBUG
		m_handle->Name = "Button";
#endif
	}

	Button::Button(Window* parent, const Rectangle& rectangle, const std::string& text, const Image& icon)
	{
		Create(parent, true, rectangle);
		SetCaption(text);
		SetIcon(icon);

#if BT_DEBUG
		m_handle->Name = "Button";
#endif
	}

	Button::Button(Window* parent, const Rectangle& rectangle, const std::wstring& text, const Image& icon)
	{
		Create(parent, true, rectangle);
		SetCaption(text);
		SetIcon(icon);

#if BT_DEBUG
		m_handle->Name = "Button";
#endif
	}

	Button::Button(Window* parent, const Rectangle& rectangle, const Berta::Image& icon)
	{
		Create(parent, true, rectangle);
		SetIcon(icon);

#if BT_DEBUG
		m_handle->Name = "Button";
#endif
	}

	void Button::SetContentHorizontalAlign(const HorizontalAlign& halign)
	{
		GetReactor().GetModule().SetContentHorizontalAlign(halign);
	}

	void Button::SetContentVerticalAlign(const VerticalAlign& valign)
	{
		GetReactor().GetModule().SetContentVerticalAlign(valign);
	}

	void Button::SetIcon(const Image& icon)
	{
		GetReactor().GetModule().SetIcon(icon);
	}
}
