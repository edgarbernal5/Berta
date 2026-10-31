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
#include "Berta/Controls/Floating/FloatBox.h"
#include "TextEditors/SingleLineEditor.h"

namespace Berta
{
	namespace Internal::ComboBox
	{
		Reactor::Reactor() = default;
		Reactor::~Reactor() = default;

		void Reactor::Update(Graphics& graphics)
		{
			auto window = m_control->Handle();
			bool enabled = m_control->GetEnabled();
			auto backgroundRect = window->ClientSize.ToRectangle();

			auto mainEditorArea = m_module.GetEditorArea();
			auto buttonSize = window->ToScale(m_module.m_comboBox->GetAppearance().ButtonSize);
			
			Rectangle buttonRect = { 
				mainEditorArea.X + static_cast<int>(mainEditorArea.Width - buttonSize), 
				mainEditorArea.Y, 
				static_cast<uint32_t>(buttonSize), 
				mainEditorArea.Height 
			};
			
			if (m_module.m_isEditable)
			{
				if (m_module.m_status == State::Normal)
				{
					graphics.DrawGradientFill(buttonRect, window->Appearance->ButtonHighlightBackground, window->Appearance->ButtonBackground);	
				}
				else if (m_module.m_status == State::Hovered)
				{
					graphics.DrawGradientFill(buttonRect, window->Appearance->ButtonHighlightBackground, window->Appearance->ButtonBackground);
				}
			}
			else
			{
				if (m_module.m_status == State::Normal)
				{
					graphics.DrawGradientFill(backgroundRect, window->Appearance->ButtonHighlightBackground, window->Appearance->ButtonBackground);
				}
				else if (m_module.m_status == State::Hovered)
				{
					graphics.DrawGradientFill(backgroundRect, window->Appearance->ButtonHighlightBackground, window->Appearance->ButtonBackground);
				}
			}

			int currentX = mainEditorArea.X;
			auto selectedIndex = m_module.GetSelectedIndex();
			
			auto textItemHeight = graphics.GetTextExtent().Height;
			Point textPosition{ currentX + window->ToScale(2), static_cast<int>(window->ClientSize.Height - textItemHeight) >> 1 };
			
			if (m_module.Data.m_drawImages && selectedIndex)
			{
				auto iconSize = window->ToScale(window->Appearance->SmallIconSize);
				auto iconMargin = window->ToScale(3u);
				
				auto& icon = m_module.Data.m_items[*selectedIndex].m_icon;
				if (icon)
				{
					auto positionY = (window->ClientSize.Height - iconSize) >> 1;
					icon.Paste(graphics, { currentX, static_cast<int>(positionY), iconSize, iconSize });
				}
				currentX += static_cast<int>(iconSize + iconMargin * 2u);
			}
			
			if (m_module.m_isEditable)
			{
				m_module.m_textEditor->Render(graphics);
			}
			else
			{
				graphics.DrawString(textPosition, m_module.GetText(), enabled ? window->Appearance->Foreground : window->Appearance->BoxBorderDisabledColor);
			}
			
			int arrowWidth = window->ToScale(4);
			int arrowLength = window->ToScale(2);
			graphics.DrawArrow({ static_cast<int>(window->ClientSize.Width - buttonSize) - 1, 1, buttonSize, window->ClientSize.Height },
			                   arrowLength, 
			                   arrowWidth, 
			                   Graphics::ArrowDirection::Downwards,
			                   enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor,
			                   true,
			                   enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor);

			if (m_module.m_isEditable)
			{
				graphics.DrawLine({ buttonRect.X - 1, 1 }, { buttonRect.X - 1, static_cast<int>(window->ClientSize.Height) - 1 },
								  enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor);
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
			if (!args.ButtonState.LeftButton)
			{
				return;
			}
			auto window = m_module.m_owner;
			auto mainEditorArea = m_module.GetEditorArea();
			auto buttonSize = window->ToScale(m_module.m_comboBox->GetAppearance().ButtonSize);
			
			Rectangle buttonRect = { 
				mainEditorArea.X + static_cast<int>(mainEditorArea.Width - buttonSize), 
				mainEditorArea.Y, 
				static_cast<uint32_t>(buttonSize), 
				mainEditorArea.Height 
			};
			
			if (m_module.m_isEditable)
			{
				// Si el clic fue en el área del texto, delegamos al TextEditor y abortamos
				if (!buttonRect.Contains(args.Position))
				{
					if (m_module.m_textEditor)
					{
						// El SingleLineEditor procesará el clic, moverá su Caret y pedirá repintado[cite: 8, 12]
						m_module.m_textEditor->OnMouseDown(args);
					}
					return; // Importante: Salimos para NO abrir el FloatBox
				}
				
				// Si llegamos aquí, el clic fue sobre el botón.
				// El Caret debería apagarse o el texto seleccionarse según tu diseño, pero procedemos a abrir la lista.
			}
			
			auto clampedItemsToShow = static_cast<uint32_t>(std::min<size_t>(m_module.Data.m_items.size(), m_module.Data.m_maxItemsToDisplay));
			auto floatBoxHeight = window->ToScale(clampedItemsToShow * m_module.m_comboBox->GetAppearance().ComboBoxItemHeight);
			auto floatAppearance = static_cast<Internal::ComboBox::Appearance*>(window->Appearance.get());
			
			auto screenPos = API::GetPointClientToScreen(window->RootHandle, { 0, 0 });
			
			// 2. Obtenemos el área de trabajo del monitor donde está el ComboBox
			Rectangle monitorWorkArea = API::GetMonitorWorkArea(window->RootHandle);
			int spaceBelow = (monitorWorkArea.Y + monitorWorkArea.Height) - (screenPos.Y + static_cast<int>(window->ClientSize.Height));
			int spaceAbove = screenPos.Y - monitorWorkArea.Y;

			uint32_t totalHeightNeeded = floatBoxHeight + 2u;
			Rectangle floatRect = { 0, static_cast<int>(window->ClientSize.Height), window->ClientSize.Width, totalHeightNeeded };

			// Si no cabe abajo, pero sí cabe arriba (o al menos hay más espacio arriba)
			if (spaceBelow < static_cast<int>(totalHeightNeeded) && spaceAbove > spaceBelow)
			{
				// Invertimos el eje Y para que nazca desde el borde superior hacia arriba
				floatRect.Y = -static_cast<int>(totalHeightNeeded);
			}
			m_module.m_floatBox = std::make_unique<FloatBox>(window, floatRect);
			m_module.m_floatBox->Init(m_module.Data, floatAppearance->ComboBoxItemHeight);

			m_module.m_floatBox->GetEvents().Destroy.Connect([this](const ArgDestroy& argDestroy)
			{
				std::optional<size_t> selectedIndex = m_module.m_floatBox->GetState().m_selectedIndex;
					
				m_module.m_floatBox.reset();

				if (m_module.Data.m_isSelected && selectedIndex && !m_module.Data.m_items.empty())
				{
					m_module.SetSelectedIndex(selectedIndex);
						
					GUI::MarkAsNeedUpdate(*m_control);
				}
			});

			GUI::Capture(m_module.m_floatBox->Handle(), true);
			m_module.m_floatBox->Show();
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_isEditable && m_module.m_textEditor)
			{
				m_module.m_textEditor->OnMouseMove(args);
			}
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_isEditable && m_module.m_textEditor)
			{
				m_module.m_textEditor->OnMouseUp(args);
			}
		}

