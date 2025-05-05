/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "LayoutNodes.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	LayoutNode::LayoutNode(LayoutNodeType type) :
		m_type(type)
	{
	}

	size_t LayoutNode::GetIndex() const
	{
		if (m_parentNode)
		{
			for (size_t i = 0; i < m_parentNode->m_children.size(); i++)
			{
				if (m_parentNode->m_children[i].get() == this)
					return i;
			}
		}
		return std::string::npos;
	}

	LayoutNode* LayoutNode::Find(const std::string& id)
	{
		return Find(id, this);
	}

	LayoutNode* LayoutNode::FindFirst(LayoutNodeType nodeType)
	{
		return FindFirst(nodeType, this);
	}

	LayoutNode* LayoutNode::Find(const std::string& id, LayoutNode* node)
	{
		if (node->GetId() == id)
			return node;

		for (auto& childPtr : node->m_children)
		{
			auto child = Find(id, childPtr.get());
			if (child)
			{
				return child;
			}
		}

		return nullptr;
	}

	LayoutNode* LayoutNode::FindFirst(LayoutNodeType nodeType, LayoutNode* node)
	{
		if (node->GetType() == nodeType)
			return node;

		for (auto& childPtr : node->m_children)
		{
			auto child = FindFirst(nodeType, childPtr.get());
			if (child)
			{
				return child;
			}
		}

		return nullptr;
	}

	void ContainerLayoutNode::CalculateAreas()
	{
		const auto& dpi = m_parentWindow->DPIScaleFactor;
		Number marginLeftNum = GetProperty<Number>("margin-left", {});
		Number marginRightNum = GetProperty<Number>("margin-right", {});
		Number marginTopNum = GetProperty<Number>("margin-top", {});
		Number marginBottomNum = GetProperty<Number>("margin-bottom", {});
		
		auto marginLeft = marginLeftNum.GetValue<int>(dpi);
		auto marginRight = marginRightNum.GetValue<int>(dpi);
		auto marginTop = marginTopNum.GetValue<int>(dpi);
		auto marginBottom = marginBottomNum.GetValue<int>(dpi);
		
		std::vector<bool> markedChildren(m_children.size(), false);
		std::vector<Rectangle> areas(m_children.size());

		auto parentArea = GetArea();
		auto remainArea = parentArea;

		int totalChildren = (int)m_children.size();
		auto fixedNodesCount = 0;

		for (size_t i = 0; i < m_children.size(); ++i)
		{
			auto& childNode = m_children[i];
			Rectangle childArea;

			if (m_isVertical)
			{
				childArea.Width = parentArea.Width;
				childNode->m_fixedWidth.Reset();
			}
			else
			{
				childArea.Height = parentArea.Height;
				childNode->m_fixedHeight.Reset();
			}

			auto dimensionType = m_isVertical ? "Height" : "Width";
			if (childNode->HasProperty<Number>(dimensionType))
			{
				auto dimensionNum = childNode->GetProperty<Number>(dimensionType);
				bool isPercentage = dimensionNum.isPercentage;
				uint32_t fixedSize = isPercentage ?
					static_cast<uint32_t>(dimensionNum.GetValue<double>() * (m_isVertical ? parentArea.Height : parentArea.Width) / 100.0) :
					static_cast<uint32_t>(dimensionNum.GetValue<int>(dpi));

				auto minDimensionType = m_isVertical ? "MinHeight" : "MinWidth";
				auto maxDimensionType = m_isVertical ? "MaxHeight" : "MaxWidth";

				if (childNode->HasProperty<Number>(minDimensionType))
				{
					auto dimensionNum = (uint32_t)childNode->GetProperty<Number>(minDimensionType).GetValue<int>(dpi);
					if (fixedSize < dimensionNum)
					{
						fixedSize = dimensionNum;
					}
				}
				if (childNode->HasProperty<Number>(maxDimensionType))
				{
					auto dimensionNum = (uint32_t)childNode->GetProperty<Number>(maxDimensionType).GetValue<int>(dpi);
					if (fixedSize > dimensionNum)
					{
						fixedSize = dimensionNum;
					}
				}

				if (m_isVertical)
				{
					childArea.Height = fixedSize;
					childNode->m_fixedHeight.SetValue((int)fixedSize);
					remainArea.Height -= fixedSize;
				}
				else
				{
					childArea.Width = fixedSize;
					childNode->m_fixedWidth.SetValue((int)fixedSize);
					remainArea.Width -= fixedSize;
				}

				markedChildren[i] = true;
				areas[i] = childArea;

				++fixedNodesCount;
			}
		}

		Point offset{ 0, 0 };
		int totalFreeCount = totalChildren - fixedNodesCount;
		uint32_t appliedPixels = 0;
		for (size_t i = 0; i < m_children.size(); ++i)
		{
			auto& childNode = m_children[i];
			Rectangle childArea{ };
			childArea.X = parentArea.X + marginLeft;
			childArea.Y = parentArea.Y + marginTop;
			
			if (markedChildren[i])
			{
				childArea.Width = areas[i].Width;
				childArea.Height = areas[i].Height;
			}
			else
			{
				childArea.Width = parentArea.Width;
				childArea.Height = parentArea.Height;
			}

			childArea.Width -= marginLeft;
			childArea.Height -= marginTop;

			if (!markedChildren[i])
			{
				if (m_isVertical)
				{
					uint32_t part = 0u;
					if (!childNode->m_fixedHeight.HasValue())
					{
						childNode->m_fixedHeight.isPercentage = true;
						childNode->m_fixedHeight.SetValue(1.0 / totalFreeCount);
						part = remainArea.Height / totalFreeCount;
					}
					else
					{
						if (childNode->m_fixedHeight.isPercentage)
						{
							auto partDouble = childNode->m_fixedHeight.GetValue<double>() * remainArea.Height;
							part = static_cast<uint32_t>(partDouble);
						}
						else
						{
							part = childNode->m_fixedHeight.GetValue<uint32_t>();
						}
					}
					appliedPixels += part;

					childArea.Height = part;
					childArea.Y += offset.Y;
					offset.Y += part;
				}
				else
				{
					uint32_t part = 0u;
					if (!childNode->m_fixedWidth.HasValue())
					{
						childNode->m_fixedWidth.isPercentage = true;
						childNode->m_fixedWidth.SetValue(1.0 / totalFreeCount);
						part = remainArea.Width / totalFreeCount;
					}
					else
					{
						if (childNode->m_fixedWidth.isPercentage)
						{
							auto partDouble = childNode->m_fixedWidth.GetValue<double>() * remainArea.Width;
							part = static_cast<uint32_t>(partDouble);
						}
						else
						{
							part = childNode->m_fixedWidth.GetValue<uint32_t>();
						}
					}
					appliedPixels += part;

					childArea.Width = part;
					childArea.X += offset.X;
					offset.X += part;
				}
				childArea.Width -= marginRight;
				childArea.Height -= marginBottom;
			}
			else
			{
				if (m_isVertical)
				{
					childArea.Y += offset.Y;
					offset.Y += areas[i].Height;
				}
				else
				{
					childArea.X += offset.X;
					offset.X += areas[i].Width;
				}
			}

			if (i == m_children.size() - 1)
			{
				if (m_isVertical && appliedPixels > 0 && remainArea.Height > appliedPixels)
				{
					childArea.Height += remainArea.Height - appliedPixels;
				}
				else if (!m_isVertical && appliedPixels > 0 && remainArea.Width > appliedPixels)
				{
					childArea.Width += remainArea.Width - appliedPixels;
				}
			}

			childNode->SetArea(childArea);
			childNode->CalculateAreas();
		}
	}

	ContainerLayoutNode::ContainerLayoutNode(LayoutNodeType type) :
		LayoutNode(type)
	{
	}

	ContainerLayoutNode::ContainerLayoutNode(bool isVertical) : 
		LayoutNode(LayoutNodeType::Container),
		m_isVertical(isVertical)
	{
	}

	void ContainerLayoutNode::AddChild(std::unique_ptr<LayoutNode>&& child)
	{
		m_children.emplace_back(std::move(child));
	}

	LeafLayoutNode::LeafLayoutNode() : 
		LayoutNode(LayoutNodeType::Leaf)
	{
	}

	void LeafLayoutNode::CalculateAreas()
	{
		if (!m_window)
			return;

		GUI::MoveWindow(m_window, GetArea(), false);
	}

	void LeafLayoutNode::AddWindow(Window* window)
	{
		m_window = window;
	}

	SplitterLayoutNode::SplitterLayoutNode(bool isVertical) :
		LayoutNode(LayoutNodeType::Splitter),
		m_isVertical(isVertical)
	{
		auto propertyName = isVertical ? "Height" : "Width";
		SetProperty(propertyName, Number{ SplitterLayoutNode::SizeInPixels });
	}

	void SplitterLayoutNode::CalculateAreas()
	{
		if (!m_splitter)
		{
			auto splitterArea = GetArea();
			m_containerNode = reinterpret_cast<ContainerLayoutNode*>(m_parentNode);

			m_splitter = std::make_unique<SplitterLayoutControl>(m_parentWindow, splitterArea);
			m_splitter->GetEvents().MouseDown.Connect([this](const ArgMouse& args)
			{
				if (!args.ButtonState.LeftButton)
					return;

				GUI::Capture(m_splitter->Handle());

				m_splitterBeginRect = m_splitter->GetArea();
				m_mousePositionOffset = -args.Position;

				m_leftArea = GetPrev()->GetArea();
				m_rightArea = m_nextNode->GetArea();

				m_isSplitterMoving = true;
			});

			m_splitter->GetEvents().MouseEnter.Connect([this](const ArgMouse& args)
			{
				GUI::ChangeCursor(*m_splitter, m_isVertical ? Cursor::SizeNS : Cursor::SizeWE);
			});

			m_splitter->GetEvents().MouseLeave.Connect([this](const ArgMouse& args)
			{
				if (m_isSplitterMoving)
					return;

				GUI::ChangeCursor(*m_splitter, Cursor::Default);
			});

			m_splitter->GetEvents().MouseMove.Connect([this](const ArgMouse& args)
			{
				if (!m_isSplitterMoving)
					return;
				
				auto delta = GUI::GetAbsoluteRootPosition(m_splitter->Handle()) + args.Position - m_splitterBeginRect + m_mousePositionOffset;
				
				auto splitterArea = GetArea();
				auto newSplitterArea = splitterArea;
				auto fixedSplitterSize = m_isVertical ? splitterArea.Height : splitterArea.Width;

				auto& deltaValue = m_isVertical ? delta.Y : delta.X;

				auto newLeftArea = m_leftArea;
				auto newRightArea = m_rightArea;

				auto& leftAreaValue = m_isVertical ? m_leftArea.Height : m_leftArea.Width;
				auto& rightAreaValue = m_isVertical ? m_rightArea.Height : m_rightArea.Width;

				auto& newLeftAreaValue = m_isVertical ? newLeftArea.Height : newLeftArea.Width;
				auto& newRightAreaValue = m_isVertical ? newRightArea.Height : newRightArea.Width;
				
				auto& leftPos = m_isVertical ? newLeftArea.Y : newLeftArea.X;
				auto& rightPos = m_isVertical ? newRightArea.Y : newRightArea.X;

				auto containerArea = m_containerNode->GetArea();
				auto splitterCount = (uint32_t)(m_containerNode->m_children.size() - 1) / 2;
				fixedSplitterSize *= splitterCount;
				Size fixedSize{ fixedSplitterSize, fixedSplitterSize };

				int leftLimit = (std::max)(0, static_cast<int>(leftAreaValue) + deltaValue);
				leftLimit = (std::min)(leftLimit, static_cast<int>(leftAreaValue + rightAreaValue));
				newLeftAreaValue = static_cast<uint32_t>(leftLimit);

				rightPos = (std::max)(leftPos, rightPos + deltaValue);
				int rightLimit = (std::max)(0, static_cast<int>(newRightAreaValue) - deltaValue);
				rightLimit = (std::min)(rightLimit, (int)(leftAreaValue + newRightAreaValue));
				newRightAreaValue = static_cast<uint32_t>(rightLimit);

				GetPrev()->SetAreaWithPercentage(newLeftArea, containerArea, fixedSize);
				m_nextNode->SetAreaWithPercentage(newRightArea, containerArea, fixedSize);

				auto& prevFixedPercent = m_isVertical ? GetPrev()->m_fixedHeight : GetPrev()->m_fixedWidth;
				auto& nextFixedPercent = m_isVertical ? m_nextNode->m_fixedHeight : m_nextNode->m_fixedWidth;

				auto& newSplitterAreaPos = m_isVertical ? newSplitterArea.Y : newSplitterArea.X;
				auto& containerAreaValue = m_isVertical ? containerArea.Height : containerArea.Width;
				if (newLeftAreaValue + newRightAreaValue != leftAreaValue + rightAreaValue)
				{
					int newValue = newLeftAreaValue + newRightAreaValue;
					int oldValue = leftAreaValue + rightAreaValue;
					auto diff = oldValue - newValue;

					double diffPercent = (double)diff / (containerAreaValue - fixedSplitterSize);
					auto prevPercent = prevFixedPercent.GetValue<double>() + diffPercent * 0.5;
					auto nextPercent = nextFixedPercent.GetValue<double>() + diffPercent * 0.5;

					prevFixedPercent.SetValue(prevPercent);
					nextFixedPercent.SetValue(nextPercent);
				}
				
				newSplitterAreaPos += deltaValue;

				SetArea(newSplitterArea);

				BT_CORE_TRACE << " -- CHANGING..." << std::endl;
				m_containerNode->CalculateAreas();

				auto windowToUpdate = m_containerNode->GetParentWindow()->FindFirstNonPanelAncestor();
				GUI::UpdateTree(windowToUpdate);
				BT_CORE_TRACE << " -- CHANGED..." << std::endl;
			});

			m_splitter->GetEvents().MouseUp.Connect([this](const ArgMouse& args)
			{
				if (!m_isSplitterMoving)
					return;

				m_isSplitterMoving = false;
				GUI::ReleaseCapture(m_splitter->Handle());
			});
		}
		else
		{
			auto splitterArea = GetArea();
			GUI::MoveWindow(m_splitter->Handle(), splitterArea);
		}
	}

	void SplitterLayoutNode::SetOrientation(bool isVertical)
	{
		m_isVertical = isVertical;
	}

	SplitterLayoutControl::SplitterLayoutControl(Window* parent, const Rectangle& rectangle, bool visible) :
		Panel(parent, false, rectangle, visible)
	{
#if BT_DEBUG
		m_handle->Name = "SplitterLayoutControl";
#endif
	}

	DockLayoutNode::DockLayoutNode() :
		LayoutNode(LayoutNodeType::Dock)
	{
	}

	void DockLayoutNode::CalculateAreas()
	{
		if (m_children.empty())
		{
			return;
		}
		auto area = GetArea();
		m_children[0]->SetArea(area);
		m_children[0]->CalculateAreas();
	}

	DockPaneLayoutNode::DockPaneLayoutNode() :
		LayoutNode(LayoutNodeType::DockPane)
	{
	}

	void DockPaneLayoutNode::AddTab(const std::string& id, ControlBase* control)
	{
		m_dockArea->AddTab(id, control);
	}

	void DockPaneLayoutNode::AddPane(DockPaneLayoutNode* paneNode)
	{
		DockArea& dockArea = *paneNode->m_dockArea;
		for (size_t i = 0; i < dockArea.m_tabBarPanels.size(); i++)
		{
			auto paneTab = reinterpret_cast<DockPaneTabLayoutNode*>(paneNode->m_children[i].get());

			auto tabId = paneTab->m_tabId.substr(paneNode->m_paneId.size() + 1);
			m_dockArea->AddTab(tabId, dockArea.m_tabBarPanels[i]);
		}
	}

	void DockPaneLayoutNode::AddWindow(Window* window)
	{
	}

	void DockPaneLayoutNode::CalculateAreas()
	{
		auto area = GetArea();
		if (m_dockArea && !m_dockArea->IsFloating())
		{
			GUI::MoveWindow(m_dockArea->Handle(), area, false);
		}

		for (auto& child : m_children)
		{
			child->SetArea(area);
			child->CalculateAreas();
		}
	}

	void DockPaneLayoutNode::NotifyFloat()
	{
		m_dockLayoutEvents->NotifyFloat(this);
	}

	void DockPaneLayoutNode::NotifyMove()
	{
		m_dockLayoutEvents->NotifyMove(this);
	}

	void DockPaneLayoutNode::NotifyMoveStopped()
	{
		m_dockLayoutEvents->NotifyMoveStopped(this);
	}

	void DockPaneLayoutNode::RequestClose()
	{
		m_dockLayoutEvents->RequestClose(this);
	}

	void DockAreaCaptionReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->MenuBackground, true);

		Point textPos{ window->ToScale(5), 0 };
		textPos.Y = (int)window->ClientSize.Height - (int)graphics.GetTextExtent().Height;
		textPos.Y >>= 1;
		graphics.DrawString(textPos, m_control->GetCaption(), window->Appearance->Foreground);

		if (!m_paneInfo || !m_paneInfo->ShouldShowCloseButton())
		{
			return;
		}
		auto color = window->Appearance->Background;
		if (m_buttonStatus == State::None)
		{
			color = window->Appearance->ButtonBackground;
		}
		else if (m_buttonStatus == State::Hovered)
		{
			color = window->Appearance->ButtonHighlightBackground;
		}
		else if (m_buttonStatus == State::Pressed)
		{
			color = window->Appearance->ButtonPressedBackground;
		}
		auto two = window->ToScale(2);
		graphics.DrawRoundRectBox(m_buttonRect, color, window->Appearance->BoxBorderColor, true);

		auto x = L"x";
		Point textExtent = graphics.GetTextExtent(x);
		Point windowSize{ (int)m_buttonRect.Width, (int)m_buttonRect.Height };
		auto center = windowSize - textExtent;
		center /= 2;
		center.Y -= two;
		graphics.DrawString({ center.X + m_buttonRect.X, center.Y + m_buttonRect.Y }, x, window->Appearance->Foreground);
	}

	void DockAreaCaptionReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (!m_paneInfo->ShouldShowCloseButton())
			return;

		m_mouseDownCloseButton = m_buttonRect.IsInside(args.Position) && args.ButtonState.LeftButton;
		m_clickedCloseButton = false;
		if (!m_mouseDownCloseButton)
		{
			return;
		}
		m_buttonStatus = State::Pressed;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	void DockAreaCaptionReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (!m_paneInfo->ShouldShowCloseButton())
			return;

		auto prevStatus = m_buttonStatus;
		m_buttonStatus = m_buttonRect.IsInside(args.Position) ? (m_mouseDownCloseButton ? State::Pressed : State::Hovered) : State::None;

		if (prevStatus == m_buttonStatus)
			return;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	void DockAreaCaptionReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (!m_paneInfo->ShouldShowCloseButton())
			return;

		m_clickedCloseButton = m_mouseDownCloseButton && m_buttonRect.IsInside(args.Position) && args.ButtonState.LeftButton;

		m_mouseDownCloseButton = false;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	void DockAreaCaptionReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		auto window = m_control->Handle();
		auto buttonSize = window->ToScale(DockAreaCaptionButtonSize);
		auto offsetY = ((int)args.NewSize.Height - buttonSize) >> 1;

		auto two = window->ToScale(2);
		m_buttonRect.X = args.NewSize.Width - buttonSize - offsetY - two;
		m_buttonRect.Y = offsetY;
		m_buttonRect.Height = buttonSize;
		m_buttonRect.Width = buttonSize;

		//Update(graphics);
		//GUI::MarkAsUpdated(*m_control);
	}

	void DockArea::AddTab(const std::string& id, ControlBase* control)
	{
		bool isFirstTab = m_tabBar->Count() == 0;
		auto panel = m_tabBar->PushBack2(id, control); 
		m_tabBarPanels.push_back(panel);
		if (isFirstTab)
		{
			m_caption->SetCaption(id);
		}
	}

	void DockArea::Create(Window* parent, PaneInfo* paneInfo)
	{
		m_hostWindow = parent;
		Control<ControlReactor>::Create(parent, false, { 0,0,1,1 }, true, true);
#if BT_DEBUG
		m_handle->Name = "DockArea-" + paneInfo->id;
#endif
		m_caption = std::make_unique<DockAreaCaption>();
		m_caption->Create(*this, false, {0,0,1,1}, paneInfo->showCaption);
#if BT_DEBUG
		m_caption->Handle()->Name = "DockAreaCaption-" + paneInfo->id;
#endif
		m_caption->SetCaption(L"caption 1");
		m_caption->SetPaneInfo(paneInfo);

		m_paneInfo = paneInfo;

		this->GetEvents().Resize.Connect([this](const ArgResize& args)
		{
			//BT_CORE_DEBUG << " dock area / Resize = " << this->GetArea() << std::endl;
			if (m_paneInfo->showCaption)
			{
				auto captionHeight = Handle()->ToScale(18u);
				m_caption->SetArea({ 0, 0, args.NewSize.Width, captionHeight });
				m_tabBar->SetArea({ 0, (int)captionHeight, args.NewSize.Width, args.NewSize.Height - captionHeight });
			}
			else
			{
				m_tabBar->SetArea({ 0, 0, args.NewSize.Width, args.NewSize.Height });
			}
		});

		m_caption->GetEvents().MouseDown.Connect([this](const ArgMouse& args)
		{
			GUI::Capture(*m_caption);

			if (m_caption->WasPressedCloseButton())
				return;

			m_mouseInteraction.m_dragStarted = true;

			m_mouseInteraction.m_dragStartPos = GUI::GetScreenMousePosition();
			m_mouseInteraction.m_dragStartLocalPos = IsFloating() ? API::GetWindowPosition(m_nativeContainer->Handle()->RootHandle) : this->GetPosition();
			m_mouseInteraction.m_dragStartCaptionPos = args.Position;

			m_savedDPI = this->Handle()->DPI;
		});

		m_caption->GetEvents().MouseMove.Connect([this](const ArgMouse& args)
		{
			if (!m_mouseInteraction.m_dragStarted)
				return;

			auto dockAreaWindow = this->Handle();
			auto screenMousePos = GUI::GetScreenMousePosition();
			if (!IsFloating())
			{
				auto floatingThreshold = dockAreaWindow->ToScale(4);
				if (std::abs(m_mouseInteraction.m_dragStartPos.X - screenMousePos.X) > floatingThreshold ||
					std::abs(m_mouseInteraction.m_dragStartPos.Y - screenMousePos.Y) > floatingThreshold)
				{
					auto pointInScreen = dockAreaWindow->Position;
					auto dockAreaSize = this->GetSize();

					Rectangle formRect;
					formRect.X = pointInScreen.X;
					formRect.Y = pointInScreen.Y;
					formRect.Width = dockAreaSize.Width;
					formRect.Height = dockAreaSize.Height;

					m_nativeContainer = std::make_unique<Form>(m_hostWindow, formRect, FormStyle::Float());
					auto nativeWindow = m_nativeContainer->Handle();
#if BT_DEBUG
					nativeWindow->Name = "DockFloat-" + m_paneInfo->id;
#endif

					GUI::SetParentWindow(dockAreaWindow, nativeWindow);
					this->SetPosition({ 0, 0 });

					m_nativeContainer->GetEvents().Resize.Connect([this](const ArgResize& args)
					{
						this->SetSize({ args.NewSize.Width, args.NewSize.Height });
					});

					m_mouseInteraction.m_dragStartLocalPos.X -= static_cast<int>(nativeWindow->BorderSize.Width / 2) - (screenMousePos.X - m_mouseInteraction.m_dragStartPos.X);
					m_mouseInteraction.m_dragStartLocalPos.Y -= static_cast<int>(nativeWindow->BorderSize.Height / 2) - (screenMousePos.Y - m_mouseInteraction.m_dragStartPos.Y);
					m_mouseInteraction.m_dragStartPos = GUI::GetScreenMousePosition();

					m_nativeContainer->Show();

					m_mouseInteraction.m_hasChanged = true;
					m_eventsNotifier->NotifyFloat();
				}
			}
			else
			{
				auto newPosition = screenMousePos - m_mouseInteraction.m_dragStartPos;
				newPosition += m_mouseInteraction.m_dragStartLocalPos;

				if (m_savedDPI != m_nativeContainer->Handle()->DPI)
				{
					float adjustScaleFactor = (float)m_nativeContainer->Handle()->DPI / m_savedDPI;
					m_mouseInteraction.m_dragStartCaptionPos.X = static_cast<int>(m_mouseInteraction.m_dragStartCaptionPos.X * adjustScaleFactor);
					m_mouseInteraction.m_dragStartCaptionPos.Y = static_cast<int>(m_mouseInteraction.m_dragStartCaptionPos.Y * adjustScaleFactor);

					auto upperLeftOffset = API::GetPointScreenToClient(m_nativeContainer->Handle()->RootHandle, screenMousePos);

					m_mouseInteraction.m_dragStartPos = screenMousePos;
					m_mouseInteraction.m_dragStartLocalPos = API::GetWindowPosition(m_nativeContainer->Handle()->RootHandle) + upperLeftOffset - m_mouseInteraction.m_dragStartCaptionPos;
					
					m_savedDPI = m_nativeContainer->Handle()->DPI;
				}
				
				m_mouseInteraction.m_hasChanged = true;
				GUI::MoveWindow(*m_nativeContainer, newPosition);

				m_eventsNotifier->NotifyMove();
			}
		});

		m_caption->GetEvents().MouseUp.Connect([this](const ArgMouse& args)
		{
			m_mouseInteraction.m_dragStarted = false;
			GUI::ReleaseCapture(*m_caption);

			if (m_caption->HaveClickedCloseButton())
			{
				m_tabBarPanels.erase(m_tabBarPanels.begin() + m_tabBar->GetSelectedIndex());
				m_eventsNotifier->RequestClose();
				return;
			}

			if (m_mouseInteraction.m_hasChanged)
			{
				m_eventsNotifier->NotifyMoveStopped();
			}
		});

		m_tabBar = std::make_unique<TabBar>(this->Handle(), Rectangle{0,0,1u,1u});
		m_tabBar->SetTabPosition(TabBarPosition::Bottom);

		m_tabBar->GetEvents().TabChanged.Connect([this](const ArgTabBar& args)
		{
			m_caption->SetCaption(args.id);
		});

		if (!paneInfo->showCaption)
			m_caption->Hide();
	}

	void DockArea::Dock()
	{
		GUI::SetParentWindow(this->Handle(), m_hostWindow);
		m_nativeContainer.reset();
	}

	int DockArea::GetTabSelectedIndex() const
	{
		return m_tabBar->GetSelectedIndex();
	}

	void DockAreaCaption::SetPaneInfo(PaneInfo* paneInfo)
	{
		m_reactor.m_paneInfo = paneInfo;
	}

	bool DockAreaCaption::WasPressedCloseButton() const
	{
		return m_reactor.m_mouseDownCloseButton;
	}

	bool DockAreaCaption::HaveClickedCloseButton() const
	{
		return m_reactor.m_clickedCloseButton;
	}

	DockPaneTabLayoutNode::DockPaneTabLayoutNode() :
		LayoutNode(LayoutNodeType::DockPaneTab)
	{
	}

	void DockPaneTabLayoutNode::CalculateAreas()
	{
	}
}