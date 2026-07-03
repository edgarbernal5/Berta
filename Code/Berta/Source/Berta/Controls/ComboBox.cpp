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
	namespace Internal::ComboBox
	{
		Reactor::~Reactor()
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

		void Reactor::Update(Graphics& graphics)
		{
			auto window = m_control->Handle();
			bool enabled = m_control->GetEnabled();
			auto backgroundRect = window->ClientSize.ToRectangle();

			if (m_module.m_status == State::Normal)
			{
				graphics.FillRectangle(backgroundRect, window->Appearance->BoxBackground);
			}
			else if (m_module.m_status == State::Hovered)
			{
				graphics.FillRectangle(backgroundRect, window->Appearance->BoxHightlightBackground);
			}

			//m_textEditor->Render();

			auto textItemHeight = graphics.GetTextExtent().Height;
			Point textPosition{ 3,static_cast<int>(window->ClientSize.Height - textItemHeight) >> 1 };
			auto selectedIndex = m_module.GetSelectedIndex();
			if (m_module.Data.m_drawImages && selectedIndex)
			{
				auto iconSize = window->ToScale(window->Appearance->SmallIconSize);
				auto iconMargin = window->ToScale(3u);
				textPosition.X += static_cast<int>(iconSize + iconMargin * 2u);
				
				auto& icon = m_module.Data.m_items[*selectedIndex].m_icon;
				if (icon)
				{
					auto positionY = (window->ClientSize.Height - iconSize) >> 1;
					icon.Paste(graphics, { 3, static_cast<int>(positionY), iconSize , iconSize });
				}
			}
			graphics.DrawString(textPosition, m_module.m_text, enabled ? window->Appearance->Foreground : window->Appearance->BoxBorderDisabledColor);

			auto buttonSize = window->ToScale(m_module.m_comboBox->GetAppearance().ButtonSize);

			graphics.FillRectangle({ static_cast<int>(window->ClientSize.Width - buttonSize - 1), 1, buttonSize, window->ClientSize.Height - 2 }, window->Appearance->Background);

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

			graphics.DrawRectangle(backgroundRect, enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor);
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
			if (args.ButtonState.LeftButton)
			{
				auto window = m_module.m_owner;

				auto clampedItemsToShow = static_cast<uint32_t>((std::min)(m_module.Data.m_items.size(), m_module.Data.m_maxItemsToDisplay));
				auto floatBoxHeight = window->ToScale(clampedItemsToShow * m_module.m_comboBox->GetAppearance().ComboBoxItemHeight);
				m_module.m_floatBox = new FloatBox(window, { 0, (int)window->ClientSize.Height, window->ClientSize.Width, floatBoxHeight + 2u });
				m_module.m_floatBox->Init(m_module.Data);

				m_module.m_floatBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
				{
					std::optional<size_t> selectedIndex = m_module.m_floatBox->GetState().m_selectedIndex;
					
					delete m_module.m_floatBox;
					m_module.m_floatBox = nullptr;

					if (m_module.Data.m_isSelected && selectedIndex && !m_module.Data.m_items.empty())
					{
						m_module.SetSelectedIndex(selectedIndex);
						//TODO:
						//DrawBatch drawBatch(m_module.m_owner);

						//m_module.EmitSelectionEvent(selectedIndex);

						GUI::UpdateWindow(*m_control);
					}
				});

				GUI::Capture(m_module.m_floatBox->Handle(), true);
				m_module.m_floatBox->Show();
			}
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
		{
			bool redraw = false;
			auto currentIndex = m_module.GetSelectedIndex();
			size_t itemCount = m_module.Count();
			
			if (itemCount == 0)
			{
				return;
			}
			
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
					if (m_module.m_floatBox->GetState().m_selectedIndex.has_value() && m_module.m_floatBox->GetState().m_selectedIndex < m_module.Data.m_items.size())
					{
						auto selectedIndex=m_module.m_floatBox->GetState().m_selectedIndex;
						//m_module.Data.m_selectedIndex = m_module.m_floatBox->GetState().m_selectedIndex;
						//m_module.m_text = m_module.Data.m_items[*m_module.Data.m_selectedIndex].m_text;
						m_module.m_floatBox->Dispose();
						m_module.SetSelectedIndex(selectedIndex);
						//m_module.EmitSelectionEvent(m_module.Data.m_selectedIndex);
						redraw = true;
					}
				}
			}
			else
			{
				if (args.Key == KeyboardKey::ArrowUp)
				{
					// If nothing selected, pick the first. If selected, decrement if > 0.
					size_t newIndex = currentIndex ? (*currentIndex > 0 ? *currentIndex - 1 : 0) : 0;
					if (newIndex != currentIndex) 
					{
						m_module.SetSelectedIndex(newIndex);
						redraw = true;
					}
				}
				if (args.Key == KeyboardKey::ArrowDown)
				{
					// If nothing selected, pick first. If selected, increment if < max.
					size_t newIndex = currentIndex ? (std::min<size_t>(*currentIndex + 1, itemCount - 1)) : 0;
					if (newIndex != currentIndex)
					{
						m_module.SetSelectedIndex(newIndex);
						redraw = true;
					}
				}
			}
			
			if (redraw)
			{
				auto window = m_control->Handle();
				GUI::MarkAsNeedUpdate(window);
			}
		}

		Float::InteractionData::ItemType& Reactor::Module::At(size_t index)
		{
			return Data.m_items[index];
		}

		void Reactor::Module::Clear()
		{
			Data.m_items.clear();
			SetSelectedIndex(std::nullopt); // Explicitly clear selection
		}

		size_t Reactor::Module::Count() const
		{
			return Data.m_items.size();
		}

		void Reactor::Module::Erase(size_t index)
		{
			if (index >= Data.m_items.size())
			{
				return;
			}
			
			auto& items = Data.m_items;
			items.erase(items.begin() + index);
				
			auto current = GetSelectedIndex();
			if (!current)
			{
				return;
			}
			if (Data.m_items.empty())
			{
				SetSelectedIndex(std::nullopt);
			}
			else if (*current >= Data.m_items.size())
			{
				// Adjust selection to the new last item if the old one was deleted
				SetSelectedIndex(Data.m_items.size() - 1);
			}
		}

		void Reactor::Module::PushBack(const std::wstring& text, const std::any& userData)
		{
			Data.m_items.emplace_back(text, userData );
		}

		void Reactor::Module::PushBack(const std::wstring& text, const Image& icon, const std::any& userData)
		{
			Data.m_items.emplace_back(text, icon, userData);
			Data.m_drawImages = true;
		}

		std::optional<size_t> Reactor::Module::GetSelectedIndex() const
		{
			return Data.m_selectedIndex;
		}

		std::any Reactor::Module::GetItemData(size_t index) const
		{
			if (index >= Data.m_items.size())
				return {};
			
			return Data.m_items[index].m_userData;
		}

		void Reactor::Module::SetSelectedIndex(std::optional<size_t> index)
		{
			std::optional<size_t> selectedIndex;
			if (index && *index >= Count())
			{
				selectedIndex = std::nullopt;
			}
			else
			{
				selectedIndex = index;
			}
			if (selectedIndex == Data.m_selectedIndex)
			{
				return;
			}
			
			Data.m_selectedIndex = selectedIndex;
			SetText(selectedIndex.has_value() ? Data.m_items[*selectedIndex].m_text : L"");
			EmitSelectionEvent(selectedIndex);
		}

		std::wstring Reactor::Module::GetText(size_t index) const
		{
			return Data.m_items[index].m_text;
		}

		std::wstring Reactor::Module::GetText() const
		{
			return m_text;
		}

		void Reactor::Module::SetText(const std::wstring& text)
		{
			if (m_text == text)
			{
				return;
			}
			
			m_text = text;
			//for (size_t i = 0; i < m_interactionData.m_items.size(); i++)
			//{
			//	if (m_interactionData.m_items[i] == m_text)
			//	{
			//		m_interactionData.m_selectedIndex = (int)i;
			//		break;
			//	}
			//}

			GUI::MarkAsNeedUpdate(m_owner);
		}

		void Reactor::Module::EmitSelectionEvent(std::optional<size_t> index) const
		{
			ArgComboBox argComboBox;
			argComboBox.SelectedIndex = index;

			auto events = dynamic_cast<Events*>(m_owner->Events.get());
			events->Selected.Emit(argComboBox);
		}

		void Reactor::Module::UpdateItem(size_t index)
		{
			GUI::UpdateWindow(m_owner);
		}

		bool Reactor::Module::IsEditable() const
		{
			return m_isEditable;
		}

		void Reactor::Module::SetEditable(bool editable)
		{
			m_isEditable = editable;
		}

		void Reactor::DoOnInit()
		{
			m_module.m_comboBox = reinterpret_cast<Berta::ComboBox*>(m_control);
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

		void ComboBoxItem::SetText(const std::wstring& text)
		{
			if (m_module->Data.m_items.at(m_index).m_text == text)
			{
				return;
			}
			
			m_module->Data.m_items.at(m_index).m_text = text;
			m_module->UpdateItem(m_index);
		}

		void ComboBoxItem::SetImage(const Image& icon)
		{
			if (m_module->Data.m_items.at(m_index).m_icon == icon)
			{
				return;
			}
			
			m_module->Data.m_items.at(m_index).m_icon = icon;
			m_module->UpdateItem(m_index);
		}
	}
	
	ComboBox::ComboBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ComboBox";
