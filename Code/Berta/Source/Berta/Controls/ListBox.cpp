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
		m_module.m_window = control.Handle();

		m_module.m_appearance = reinterpret_cast<ListBoxAppearance*>(control.Handle()->Appearance.get());

		m_module.CalculateViewport(m_module.m_viewport);
	}

	void ListBoxReactor::Update(Graphics& graphics)
	{
		auto enabled = m_control->GetEnabled();
		graphics.DrawRectangle(m_module.m_window->Size.ToRectangle(), m_module.m_window->Appearance->BoxBackground, true);

		DrawList(graphics);
		DrawHeaders(graphics);

		if (m_module.m_viewport.NeedHorizontalScroll && m_module.m_viewport.NeedVerticalScroll)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			graphics.DrawRectangle({ (int)(m_module.m_window->Size.Width - scrollSize) - 1, (int)(m_module.m_window->Size.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background, true);
		}
		graphics.DrawRectangle(m_module.m_window->Size.ToRectangle(), enabled ? m_module.m_window->Appearance->BoxBorderColor : m_module.m_window->Appearance->BoxBorderDisabledColor, false);
	}

	void ListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.CalculateViewport(m_module.m_viewport);

		UpdateScrollBars();
	}

	void ListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto hoveredArea = DetermineHoverArea(args.Position);
	}

	void ListBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
	}

	void ListBoxReactor::Module::CalculateViewport(ViewportData& viewportData)
	{
		viewportData.BackgroundRect = m_window->Size.ToRectangle();
		viewportData.BackgroundRect.Y = viewportData.BackgroundRect.X = 1;
		viewportData.BackgroundRect.Width -= 2u;
		viewportData.BackgroundRect.Height -= 2u;
		viewportData.InnerMargin = m_window->ToScale(3u);

		auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
		auto itemHeight = m_window->ToScale(m_appearance->ListItemHeight);

		viewportData.BackgroundRect.Y += headerHeight;
		viewportData.BackgroundRect.Height -= headerHeight;

		viewportData.ContentSize.Height = (uint32_t)List.Items.size() * (itemHeight + viewportData.InnerMargin * 2u);

		viewportData.ContentSize.Width = 0u;
		for (size_t i = 0; i < Headers.Items.size(); i++)
		{
			auto headerWidth = m_window->ToScale(Headers.Items[i].Bounds.Width);
			viewportData.ContentSize.Width += headerWidth;
		}

		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		viewportData.NeedVerticalScroll = viewportData.ContentSize.Height > viewportData.BackgroundRect.Height;
		if (viewportData.NeedVerticalScroll)
		{
			viewportData.BackgroundRect.Width -= scrollSize;
		}
		viewportData.NeedHorizontalScroll = viewportData.ContentSize.Width > viewportData.BackgroundRect.Width;
		if (viewportData.NeedHorizontalScroll)
		{
			viewportData.BackgroundRect.Height -= scrollSize;
			viewportData.NeedVerticalScroll = viewportData.ContentSize.Height > viewportData.BackgroundRect.Height;
			if (viewportData.NeedVerticalScroll)
			{
				viewportData.BackgroundRect.Width -= scrollSize;
			}
		} 
	}

	void ListBoxReactor::Module::BuildHeaderBounds(uint32_t startIndex)
	{
		Point offset{ 0,0 };
		if (startIndex > 0)
		{
			offset.X = Headers.Items[startIndex - 1].Bounds.X + Headers.Items[startIndex - 1].Bounds.Width;
		}
		for (size_t i = startIndex; i < Headers.Items.size(); i++)
		{
			Headers.Items[i].Bounds.X = offset.X;

			offset.X += Headers.Items[i].Bounds.Width;
		}
	}

	void ListBoxReactor::Module::BuildListItemBounds(uint32_t startIndex)
	{
		auto listItemHeight = m_window->ToScale(m_appearance->ListItemHeight);
		Point offset{ 0,0 };
		if (startIndex > 0)
		{
			offset.Y = List.Items[startIndex - 1].Bounds.Y + List.Items[startIndex - 1].Bounds.Height;
		}
		for (size_t i = startIndex; i < List.Items.size(); i++)
		{
			List.Items[i].Bounds.Y = offset.Y;
			List.Items[i].Bounds.Height = listItemHeight;

			offset.Y += List.Items[i].Bounds.Height;
		}
	}

	void ListBoxReactor::DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds)
	{
		auto textExtent = graphics.GetTextExtent(str);
		if (boxBounds.X + (int)textExtent.Width < 0 /* || boxBounds.X >= */)
		{
			return;
		}

		if (textExtent.Width < boxBounds.Width)
		{
			graphics.DrawString({ boxBounds.X, boxBounds.Y + ((int)(boxBounds.Height - textExtent.Height) >> 1) }, str, m_module.m_appearance->Foreground);
			return;
		}

		auto ellipsisTextExtent = graphics.GetTextExtent("...").Width;
		for (size_t i = str.size(); i >= 1; --i)
		{
			auto subStr = str.substr(0, i);// +"...";
			auto subTextExtent = graphics.GetTextExtent(subStr).Width;
			if (boxBounds.X + (int)(subTextExtent + ellipsisTextExtent) <= (int)boxBounds.Width - 2)
			{
				graphics.DrawString({ boxBounds.X, boxBounds.Y + ((int)(boxBounds.Height - textExtent.Height) >> 1) }, subStr + "...", m_module.m_appearance->Foreground);
				break;
			}
		}
	}

	void ListBoxReactor::Module::AppendHeader(const std::string& text, uint32_t width)
	{
		auto startIndex = Headers.Items.size();
		Headers.Items.emplace_back(text, (std::max)(width, LISTBOX_MIN_HEADER_WIDTH));

		BuildHeaderBounds(startIndex);
		CalculateViewport(m_viewport);
	}

	void ListBoxReactor::Module::Append(const std::string& text)
	{
		List.Items.emplace_back(text);
		CalculateViewport(m_viewport);
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
		m_reactor.GetModule().AppendHeader(name, width);
	}

	void ListBox::Append(const std::string& text)
	{
		m_reactor.GetModule().Append(text);
	}

	void ListBox::Append(std::initializer_list<std::string> texts)
	{
		m_reactor.GetModule().Append(texts);
	}

	void ListBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	void ListBox::ClearHeaders()
	{
	}

	void ListBoxReactor::Module::Append(std::initializer_list<std::string> texts)
	{
		auto startIndex = List.Items.size();

		auto headersCount = Headers.Items.size();
		auto& item = List.Items.emplace_back("{}");
		size_t position = 0;
		for (auto& text : texts)
		{
			if (item.Cells.size() == position)
			{
				item.Cells.emplace_back(text);
			}
			else
			{
				item.Cells[position] = text;
			}
			++position;
			if (position >= headersCount)
				break;
		}

		BuildListItemBounds(startIndex);
		CalculateViewport(m_viewport);
	}

	void ListBoxReactor::Module::Clear()
	{

	}

	bool ListBoxReactor::UpdateScrollBars()
	{
		auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
		bool needUpdate = false;
		if (m_module.m_viewport.NeedVerticalScroll && !m_module.m_scrollBarVert)
		{
			auto listItemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight);
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			if (m_module.m_viewport.NeedHorizontalScroll)
				scrollRect.Height -= scrollSize;

			m_module.m_scrollBarVert = std::make_unique<ScrollBar>(m_module.m_window, false, scrollRect);
			m_module.m_scrollBarVert->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
				{
					m_module.ScrollOffset.Y = args.Value;

					m_module.m_window->Renderer.Update();
					GUI::RefreshWindow(m_module.m_window);
				});

			m_module.m_scrollBarVert->SetMinMax(0, (int)(m_module.m_viewport.ContentSize.Height - m_module.m_viewport.BackgroundRect.Height));
			m_module.m_scrollBarVert->SetPageStepValue(m_module.m_viewport.BackgroundRect.Height);
			m_module.m_scrollBarVert->SetStepValue(listItemHeight);
			needUpdate = true;
		}
		else if (m_module.m_viewport.NeedVerticalScroll && m_module.m_scrollBarVert)
		{
			auto listItemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight);
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			if (m_module.m_viewport.NeedHorizontalScroll)
				scrollRect.Height -= scrollSize;

			m_module.m_scrollBarVert->SetMinMax(0, (int)(m_module.m_viewport.ContentSize.Height - m_module.m_viewport.BackgroundRect.Height));
			m_module.m_scrollBarVert->SetPageStepValue(m_module.m_viewport.BackgroundRect.Height);
			m_module.m_scrollBarVert->SetStepValue(listItemHeight);

			GUI::MoveWindow(m_module.m_scrollBarVert->Handle(), scrollRect);
			needUpdate = true;
		}
		else if (!m_module.m_viewport.NeedVerticalScroll && m_module.m_scrollBarVert)
		{
			m_module.m_scrollBarVert.reset();
			m_module.ScrollOffset.Y = 0;

			needUpdate = true;
		}

		if (m_module.m_viewport.NeedHorizontalScroll && !m_module.m_scrollBarHoriz)
		{
			Rectangle scrollRect{ 1, static_cast<int>(m_module.m_window->Size.Height - scrollSize) - 1, m_module.m_window->Size.Width - 2u, scrollSize };
			if (m_module.m_viewport.NeedVerticalScroll)
				scrollRect.Width -= scrollSize;

			m_module.m_scrollBarHoriz = std::make_unique<ScrollBar>(m_module.m_window, false, scrollRect, false);
			m_module.m_scrollBarHoriz->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
				{
					m_module.ScrollOffset.X = args.Value;

					m_module.m_window->Renderer.Update();
					GUI::RefreshWindow(m_module.m_window);
				});

			m_module.m_scrollBarHoriz->SetMinMax(0, (int)(m_module.m_viewport.ContentSize.Width - m_module.m_viewport.BackgroundRect.Width));
			m_module.m_scrollBarHoriz->SetPageStepValue(m_module.m_viewport.BackgroundRect.Width);
			m_module.m_scrollBarHoriz->SetStepValue(scrollSize);
			needUpdate = true;
		}
		else if (m_module.m_viewport.NeedHorizontalScroll && m_module.m_scrollBarHoriz)
		{
			Rectangle scrollRect{ 1, static_cast<int>(m_module.m_window->Size.Height - scrollSize) - 1, m_module.m_window->Size.Width - 2u, scrollSize };
			if (m_module.m_viewport.NeedVerticalScroll)
				scrollRect.Width -= scrollSize;

			m_module.m_scrollBarHoriz->SetMinMax(0, (int)(m_module.m_viewport.ContentSize.Width - m_module.m_viewport.BackgroundRect.Width));
			m_module.m_scrollBarHoriz->SetPageStepValue(m_module.m_viewport.BackgroundRect.Width);

			GUI::MoveWindow(m_module.m_scrollBarHoriz->Handle(), scrollRect);
			needUpdate = true;
		}
		else if (!m_module.m_viewport.NeedHorizontalScroll && m_module.m_scrollBarHoriz)
		{
			m_module.m_scrollBarHoriz.reset();
			m_module.ScrollOffset.X = 0;

			needUpdate = true;
		}
		return needUpdate;
	}

	void ListBoxReactor::DrawHeaders(Graphics& graphics)
	{
		auto headerHeight = m_module.m_window->ToScale(m_module.m_appearance->HeadersHeight);
		auto leftMarginTextHeader = m_module.m_window->ToScale(5u);
		graphics.DrawRectangle({ 0,0, m_module.m_window->Size.Width, headerHeight }, m_module.m_appearance->ButtonBackground, true);
		graphics.DrawLine({ m_module.m_viewport.BackgroundRect.X, (int)headerHeight - 1 }, { (int)m_module.m_window->Size.Width - 1, (int)headerHeight - 1 }, m_module.m_appearance->BoxPressedBackground);
		graphics.DrawLine({ m_module.m_viewport.BackgroundRect.X, (int)headerHeight }, { (int)m_module.m_window->Size.Width - 1, (int)headerHeight }, m_module.m_appearance->Foreground);
		Point headerOffset{ -m_module.ScrollOffset.X, 0 };

		for (size_t i = 0; i < m_module.Headers.Items.size(); i++)
		{
			const auto& header = m_module.Headers.Items[i];
			auto headerWidth = m_module.m_window->ToScale(header.Bounds.Width);
			auto headerWidthInt = (int)headerWidth;
			if (headerOffset.X + headerWidthInt < 0 || headerOffset.X >= (int)m_module.m_viewport.BackgroundRect.Width)
			{
				headerOffset.X += headerWidthInt;
				continue;
			}

			DrawStringInBox(graphics, header.Name, { headerOffset.X + (int)leftMarginTextHeader, 0, headerWidth, headerHeight });

			graphics.DrawLine({ headerOffset.X + headerWidthInt - 1, 0 }, { headerOffset.X + headerWidthInt - 1, (int)headerHeight - 1 }, m_module.m_appearance->BoxPressedBackground);
			graphics.DrawLine({ headerOffset.X + headerWidthInt, 0 }, { headerOffset.X + headerWidthInt, (int)headerHeight - 1 }, m_module.m_appearance->Foreground);

			headerOffset.X += headerWidthInt;
		}
	}

	void ListBoxReactor::DrawList(Graphics& graphics)
	{
		auto headerHeight = m_module.m_window->ToScale(m_module.m_appearance->HeadersHeight);

		Point listOffset{ -m_module.ScrollOffset.X, m_module.m_viewport.BackgroundRect.Y - m_module.ScrollOffset.Y };
		auto itemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight);
		auto leftMarginListItemText = m_module.m_window->ToScale(3u);

		for (size_t i = 0; i < m_module.List.Items.size(); i++)
		{
			const auto& item = m_module.List.Items[i];
			listOffset.X = -m_module.ScrollOffset.X;

			for (size_t j = 0; j < item.Cells.size(); j++)
			{
				const auto& cell = item.Cells[j];
				const auto& header = m_module.Headers.Items[j];
				auto headerWidth = m_module.m_window->ToScale(header.Bounds.Width);
				auto headerWidthInt = (int)headerWidth;
				if (listOffset.X + headerWidthInt < 0)
				{
					listOffset.X += headerWidthInt;
					continue;
				}
				else if (listOffset.X >= (int)m_module.m_viewport.BackgroundRect.Width)
				{
					break;
				}

				DrawStringInBox(graphics, cell.Text, { listOffset.X + (int)leftMarginListItemText, listOffset.Y + (int)m_module.m_viewport.InnerMargin, headerWidth, itemHeight });

				listOffset.X += headerWidthInt;
			}
			listOffset.Y += (int)(itemHeight + m_module.m_viewport.InnerMargin * 2u);
		}
	}

	Berta::ListBoxReactor::InteractionArea ListBoxReactor::DetermineHoverArea(const Point& mousePosition)
	{
		auto headerHeight = m_module.m_window->ToScale(m_module.m_appearance->HeadersHeight);
		if (mousePosition.Y <= (int)headerHeight)
		{
			Point headerOffset{ -m_module.ScrollOffset.X, 0 };

			auto splitterThreshold = m_module.m_window->ToScale(3);
			for (size_t i = 0; i < m_module.Headers.Items.size(); i++)
			{
				const auto& header = m_module.Headers.Items[i];
				auto headerWidth = m_module.m_window->ToScale(header.Bounds.Width);
				auto headerWidthInt = (int)headerWidth;
				if (headerOffset.X + headerWidthInt < -splitterThreshold || headerOffset.X - splitterThreshold >= (int)m_module.m_viewport.BackgroundRect.Width)
				{
					headerOffset.X += headerWidthInt;
					continue;
				}

				if (mousePosition.X >= headerOffset.X + headerWidthInt - splitterThreshold &&
					mousePosition.X <= headerOffset.X + headerWidthInt + splitterThreshold)
				{
					BT_CORE_TRACE << "  ** InteractionArea = HeaderSplitter." << std::endl;
					return InteractionArea::HeaderSplitter;
				}
				else if (mousePosition.X >= headerOffset.X &&
					mousePosition.X < headerOffset.X + headerWidthInt - splitterThreshold)
				{
					BT_CORE_TRACE << "  ** InteractionArea = Header." << std::endl;
					return InteractionArea::Header;
				}

				headerOffset.X += headerWidthInt;
			}

			BT_CORE_TRACE << "  ** InteractionArea = None. Header" << std::endl;
			return InteractionArea::None;
		}

		auto itemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight) + m_module.m_viewport.InnerMargin * 2u;

		auto position = mousePosition.Y - m_module.m_viewport.BackgroundRect.Y + m_module.ScrollOffset.Y;
		int index = position / (int)itemHeight;

		if (index < m_module.List.Items.size())
		{
			BT_CORE_TRACE << "  ** InteractionArea = List. index = " << index << std::endl;
			return InteractionArea::List;
		}

		BT_CORE_TRACE << "  ** InteractionArea = None." << std::endl;
		return InteractionArea::None;
	}
}
