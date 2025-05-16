/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ComboBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/Caret.h"
#include "Berta/Paint/DrawBatch.h"
#include "Berta/GUI/EnumTypes.h"
#include "Berta/Controls/TextEditors/TextEditor.h"
#include "Berta/Controls/Floating/FloatBox.h"

namespace Berta
{
	ComboBoxReactor::~ComboBoxReactor()
	{
		if (m_module.m_textEditor)
		{
			delete m_module.m_textEditor;
			m_module.m_textEditor = nullptr;
		}

		if (m_module.m_floatBox)
		{
			delete m_module.m_floatBox;
			m_module.m_floatBox = nullptr;
		}
	}

	void ComboBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.m_comboBox = reinterpret_cast<ComboBox*>(&control);
		m_module.m_textEditor = new TextEditor(*m_control);

		auto window = m_control->Handle();
		window->Events->Focus.Connect([&](const ArgFocus& args)
		{
			if (!args.Focused && m_module.m_floatBox)
			{
				m_module.m_floatBox->Dispose();
			}
		});
		m_module.m_owner = window;
	}

	void ComboBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		auto backgroundRect = window->ClientSize.ToRectangle();

		if (m_module.m_status == State::Normal)
		{
			graphics.DrawRectangle(backgroundRect, window->Appearance->BoxBackground, true);
		}
		else if (m_module.m_status == State::Hovered)
		{
			graphics.DrawRectangle(backgroundRect, window->Appearance->BoxHightlightBackground, true);
		}

		//m_textEditor->Render();

		auto textItemHeight = graphics.GetTextExtent().Height;
		Point textPosition{ 3,static_cast<int>(window->ClientSize.Height - textItemHeight) >> 1 };
		if (m_module.Data.m_drawImages && m_module.Data.m_selectedIndex != -1)
		{
			auto iconSize = window->ToScale(window->Appearance->SmallIconSize);
			auto iconMargin = window->ToScale(3u);
			textPosition.X += (int)(iconSize + iconMargin * 2u);

			auto& icon = m_module.Data.m_items[m_module.Data.m_selectedIndex].m_icon;
			if (icon)
			{
				auto iconSourceSize = icon.GetSize();
				auto positionY = (window->ClientSize.Height - iconSize) >> 1;
				icon.Paste(graphics, { 3, (int)positionY, iconSize , iconSize });
			}
		}
		graphics.DrawString(textPosition, m_module.m_text, enabled ? window->Appearance->Foreground : window->Appearance->BoxBorderDisabledColor);

		auto buttonSize = window->ToScale(m_module.m_comboBox->GetAppearance().ButtonSize);

		graphics.DrawRectangle({ static_cast<int>(window->ClientSize.Width - buttonSize - 1), 1, buttonSize, window->ClientSize.Height - 2 }, window->Appearance->Background, true);

		int arrowWidth = window->ToScale(4);
		int arrowLength = window->ToScale(2);
		graphics.DrawArrow({ static_cast<int>(window->ClientSize.Width - buttonSize) - 1, 1, buttonSize, window->ClientSize.Height },
			arrowLength, 
			arrowWidth, 
			Graphics::ArrowDirection::Downwards,
			enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor,
			true,
			enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor);

		graphics.DrawLine({ static_cast<int>(window->ClientSize.Width - buttonSize) - 1, 1 },
			{ static_cast<int>(window->ClientSize.Width - buttonSize) - 1, (int)window->ClientSize.Height - 1 },
			enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor);

		graphics.DrawRectangle(backgroundRect, enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void ComboBoxReactor::MouseEnter(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_status = State::Hovered;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	void ComboBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_status = State::Normal;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	void ComboBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (args.ButtonState.LeftButton)
		{
			auto window = m_module.m_owner;
			auto pointInScreen = GUI::GetAbsoluteRootPosition(window);

			auto clampedItemsToShow = static_cast<uint32_t>((std::min)(m_module.Data.m_items.size(), m_module.Data.m_maxItemsToDisplay));
			auto floatBoxHeight = window->ToScale(clampedItemsToShow * m_module.m_comboBox->GetAppearance().ComboBoxItemHeight);
			m_module.m_floatBox = new FloatBox(window, { pointInScreen.X, pointInScreen.Y + (int)window->ClientSize.Height, window->ClientSize.Width, floatBoxHeight + 2u });
			m_module.m_floatBox->Init(m_module.Data);

			m_module.m_floatBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
			{
				int selectedIndex = m_module.m_floatBox->GetState().m_selectedIndex;

				delete m_module.m_floatBox;
				m_module.m_floatBox = nullptr;

				if (m_module.Data.m_isSelected && selectedIndex != m_module.Data.m_selectedIndex && !m_module.Data.m_items.empty())
				{
					m_module.Data.m_selectedIndex = selectedIndex;
					m_module.m_text = m_module.Data.m_items[m_module.Data.m_selectedIndex].m_text;

					DrawBatch drawBatch(m_module.m_owner);

					m_module.EmitSelectionEvent(selectedIndex);

					GUI::UpdateWindow(*m_control);
				}
			});

			GUI::Capture(m_module.m_floatBox->Handle(), true);
			m_module.m_floatBox->Show();
		}
	}

	void ComboBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		bool redraw = false;
		if (m_module.m_floatBox)
		{
			if (args.Key == KeyboardKey::ArrowUp)
			{
				if (m_module.m_floatBox->MoveSelectedItem(-1))
				{
					redraw = true;
				}
			}

			if (args.Key == KeyboardKey::ArrowDown)
			{
				if (m_module.m_floatBox->MoveSelectedItem(1))
				{
					redraw = true;
				}
			}
			if (args.Key == KeyboardKey::Enter)
			{
				if (m_module.m_floatBox->GetState().m_selectedIndex >= 0 && m_module.m_floatBox->GetState().m_selectedIndex < m_module.Data.m_items.size())
				{
					m_module.Data.m_selectedIndex = m_module.m_floatBox->GetState().m_selectedIndex;
					m_module.m_text = m_module.Data.m_items[m_module.Data.m_selectedIndex].m_text;
					m_module.m_floatBox->Dispose();
					m_module.EmitSelectionEvent(m_module.Data.m_selectedIndex);
					redraw = true;
				}
			}
		}
		else
		{
			if (args.Key == KeyboardKey::ArrowUp)
			{
				int newIndex = (std::max)(0, (std::min)(m_module.Data.m_selectedIndex - 1, (int)(m_module.Data.m_items.size()) - 1));
				if (m_module.Data.m_selectedIndex != newIndex && !m_module.Data.m_items.empty()) {
					m_module.Data.m_selectedIndex = newIndex;
					m_module.m_text = m_module.Data.m_items[newIndex].m_text;
					m_module.EmitSelectionEvent(newIndex);
					redraw = true;
				}
			}
			if (args.Key == KeyboardKey::ArrowDown)
			{
				int newIndex = (std::max)(0, (std::min)(m_module.Data.m_selectedIndex + 1, (int)(m_module.Data.m_items.size()) - 1));
				if (m_module.Data.m_selectedIndex != newIndex && !m_module.Data.m_items.empty()) {
					m_module.Data.m_selectedIndex = newIndex;
					m_module.m_text = m_module.Data.m_items[newIndex].m_text;
					m_module.EmitSelectionEvent(newIndex);
					redraw = true;
				}
			}
		}
		if (redraw)
		{
			auto window = m_control->Handle();
			window->Renderer.Update();
			GUI::MarkAsUpdated(window);
		}
	}

	std::wstring ComboBoxReactor::GetText(uint32_t index) const
	{
		return m_module.Data.m_items[index].m_text;
	}

	std::wstring ComboBoxReactor::GetText() const
	{
		return m_module.m_text;
	}

	void ComboBoxReactor::SetText(const std::wstring& text)
	{
		m_module.m_text = text;
		//for (size_t i = 0; i < m_interactionData.m_items.size(); i++)
		//{
		//	if (m_interactionData.m_items[i] == m_text)
		//	{
		//		m_interactionData.m_selectedIndex = (int)i;
		//		break;
		//	}
		//}

		auto window = m_control->Handle();
		window->Renderer.Update();
		GUI::MarkAsUpdated(window);
	}

	void ComboBoxReactor::Clear()
	{
		m_module.Data.m_items.clear();
		m_module.Data.m_selectedIndex = -1;
	}

	void ComboBoxReactor::Erase(uint32_t index)
	{
		if (index < m_module.Data.m_items.size())
		{
			auto& items = m_module.Data.m_items;
			auto& selectedIndex = m_module.Data.m_selectedIndex;
			items.erase(items.begin() + index);
			if (selectedIndex >= static_cast<int>(items.size()))
			{
				selectedIndex = static_cast<int>(items.size()) - 1;

				SetText(items[selectedIndex].m_text);
			}
		}
	}

	void ComboBoxReactor::PushItem(const std::wstring& text)
	{
		m_module.Data.m_items.push_back(Float::InteractionData::ItemType{ text });
	}

	void ComboBoxReactor::PushItem(const std::wstring& text, const Image& icon)
	{
		m_module.Data.m_items.push_back(Float::InteractionData::ItemType{ text, icon });
		m_module.Data.m_drawImages = true;
	}

	uint32_t ComboBoxReactor::GetSelectedIndex() const
	{
		return m_module.Data.m_selectedIndex;
	}

	void ComboBoxReactor::SetSelectedIndex(uint32_t index)
	{
		if (index < m_module.Data.m_items.size())
		{
			auto& items = m_module.Data.m_items;
			auto& selectedIndex = m_module.Data.m_selectedIndex;
			selectedIndex = static_cast<int>(index);

			SetText(items[selectedIndex].m_text);
		}
	}

	void ComboBoxReactor::Module::EmitSelectionEvent(int index)
	{
		ArgComboBox argComboBox{};
		argComboBox.SelectedIndex = index;

		auto events = dynamic_cast<ComboboxEvents*>(m_owner->Events.get());
		events->Selected.Emit(argComboBox);
	}

	ComboBox::ComboBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ComboBox";
#endif
	}

	void ComboBox::Clear()
	{
		m_reactor.Clear();
	}

	void ComboBox::Erase(uint32_t index)
	{
		m_reactor.Erase(index);
	}

	std::wstring ComboBox::GetText(uint32_t index)
	{
		return m_reactor.GetText(index);
	}

	void ComboBox::SetSelectedIndex(uint32_t index)
	{
		m_reactor.SetSelectedIndex(index);
	}

	void ComboBox::PushItem(const std::wstring& text)
	{
		m_reactor.PushItem(text);
	}

	void ComboBox::PushItem(const std::string& text)
	{
		std::wstring wText = StringUtils::Convert(text);
		m_reactor.PushItem(wText);
	}

	void ComboBox::PushItem(const std::wstring& text, const Image& icon)
	{
		m_reactor.PushItem(text, icon);
	}

	void ComboBox::PushItem(const std::string& text, const Image& icon)
	{
		std::wstring wText = StringUtils::Convert(text);
		m_reactor.PushItem(wText, icon);
	}

	void ComboBox::DoOnCaption(const std::wstring& caption)
	{
		m_reactor.SetText(caption);
	}

	std::wstring ComboBox::DoOnCaption() const
	{
		return m_reactor.GetText();
	}
}