#endif
	}

	ComboBox::ComboBoxItem ComboBox::At(size_t index)
	{
		auto& module = GetReactor().GetModule();
		if (index < module.Count())
		{
			return {index, &module};	
		}
		return {};
	}

	void ComboBox::Clear()
	{
		GetReactor().GetModule().Clear();
	}

	size_t ComboBox::Count() const
	{
		return GetReactor().GetModule().Count();
	}

	void ComboBox::Erase(size_t index)
	{
		GetReactor().GetModule().Erase(index);
	}

	std::wstring ComboBox::GetText() const
	{
		return DoOnCaption();
	}

	std::any ComboBox::GetSelectedData() const
	{
		auto selectedIndex = GetSelectedIndex();
		if (selectedIndex.has_value())
		{
			return GetItemData(selectedIndex.value());
		}
		return {};
	}

	std::any ComboBox::GetItemData(size_t index) const
	{
		return GetReactor().GetModule().GetItemData(index);
	}

	void ComboBox::SetSelectedIndex(std::optional<size_t> index)
	{
		GetReactor().GetModule().SetSelectedIndex(index);
	}

	void ComboBox::PushBack(const std::wstring& text, const std::any& userData)
	{
		GetReactor().GetModule().PushBack(text, userData);
	}

	void ComboBox::PushBack(const std::string& text, const std::any& userData)
	{
		std::wstring wText = StringUtils::UTF8ToWide(text);
		GetReactor().GetModule().PushBack(wText, userData);
	}

	void ComboBox::PushBack(const std::wstring& text, const Image& icon, const std::any& userData)
	{
		GetReactor().GetModule().PushBack(text, icon, userData);
	}

	void ComboBox::PushBack(const std::string& text, const Image& icon, const std::any& userData)
	{
		std::wstring wText = StringUtils::UTF8ToWide(text);
		GetReactor().GetModule().PushBack(wText, icon, userData);
	}

	void ComboBox::SetEditable(bool editable)
	{
		GetReactor().GetModule().SetEditable(editable);
	}

	void ComboBox::DoOnCaption(const std::wstring& caption)
	{
		GetReactor().GetModule().SetText(caption);
	}

	std::wstring ComboBox::DoOnCaption() const
	{
		return GetReactor().GetModule().GetText();
	}
}