		void Reactor::DblClick(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_isEditable && m_module.m_textEditor)
			{
				if (m_module.m_textEditor->OnDblClick(args))
				{
					GUI::MarkAsNeedUpdate(m_control->Handle());
				}
			}
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
		{
			if (m_module.m_isEditable && m_module.m_textEditor)
			{
				if (m_module.m_textEditor->OnKeyPressed(args)) 
				{
					GUI::MarkAsNeedUpdate(m_control->Handle());
					return;
				}
				if (args.Key == KeyboardKey::Enter)
				{
					//m_module.SetSelectedIndex()
				}
			}
			
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
						m_module.m_floatBox->Dispose();
						m_module.SetSelectedIndex(selectedIndex);
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

		void Reactor::Focus(Graphics& graphics, const ArgFocus& args)
		{
			if (m_module.m_isEditable && m_module.m_textEditor)
			{
				if (m_module.m_textEditor->OnFocus(args)) 
				{
					GUI::MarkAsNeedUpdate(m_control->Handle());
				}
			}
		}

		void Reactor::KeyChar(Graphics& graphics, const ArgKeyboard& args)
		{
			if (m_module.m_isEditable && m_module.m_textEditor)
			{
				if (m_module.m_textEditor->OnKeyChar(args)) 
				{
					GUI::MarkAsNeedUpdate(m_control->Handle());
				}
			}
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			if (m_module.m_isEditable && m_module.m_textEditor)
			{
				m_module.UpdateEditorArea();
				m_module.m_textEditor->OnResize(args);
			}
		}

		void Reactor::DpiChanged(Graphics& graphics)
		{
			if (m_module.m_isEditable && m_module.m_textEditor)
			{
				//m_module.UpdateEditorArea();
				m_module.m_textEditor->OnDpiChanged();
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
		}

		std::optional<size_t> Reactor::Module::GetSelectedIndex() const
		{
			return Data.m_selectedIndex;
		}

		std::any Reactor::Module::GetItemData(size_t index) const
		{
			if (index >= Data.m_items.size())
			{
				return {};
			}
			
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
			if (selectedIndex.has_value())
			{
				SetText(Data.m_items[*selectedIndex].m_text);
			}
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
			if (m_textEditor)
			{
				m_textEditor->SetContent(m_text);
			}
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
			if (m_isEditable == editable)
			{
				return;
			}
			
			m_isEditable = editable;
			UpdateEditorArea();
			GUI::MarkAsNeedUpdate(m_owner);
		}

		Rectangle Reactor::Module::GetEditorArea() const
		{
			auto area = m_comboBox->GetSize().ToRectangle();
			if (!GUI::IsWindowBorderless(m_owner))
			{
				auto one = m_owner->ToScale(1);
				
				area.X = area.Y = one;
				if (area.Width > one * 2) area.Width -= one * 2;
				else area.Width = 0;
				
				if (area.Height > one * 2) area.Height -= one * 2;
				else area.Height = 0;
			}
			return area;
		}

		void Reactor::Module::UpdateEditorArea()
		{
			auto mainEditorArea = GetEditorArea();
			auto buttonSize = m_owner->ToScale(m_comboBox->GetAppearance().ButtonSize);
			
			int currentX = mainEditorArea.X;
			
			Rectangle editorArea = { 
				currentX,
				mainEditorArea.Y,
				static_cast<uint32_t>(mainEditorArea.Width - buttonSize - currentX - 1),
				m_owner->ClientSize.Height - 2u
			};
			m_textEditor->SetEditorArea(editorArea, {0,0,m_owner->ToScale(2),0});
		}

		void Reactor::Module::ShowIcons(bool show)
		{
			if (Data.m_drawImages == show)
			{
				return;
			}
			
			Data.m_drawImages = show;
			GUI::MarkAsNeedUpdate(m_owner);
		}

		void Reactor::DoOnInit()
		{
			m_module.m_comboBox = reinterpret_cast<Berta::ComboBox*>(m_control);
			m_module.m_textEditor = std::make_unique<SingleLineEditor>(*m_control);
			m_module.m_textEditor->SetValueChangedCallback([this]()
			{
				std::wstring currentText = m_module.m_textEditor->GetContent();
				auto& data = m_module.Data;
				std::optional<size_t> matchIndex = std::nullopt;
				
				for (size_t i = 0; i < data.m_items.size(); ++i)
				{
					if (data.m_items[i].m_text == currentText)
					{
						matchIndex = i;
						break;
					}
				}
				
				if (m_module.m_floatBox)
				{
					GUI::MarkAsNeedUpdate(m_module.m_floatBox->Handle());
				}
				
				if (data.m_selectedIndex != matchIndex)
				{
					data.m_selectedIndex = matchIndex;
					m_module.EmitSelectionEvent(matchIndex);
				}
			});
			m_module.m_textEditor->SetSubmitCallback([this]()
			{
				m_module.SetSelectedIndex(m_module.Data.m_selectedIndex);
				if (m_module.m_floatBox)
				{
					m_module.m_floatBox->Dispose();
				}
				/*std::wstring currentText = m_module.m_textEditor->GetContent();
				if (m_module.m_floatBox)
				{
					m_module.m_floatBox->Dispose();
					m_module.m_floatBox.reset();
				}
				m_module.EmitSelectionEvent(m_module.Data.m_selectedIndex);
				
				if (m_module.Data.m_selectedIndex)
				{
					std::wstring exactText = m_module.Data.m_items[*m_module.Data.m_selectedIndex].m_text;
					if (currentText != exactText)
					{
						m_module.m_textEditor->SetContent(exactText); 
					}
				}*/
			});
			
			auto window = m_control->Handle();
			window->Events->Focus.Connect([this](const ArgFocus& args)
			{
				if (!args.Focused && m_module.m_floatBox)
				{
					m_module.m_floatBox->Dispose();
				}
			});
			m_module.m_owner = window;
			m_module.UpdateEditorArea();
			
			GUI::SetWindowBorderless(*m_control, false);
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

	void ComboBox::ShowIcons(bool show)
	{
		GetReactor().GetModule().ShowIcons(show);
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
