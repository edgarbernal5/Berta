/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridFieldColor.h"

#include <cwctype>

#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	PropertyGridFieldColor::PropertyGridFieldColor(std::string_view label, GetterFn getter, SetterFn setter, ClickCallback onButtonClick, bool hasAlphaChannel) :
		TypedPropertyField(label, std::move(getter), std::move(setter)), m_clickCallback(std::move(onButtonClick)),
		m_hasAlphaChannel(hasAlphaChannel)
	{
	}
	
	void PropertyGridFieldColor::Draw(Graphics& graphics, const Rectangle& area, const LayoutConfig& config)
	{
		int buttonWidth = m_parent->ToScale(24);
		int margin = m_parent->ToScale(2);
		uint32_t labelWidth = graphics.GetTextExtent("X").Width;

		Rectangle inputArea = area;
		inputArea.Width -= (buttonWidth + margin);

		Rectangle buttonArea = area;
		buttonArea.X += static_cast<int>(inputArea.Width) + margin;
		buttonArea.Width = buttonWidth;
		
		m_colorRegion.SetArea(buttonArea);
		
		int channelsCount = m_hasAlphaChannel ? 4 : 3;
		
		int boxesSpace = static_cast<int>(inputArea.Width);
		int boxWidth = boxesSpace / channelsCount;
		
		for (int i = 0; i < channelsCount; ++i)
		{
			Rectangle boxRect = { 
				static_cast<int>(area.X + labelWidth + i * boxWidth) + margin, 
				area.Y, 
				static_cast<uint32_t>(boxWidth - margin * 2) - labelWidth,
				area.Height 
			};
			
			auto textExtents = graphics.GetTextExtent().Height;
			int offset = (static_cast<int>(area.Height) - static_cast<int>(textExtents)) / 2;
			graphics.DrawString({ area.X + (i * boxWidth), inputArea.Y + offset }, m_textBoxLabels[i], config.Foreground);
			
			m_textBoxes[i].SetArea(boxRect);
		}
	}

	void PropertyGridFieldColor::SetFocus()
	{
	}

	bool PropertyGridFieldColor::HasFocus() const
	{
		return false;
	}

	std::wstring PropertyGridFieldColor::GetValueAsString() const
	{
		return m_summaryText;
	}

	void PropertyGridFieldColor::SetButtonClick(ClickCallback callback)
	{
		m_clickCallback = std::move(callback);
	}

	void PropertyGridFieldColor::OnCreate(Window* parent)
	{
		m_colorRegion.Create(parent);

		m_colorRegion.GetEvents().Click.Connect([this](const ArgClick& args)
		{
			NotifySelected();
			if (m_clickCallback && m_getter)
			{
				std::optional<OptionalColor> result = m_clickCallback(m_getter());
				if (result.has_value()) 
				{
					m_setter(result.value());
					NotifyValueChanged();
					Refresh();
				}
			}
		});

		int channelsCount = m_hasAlphaChannel ? 4 : 3;
		for (int i = 0; i < channelsCount; ++i)
		{
			m_textBoxes[i].Create(parent);
			m_textBoxes[i].SetFocusBehavior(TextFocusBehavior::SelectOnClick);
            
			m_textBoxes[i].SetCharFilter([](wchar_t c)
			{
				return std::iswdigit(c);
			});
			
			m_textBoxes[i].GetEvents().Focus.Connect([this, i](const ArgFocus& args)
			{
				if (args.Focused)
				{
					this->NotifySelected();
				}
				else
				{
					ApplyValue(i);
				}
			});

			m_textBoxes[i].GetEvents().KeyPressed.Connect([this, i](const ArgKeyboard& args)
			{
				if (args.Key == KeyboardKey::Enter)
				{
					ApplyValue(i);
				}
			});
		}
		
		m_colorRegion.GetEvents().Focus.Connect([this](const ArgFocus& args)
		{
			if (args.Focused)
			{
				NotifySelected();
			}
		});
		
		Refresh();
	}

	void PropertyGridFieldColor::OnVisibilityChanged(bool visible)
	{
		int channelsCount = m_hasAlphaChannel ? 4 : 3;
		if (visible)
		{
			for (int i = 0; i < channelsCount; ++i)
			{
				m_textBoxes[i].Show();
			}
			m_colorRegion.Show();
		}
		else
		{
			for (int i = 0; i < channelsCount; ++i)
			{
				m_textBoxes[i].Hide();
			}
			m_colorRegion.Hide();
		}
	}

	void PropertyGridFieldColor::OnEnableChanged(bool enabled)
	{
		int channelsCount = m_hasAlphaChannel ? 4 : 3;
		for (int i = 0; i < channelsCount; ++i)
		{
			m_textBoxes[i].SetEnabled(enabled);
		}
		
		m_colorRegion.SetEnabled(enabled);
	}

	void PropertyGridFieldColor::SetValueInternal(const OptionalColor& value)
	{
		m_summaryText = FormatSummary(value);
		
		m_textBoxes[0].SetCaption(value.r.has_value() ? std::to_wstring(value.r.value()) : L"---");
		m_textBoxes[1].SetCaption(value.g.has_value() ? std::to_wstring(value.g.value()) : L"---");
		m_textBoxes[2].SetCaption(value.b.has_value() ? std::to_wstring(value.b.value()) : L"---");
		
		if (m_hasAlphaChannel)
		{
			m_textBoxes[3].SetCaption(value.a.has_value() ? std::to_wstring(value.a.value()) : L"---");
		}
		
		m_colorRegion.SetColor(Color(value.r.has_value() ? value.r.value() : 0, value.g.has_value() ? value.g.value() : 0, value.b.has_value() ? value.b.value() : 0, value.a.has_value() ? value.a.value() : 255));
		m_colorRegion.SetCaption(m_summaryText);
	}

	void PropertyGridFieldColor::SetMixedValuesInternal()
	{		
		if (!this->m_getter)
		{
			return;
		}
		
		std::optional<OptionalColor> stateOpt = this->m_getter();
    
		if (!stateOpt.has_value())
		{
			for (int i = 0; i < (m_hasAlphaChannel ? 4 : 3); ++i)
			{
				m_textBoxes[i].SetCaption(L"---");
			}
			return;
		}

		OptionalColor color = stateOpt.value();

		m_textBoxes[0].SetCaption(color.r.has_value() ? std::to_wstring(color.r.value()) : L"---");
		m_textBoxes[1].SetCaption(color.g.has_value() ? std::to_wstring(color.g.value()) : L"---");
		m_textBoxes[2].SetCaption(color.b.has_value() ? std::to_wstring(color.b.value()) : L"---");
    
		if (m_hasAlphaChannel)
		{
			m_textBoxes[3].SetCaption(color.a.has_value() ? std::to_wstring(color.a.value()) : L"---");
		}
	}
	void PropertyGridFieldColor::ApplyValue(int index)
	{		
		if (!this->m_setter || !this->m_getter)
	    {
	       return;
	    }
	    
	    std::wstring uiValue = m_textBoxes[index].GetCaptionW();
	    
	    std::optional<OptionalColor> currentStateOpt = this->m_getter();
	    OptionalColor currentState = currentStateOpt.value_or(OptionalColor{});
	    
	    std::optional<uint8_t>* currentChannel = nullptr;
	    switch (index)
	    {
	        case 0: currentChannel = &currentState.r; break;
	        case 1: currentChannel = &currentState.g; break;
	        case 2: currentChannel = &currentState.b; break;
	        case 3: currentChannel = &currentState.a; break;
	        default: return;
	    }
	    
	    if (uiValue == L"---")
	    {
	        if (currentChannel->has_value())
	        {
	            currentChannel->reset();
	            this->m_setter(currentState);
	            this->NotifyValueChanged();
	        }
	        this->Refresh();
	        return;
	    }
	    
	    try
	    {
	       int parsedInt = std::stoi(uiValue); 
	       uint8_t parsedValue = static_cast<uint8_t>(std::clamp(parsedInt, 0, 255));
	       
	       if (!currentChannel->has_value() || currentChannel->value() != parsedValue)
	       {
	          *currentChannel = parsedValue;
	          this->m_setter(currentState);
	          this->NotifyValueChanged(); 
	       }
	       
	       this->Refresh();
	    } 
	    catch (const std::exception&)
	    {
		    // Si el parseo falla (ej. el usuario tipeó letras), el Refresh restaurará
    		// el texto visual al último valor válido.
    		this->Refresh();
	    }
	}

	std::wstring PropertyGridFieldColor::FormatSummary(const OptionalColor& v) const
	{
		std::wstring summary = L"(";
		summary += v.r.has_value() ? std::to_wstring(v.r.value()) : L"---";
		summary += L", ";
		summary += v.g.has_value() ? std::to_wstring(v.g.value()) : L"---";
		summary += L", ";
		summary += v.b.has_value() ? std::to_wstring(v.b.value()) : L"---";
		
		if (m_hasAlphaChannel)
		{
			summary += L", ";
			summary += v.a.has_value() ? std::to_wstring(v.a.value()) : L"---";
		}
		summary += L")";
		
		return summary;
	}
}
