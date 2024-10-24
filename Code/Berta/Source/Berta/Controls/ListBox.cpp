/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ListBox.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void ListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_window = control.Handle();

		m_appearance = reinterpret_cast<ListBoxAppearance*>(control.Handle()->Appearance.get());
	}

	void ListBoxReactor::Update(Graphics& graphics)
	{
		auto enabled = m_control->GetEnabled();
		graphics.DrawRectangle(m_window->Size.ToRectangle(), m_window->Appearance->BoxBackground, true);

		Rectangle backgroundRect;
		CalculateViewport(backgroundRect);

		auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
		graphics.DrawRectangle({ 0,0, m_window->Size.Width, headerHeight }, m_appearance->ButtonBackground, true);
		graphics.DrawLine({ backgroundRect.X, (int)headerHeight }, { (int)m_window->Size.Width - 1, (int)headerHeight }, m_appearance->HighlightBorderColor);
		for (size_t i = 0; i < m_headers.Items.size(); i++)
		{
			const auto& header = m_headers.Items;
		}

		graphics.DrawRectangle(m_window->Size.ToRectangle(), enabled ? m_window->Appearance->BoxBorderColor : m_window->Appearance->BoxBorderDisabledColor, false);
	}

	void ListBoxReactor::CalculateViewport(Rectangle& backgroundRect)
	{
		backgroundRect = m_window->Size.ToRectangle();
		backgroundRect.Y = backgroundRect.X = 1;
		backgroundRect.Width -= 2u;
		backgroundRect.Height -= 2u;
	}

	void ListBoxReactor::Headers::Append(const std::string& text, uint32_t width)
	{
		Items.emplace_back(text, width);
	}

	ListBox::ListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ListBox";
#endif
	}

	void ListBox::AppendHeader(const std::string& name, uint32_t width)
	{
		m_reactor.GetHeaders().Append(name, width);
	}

	void ListBox::Append(const std::string& text)
	{
	}

	void ListBox::Append(std::initializer_list<std::string> texts)
	{
	}

	void ListBox::Clear()
	{
	}

	void ListBox::ClearHeaders()
	{
	}
}
