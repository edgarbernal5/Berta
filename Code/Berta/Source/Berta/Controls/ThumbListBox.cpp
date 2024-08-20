/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ThumbListBox.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	void ThumbListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);

		for (size_t i = 0; i < m_module.Items.size(); i++)
		{

		}

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& newItem = Items.emplace_back();
		newItem.Text = text;
		newItem.Thumbnail = thumbnail;
	}

	void ThumbListBoxReactor::Module::Clear()
	{
	}

	void ThumbListBoxReactor::Module::SetThumbnailSize(uint32_t size)
	{
		ThumbnailSize = size;
	}

	void ThumbListBoxReactor::Module::BuildItems()
	{
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	void ThumbListBox::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		m_reactor.GetModule().AddItem(text, thumbnail);
	}

	void ThumbListBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	void ThumbListBox::SetThumbnailSize(uint32_t size)
	{
		m_reactor.GetModule().SetThumbnailSize(size);
	}
}
