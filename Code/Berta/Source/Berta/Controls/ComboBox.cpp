/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ComboBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/Caret.h"
#include "Berta/GUI/EnumTypes.h"
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

		auto buttonSize = static_cast<uint32_t>(20 * window->DPIScaleFactor);

		graphics.DrawRectangle({ static_cast<int>(window->Size.Width - buttonSize), 1, buttonSize, window->Size.Height - 2 }, window->Appereance->Background, true);

		int arrowWidth = static_cast<int>(6 * window->DPIScaleFactor);
		int arrowLength = static_cast<int>(3 * window->DPIScaleFactor);
		graphics.DrawArrow({ static_cast<int>(window->Size.Width - buttonSize) , 1, buttonSize, window->Size.Height }, arrowLength, arrowWidth, window->Appereance->BoxBorderColor, Graphics::ArrowDirection::Downwards, true);

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
			auto pointInScreen = GUI::GetPointClientToScreen(window, m_control->Handle()->Position);

			auto clampedItemsToShow = (std::min)(m_interactionData.m_items.size(), m_interactionData.m_maxItemsToDisplay);
			auto floatBoxHeight = static_cast<uint32_t>(clampedItemsToShow * window->Appereance->ComboBoxItemHeight * window->DPIScaleFactor);
			m_floatBox = new FloatBox(window, { pointInScreen.X, pointInScreen.Y + (int)window->Size.Height, window->Size.Width, floatBoxHeight + 2u });
			m_floatBox->Init(m_interactionData);

			m_floatBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
			{
				int selectedIndex = m_floatBox->GetState().m_index;

				delete m_floatBox;
				m_floatBox = nullptr;

				if (m_interactionData.m_isSelected && selectedIndex != m_interactionData.m_selectedIndex && !m_interactionData.m_items.empty())
				{
					m_interactionData.m_selectedIndex = selectedIndex;
					m_text = m_interactionData.m_items[m_interactionData.m_selectedIndex];

					EmitSelectionEvent(selectedIndex);
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
			if (args.Key == KeyboardKey::ArrowUp)
			{
				if (m_floatBox->MoveSelectedItem(-1))
				{
					redraw = true;
				}
			}

			if (args.Key == KeyboardKey::ArrowDown)
			{
				if (m_floatBox->MoveSelectedItem(1))
				{
					redraw = true;
				}
			}
			if (args.Key == KeyboardKey::Enter)
			{
				if (m_floatBox->GetState().m_index >= 0 && m_floatBox->GetState().m_index < m_interactionData.m_items.size())
				{
					m_interactionData.m_selectedIndex = m_floatBox->GetState().m_index;
					m_text = m_interactionData.m_items[m_interactionData.m_selectedIndex];
					m_floatBox->Dispose();
					EmitSelectionEvent(m_interactionData.m_selectedIndex);
					redraw = true;
				}
				
			}
		}
		else
		{
			if (args.Key == KeyboardKey::ArrowUp)
			{
				int newIndex = (std::max)(0, (std::min)(m_interactionData.m_selectedIndex - 1, (int)(m_interactionData.m_items.size()) - 1));
				if (m_interactionData.m_selectedIndex != newIndex && !m_interactionData.m_items.empty()) {
					m_interactionData.m_selectedIndex = newIndex;
					m_text = m_interactionData.m_items[newIndex];
					EmitSelectionEvent(newIndex);
					redraw = true;
				}
			}
			if (args.Key == KeyboardKey::ArrowDown)
			{
				int newIndex = (std::max)(0, (std::min)(m_interactionData.m_selectedIndex + 1, (int)(m_interactionData.m_items.size()) - 1));
				if (m_interactionData.m_selectedIndex != newIndex && !m_interactionData.m_items.empty()) {
					m_interactionData.m_selectedIndex = newIndex;
					m_text = m_interactionData.m_items[newIndex];
					EmitSelectionEvent(newIndex);
					redraw = true;
				}
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


	void ComboBoxReactor::EmitSelectionEvent(int index)
	{
		ArgComboBox argComboBox;
		argComboBox.SelectedIndex = index;

		auto events = dynamic_cast<ComboboxEvents*>(m_control->Handle()->Events.get());
		events->Selected.Emit(argComboBox);
	}

	ComboBox::ComboBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle);
	}

	void ComboBox::Clear()
	{
		m_reactor.GetInteractionData().m_items.clear();
		m_reactor.GetInteractionData().m_selectedIndex = -1;
	}

	void ComboBox::Erase(uint32_t index)
	{
		if (index < m_reactor.GetInteractionData().m_items.size())
		{
			auto& items = m_reactor.GetInteractionData().m_items;
			auto& selectedIndex = m_reactor.GetInteractionData().m_selectedIndex;
			items.erase(items.begin() + index);
			if (selectedIndex >= static_cast<int>(items.size()))
			{
				selectedIndex = static_cast<int>(items.size()) - 1;

				m_reactor.SetText(items[selectedIndex]);
			}
		}
	}

	void ComboBox::SetSelectedIndex(uint32_t index)
	{
		if (index < m_reactor.GetInteractionData().m_items.size())
		{
			auto& items = m_reactor.GetInteractionData().m_items;
			auto& selectedIndex = m_reactor.GetInteractionData().m_selectedIndex;
			selectedIndex = static_cast<int>(index);

			m_reactor.SetText(items[selectedIndex]);
		}
	}

	void ComboBox::PushItem(const std::wstring& text)
	{
		m_reactor.GetInteractionData().m_items.push_back(text);
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