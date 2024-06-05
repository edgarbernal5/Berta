/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ComboBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/Caret.h"
#include "Berta/Controls/TextEditors/TextEditor.h"
#include "Berta/Controls/Floating/FloatBox.h"

namespace Berta
{
	ComboBoxReactor::~ComboBoxReactor()
	{
		if (m_textEditor)
		{
			delete m_textEditor;
			m_textEditor = nullptr;
		}

		if (m_floatBox)
		{
			delete m_floatBox;
			m_floatBox = nullptr;
		}
	}

	void ComboBoxReactor::Init(ControlBase& control)
	{
		m_control = reinterpret_cast<ComboBox*>(&control);
		m_textEditor = new TextEditor(*m_control);

		auto window = m_control->Handle();
		window->Events->Focus.Connect([&](const ArgFocus& args) {
			if (!args.Focused && m_floatBox)
			{
				m_floatBox->Dispose();
			}
		});
	}

	void ComboBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		if (m_status == State::Normal)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);
		}
		else if (m_status == State::Hovered)
		{
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxHightlightBackground, true);
		}

		//m_textEditor->Render();
		auto textItemHeight = graphics.GetTextExtent().Height;
		graphics.DrawString({ 3,static_cast<int>(window->Size.Height - textItemHeight) >> 1 }, m_text, window->Appereance->Foreground);

		auto buttonSize = static_cast<uint32_t>(24 * window->DPIScaleFactor);

		graphics.DrawRectangle({ static_cast<int>(window->Size.Width - buttonSize), 1, buttonSize, window->Size.Height - 2 }, window->Appereance->Background, true);

		int arrowWidth = static_cast<int>(6 * window->DPIScaleFactor);
		int arrowLength = static_cast<int>(3 * window->DPIScaleFactor);
		graphics.DrawArrow({ static_cast<int>(window->Size.Width - buttonSize) , 1, buttonSize, window->Size.Height }, arrowLength, arrowWidth, window->Appereance->BoxBorderColor, true);

		graphics.DrawLine({ static_cast<int>(window->Size.Width - buttonSize), 1 }, { static_cast<int>(window->Size.Width - buttonSize), (int)window->Size.Height - 1 }, window->Appereance->BoxBorderColor);
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBorderColor, false);
	}

	void ComboBoxReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Hovered;
		Update(graphics);
		GUI::UpdateDeferred(*m_control);
	}

	void ComboBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_status = State::Normal;
		Update(graphics);
		GUI::UpdateDeferred(*m_control);
	}

	void ComboBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (args.ButtonState.LeftButton)
		{
			auto window = m_control->Handle();
			auto point = GUI::GetPointClientToScreen(window, m_control->Handle()->Position);

			auto floatBoxHeight = static_cast<uint32_t>(m_selectionState.m_items.size() * window->Appereance->ComboBoxItemHeight * window->DPIScaleFactor);
			m_floatBox = new FloatBox(window, { point.X,point.Y + (int)window->Size.Height,window->Size.Width,floatBoxHeight + 2 });
			m_floatBox->Init(m_selectionState);

			m_floatBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
			{
				int selectedIndex = m_floatBox->GetState().m_index;

				delete m_floatBox;
				m_floatBox = nullptr;

				if (m_selectionState.m_isSelected && selectedIndex != m_selectionState.m_selectedIndex)
				{
					m_selectionState.m_selectedIndex = selectedIndex;
					m_text = m_selectionState.m_items[m_selectionState.m_selectedIndex];
					
					auto window = m_control->Handle();
					window->Renderer.Update();
					GUI::UpdateDeferred(window);
				}
			});

			GUI::Capture(m_floatBox->Handle());
			m_floatBox->Show();
		}
	}

	void ComboBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ComboBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ComboBoxReactor::Focus(Graphics& graphics, const ArgFocus& args)
	{
	}

	void ComboBoxReactor::KeyChar(Graphics& graphics, const ArgKeyboard& args)
	{

	}

	void ComboBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		bool redraw = false;
		if (m_floatBox)
		{
			if (args.Key == VK_UP)
			{
				m_floatBox->MoveSelectedItem(-1);
				redraw = true;
			}

			if (args.Key == VK_DOWN)
			{
				m_floatBox->MoveSelectedItem(1);
				redraw = true;
			}
		}
		if (redraw)
		{
			auto window = m_control->Handle();
			window->Renderer.Update();
			GUI::UpdateDeferred(window);
		}
	}

	std::wstring ComboBoxReactor::GetText() const
	{
		return m_text;
	}

	void ComboBoxReactor::SetText(const std::wstring& text)
	{
		m_text = text;
		auto window = m_control->Handle();
		window->Renderer.Update();
		GUI::UpdateDeferred(window);
	}

	ComboBox::ComboBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle);
	}

	void ComboBox::PushItem(const std::wstring& text)
	{
		m_reactor.GetSelectionState().m_items.push_back(text);
	}

	void ComboBox::DoOnCaption(const std::wstring& caption)
	{
		m_reactor.SetText(caption);
	}

	std::wstring ComboBox::DoOnCaption()
	{
		return m_reactor.GetText();
	}
}