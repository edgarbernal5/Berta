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

	LayoutNode* LayoutNode::Find(std::string_view id)
	{
		return Find(id, this);
	}

	LayoutNode* LayoutNode::FindFirst(LayoutNodeType nodeType)
	{
		return FindFirst(nodeType, this);
	}

	LayoutNode* LayoutNode::Find(std::string_view id, LayoutNode* node)
	{
		if (node->GetId() == id)
		{
			return node;
		}

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
		auto parentArea = GetArea();
		auto remainArea = parentArea;
		const float dpi = m_ownerWindow->DPIScaleFactor;

		std::vector<bool> markedChildren(m_children.size(), false);
		std::vector<Rectangle> areas(m_children.size());
		int fixedNodesCount = 0;

		// FASE 1: Calcular los nodos que tienen un tamaño fijo (Pixels o Porcentaje)
		ProcessFixedChildren(parentArea, remainArea, areas, markedChildren, fixedNodesCount, dpi);

		// FASE 2: Distribuir el espacio restante entre los nodos dinámicos
		ProcessDynamicChildren(parentArea, remainArea, areas, markedChildren, fixedNodesCount, dpi);
	}

	ContainerLayoutNode::ContainerLayoutNode(LayoutNodeType type) :
		LayoutNode(type)
	{
	}

	void ContainerLayoutNode::ProcessFixedChildren(const Rectangle& parentArea, Rectangle& remainArea, std::vector<Rectangle>& areas, std::vector<bool>& markedChildren, int& fixedNodesCount, float dpi)
{
    const std::string_view dimType    = m_isVertical ? "Height"    : "Width";
    const std::string_view minDimType = m_isVertical ? "MinHeight" : "MinWidth";
    const std::string_view maxDimType = m_isVertical ? "MaxHeight" : "MaxWidth";

    for (size_t i = 0; i < m_children.size(); ++i)
    {
       auto& childNode = m_children[i];
       Rectangle childArea;

       auto& crossDim       = m_isVertical ? childArea.Width : childArea.Height;
       auto& parentCrossDim = m_isVertical ? parentArea.Width : parentArea.Height;
       crossDim = parentCrossDim;

       // 1. La Verdad Absoluta: Identificamos su naturaleza real
       bool isSplitter = (childNode->GetType() == LayoutNodeType::Splitter);
       auto* dimensionDim = childNode->TryGetProperty<Berta::Dimension>(dimType);

       // Entra si es un Splitter O si es un panel con tamaño estático en píxeles
       if (isSplitter || (dimensionDim && !dimensionDim->IsPercentage()))
       {
          uint32_t fixedSize = 0;

          if (isSplitter) {
             // El muro recobra su identidad sin importar qué diga el mapa de propiedades
             fixedSize = static_cast<uint32_t>(SplitterLayoutNode::SizeInPixels * dpi); 
          } else {
             fixedSize = static_cast<uint32_t>(dimensionDim->value * dpi);
          }

          // Los límites Min/Max solo aplican a paneles, no a splitters
          if (!isSplitter) {
              if (auto* minDim = childNode->TryGetProperty<Berta::Dimension>(minDimType)) {
                 fixedSize = std::max<uint32_t>(fixedSize, static_cast<uint32_t>(minDim->value * dpi));
              }
              if (auto* maxDim = childNode->TryGetProperty<Berta::Dimension>(maxDimType)) {
                 fixedSize = std::min<uint32_t>(fixedSize, static_cast<uint32_t>(maxDim->value * dpi));
              }
          }

          auto& mainDim       = m_isVertical ? childArea.Height : childArea.Width;
          auto& remainMainDim = m_isVertical ? remainArea.Height : remainArea.Width;

          mainDim = fixedSize;
          
          // 2. Escudo contra Underflow: Evita valores negativos si el layout colapsa
          if (fixedSize > remainMainDim) remainMainDim = 0; 
          else remainMainDim -= fixedSize;

          markedChildren[i] = true;
          areas[i] = childArea;
          ++fixedNodesCount;
       }
    }
}

	void ContainerLayoutNode::ProcessDynamicChildren(const Rectangle& parentArea, const Rectangle& remainArea, const std::vector<Rectangle>& areas, const std::vector<bool>& markedChildren, int fixedNodesCount, float dpi)
	{
		auto getMargin = [&](std::string_view name) -> int {
			auto* dim = TryGetProperty<Berta::Dimension>(name);
			return dim ? static_cast<int>(dim->value * dpi) : 0;
		};

		int marginLeft   = getMargin("margin-left");
		int marginRight  = getMargin("margin-right");
		int marginTop    = getMargin("margin-top");
		int marginBottom = getMargin("margin-bottom");

		int totalFreeCount = static_cast<int>(m_children.size()) - fixedNodesCount;
		if (totalFreeCount <= 0) return; // Si no hay nodos libres, el maestro descansa.

		// --- FASE 1: LA BALANZA DE LA VERDAD (Normalización) ---
		double totalWeight = 0.0;
		std::vector<double> dynamicWeights(m_children.size(), 0.0);

		for (size_t i = 0; i < m_children.size(); ++i)
		{
			if (!markedChildren[i])
			{
				auto& childNode = m_children[i];
				auto* weightDim = childNode->TryGetProperty<Berta::Dimension>("LayoutWeight");
            
				// Si el panel no tiene peso (ej. recién dockeado), recibe su parte justa
				double weight = weightDim ? weightDim->value : (1.0 / totalFreeCount);
				dynamicWeights[i] = weight;
				totalWeight += weight;
			}
		}

		if (totalWeight <= 0.0)
		{
			totalWeight = 1.0;
		}

		double exactOffsetMain = 0.0;
		int currentOffsetMain = 0; 

		for (size_t i = 0; i < m_children.size(); ++i)
		{
			auto& childNode = m_children[i];
			Rectangle childArea = parentArea;
       
			childArea.X += marginLeft;
			childArea.Y += marginTop;
			childArea.Width  -= (marginLeft + marginRight);
			childArea.Height -= (marginTop + marginBottom);

			auto& mainDim       = m_isVertical ? childArea.Height : childArea.Width;
			auto& mainPos       = m_isVertical ? childArea.Y : childArea.X;
			auto& remainMainDim = m_isVertical ? remainArea.Height : remainArea.Width;

			if (markedChildren[i])
			{
				uint32_t savedMainDim = m_isVertical ? areas[i].Height : areas[i].Width;
				childArea.Width = areas[i].Width;
				childArea.Height = areas[i].Height;
          
				mainPos += currentOffsetMain;
          
				exactOffsetMain += savedMainDim;
				currentOffsetMain = static_cast<int>(std::round(exactOffsetMain));
			}
			else
			{
				double normalizedFraction = dynamicWeights[i] / totalWeight;
				double exactSize = normalizedFraction * remainMainDim;
				double nextExactOffset = exactOffsetMain + exactSize;
          
				uint32_t part = static_cast<uint32_t>(std::round(nextExactOffset) - currentOffsetMain);

				mainDim = part;
				mainPos += currentOffsetMain;
          
				exactOffsetMain = nextExactOffset;
				currentOffsetMain = static_cast<int>(std::round(exactOffsetMain));
          
				childNode->SetProperty("LayoutWeight", Berta::Dimension{ normalizedFraction, Berta::DimensionUnit::Percentage });
			}

			childNode->SetArea(childArea);
			childNode->CalculateAreas();
		}
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
		{
			return;
		}

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
		std::string_view propertyName = isVertical ? "Height" : "Width";
    
		SetProperty(propertyName, Berta::Dimension{ 
			static_cast<double>(SplitterLayoutNode::SizeInPixels), 
			Berta::DimensionUnit::Pixels 
		});
	}

	void SplitterLayoutNode::CalculateAreas()
	{
		EnsureControlCreated();
		
		GUI::MoveWindow(m_splitter->Handle(), GetArea(), false);
	}

	void SplitterLayoutNode::SetOrientation(bool isVertical)
	{
		m_isVertical = isVertical;
	}

	void SplitterLayoutNode::EnsureControlCreated()
	{
		if (m_splitter) return; // Si ya existe, no hacemos nada

		m_containerNode = static_cast<ContainerLayoutNode*>(m_parentNode);

		m_splitter = std::make_unique<SplitterLayoutControl>(m_ownerWindow, GetArea());
    
		auto& events = m_splitter->GetEvents();
		events.MouseDown.Connect([this](const ArgMouse& args)  { OnMouseDown(args); });
		events.MouseMove.Connect([this](const ArgMouse& args)  { OnMouseMove(args); });
		events.MouseUp.Connect([this](const ArgMouse& args)    { OnMouseUp(); });
		events.MouseEnter.Connect([this](const ArgMouse& args) { OnMouseEnter(); });
		events.MouseLeave.Connect([this](const ArgMouse& args) { OnMouseLeave(); });
	}

	void SplitterLayoutNode::OnMouseDown(const ArgMouse& args)
	{
		if (!args.ButtonState.LeftButton) return;

		GUI::Capture(m_splitter->Handle());

		m_splitterBeginRect = m_splitter->GetArea();
		m_mousePositionOffset = -args.Position;

		m_leftArea = m_prevNode->GetArea();
		m_rightArea = m_nextNode->GetArea();

		m_isSplitterMoving = true;
	}

	void SplitterLayoutNode::OnMouseMove(const ArgMouse& args)
	{
		if (!m_isSplitterMoving)
		{
			return;
		}
		auto delta = GUI::GetWindowRootPosition(m_splitter->Handle()) + args.Position - m_splitterBeginRect + m_mousePositionOffset;
   
		auto newSplitterArea   = GetArea();
		auto fixedSplitterSize = m_isVertical ? newSplitterArea.Height : newSplitterArea.Width;
		auto deltaValue        = m_isVertical ? delta.Y : delta.X;

		auto newLeftArea  = m_leftArea;
		auto newRightArea = m_rightArea;

		auto leftAreaValue  = m_isVertical ? m_leftArea.Height  : m_leftArea.Width;
		auto rightAreaValue = m_isVertical ? m_rightArea.Height : m_rightArea.Width;

		auto& newLeftAreaValue  = m_isVertical ? newLeftArea.Height  : newLeftArea.Width;
		auto& newRightAreaValue = m_isVertical ? newRightArea.Height : newRightArea.Width;
   
		auto leftPos   = m_isVertical ? newLeftArea.Y  : newLeftArea.X;
		auto& rightPos = m_isVertical ? newRightArea.Y : newRightArea.X;

		auto containerArea = m_containerNode->GetArea();
		auto splitterCount = static_cast<uint32_t>((m_containerNode->m_children.size() - 1) / 2);
		Size fixedSize{ fixedSplitterSize * splitterCount, fixedSplitterSize * splitterCount };

		int totalAvailableSpace = static_cast<int>(leftAreaValue + rightAreaValue);
    
		int leftLimit = static_cast<int>(leftAreaValue) + deltaValue;
		newLeftAreaValue = static_cast<uint32_t>(std::clamp(leftLimit, 0, totalAvailableSpace));

		newRightAreaValue = static_cast<uint32_t>(totalAvailableSpace - static_cast<int>(newLeftAreaValue));
    
		rightPos = leftPos + static_cast<int>(newLeftAreaValue);

		m_prevNode->SetAreaWithPercentage(newLeftArea, containerArea, fixedSize, m_isVertical);
		m_nextNode->SetAreaWithPercentage(newRightArea, containerArea, fixedSize, m_isVertical);

		auto& newSplitterAreaPos = m_isVertical ? newSplitterArea.Y : newSplitterArea.X;
		newSplitterAreaPos = leftPos + static_cast<int>(newLeftAreaValue);

		SetArea(newSplitterArea);

		m_containerNode->CalculateAreas();

		API::RefreshWindow(m_ownerWindow->RootHandle);
		//API::UpdateWindow(m_ownerWindow->RootHandle);
	}

	void SplitterLayoutNode::OnMouseUp()
	{
		if (!m_isSplitterMoving) return;

		m_isSplitterMoving = false;
		GUI::ReleaseCapture(m_splitter->Handle());
	}

	void SplitterLayoutNode::OnMouseEnter()
	{
		GUI::ChangeCursor(*m_splitter, m_isVertical ? Cursor::SizeNS : Cursor::SizeWE);
	}

	void SplitterLayoutNode::OnMouseLeave()
	{
		if (!m_isSplitterMoving)
		{
			GUI::ChangeCursor(*m_splitter, Cursor::Default);
		}
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
		BT_ASSERT(m_children.size() == 1, "DockLayoutNode must have exactly 1 child.");
		
		auto area = GetArea();
		m_children[0]->SetArea(area);
		m_children[0]->CalculateAreas();
	}

	DockPaneLayoutNode::DockPaneLayoutNode() :
		LayoutNode(LayoutNodeType::DockPane)
	{
	}

	void DockPaneLayoutNode::AddTab(std::string_view id, std::unique_ptr<ControlBase> control)
	{
		m_dockArea->AddTab(std::string(id), std::move(control));
	}

	void DockPaneLayoutNode::AppendPane(DockPaneLayoutNode* paneNode)
	{
		if (!paneNode || !paneNode->m_dockArea)
		{
			return;
		}
		
		DockArea& sourceDockArea = *paneNode->m_dockArea;
		for (size_t i = 0; i < sourceDockArea.m_tabBarPanels.size(); i++)
		{
			auto paneTab = static_cast<DockPaneTabLayoutNode*>(paneNode->m_children[i].get());

			// FIX: Evitamos la bomba de tiempo (crashes por std::out_of_range)
			size_t prefixLen = paneNode->m_paneId.size() + 1;
			std::string tabId = (paneTab->m_tabId.size() > prefixLen) 
								? paneTab->m_tabId.substr(prefixLen) 
								: paneTab->m_tabId;

			sourceDockArea.m_tabBar->Detach(i);
			m_dockArea->AddTab(tabId, std::move(sourceDockArea.m_tabBarPanels[i].ControlPtr));
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
		DockPaneLayoutNode* self = this;
		Events.OnFloat.Emit(self);
	}

	void DockPaneLayoutNode::NotifyMove()
	{
		DockPaneLayoutNode* self = this;
		Events.OnMove.Emit(self);
	}

	void DockPaneLayoutNode::NotifyMoveStarted()
	{
		DockPaneLayoutNode* self = this;
		Events.OnMoveStarted.Emit(self);
	}

	void DockPaneLayoutNode::NotifyMoveStopped()
	{
		DockPaneLayoutNode* self = this;
		Events.OnMoveStopped.Emit(self);
	}

	void DockPaneLayoutNode::RequestClose()
	{
		DockPaneLayoutNode* self = this;
		Events.OnRequestClose.Emit(self);
	}

	void DockAreaCaptionReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		graphics.FillRectangle(window->ClientSize.ToRectangle(), window->Appearance->MenuBackground);

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

		std::wstring x = L"x";
		Point textExtent = graphics.GetTextExtent(x);
		Point windowSize{ static_cast<int>(m_buttonRect.Width), static_cast<int>(m_buttonRect.Height) };
		auto center = windowSize - textExtent;
		center /= 2;
		center.Y -= two;
		graphics.DrawString({ center.X + m_buttonRect.X, center.Y + m_buttonRect.Y }, x, window->Appearance->Foreground);
	}

	void DockAreaCaptionReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (!m_paneInfo->ShouldShowCloseButton())
		{
			return;
		}

		m_mouseDownCloseButton = m_buttonRect.Contains(args.Position) && args.ButtonState.LeftButton;
		m_clickedCloseButton = false;
		if (!m_mouseDownCloseButton)
		{
			return;
		}
		m_buttonStatus = State::Pressed;

		GUI::MarkAsNeedUpdate(*m_control);
	}

	void DockAreaCaptionReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (!m_paneInfo->ShouldShowCloseButton())
		{
			return;
		}

		auto prevStatus = m_buttonStatus;
		m_buttonStatus = m_buttonRect.Contains(args.Position) ? (m_mouseDownCloseButton ? State::Pressed : State::Hovered) : State::None;

		if (prevStatus == m_buttonStatus)
		{
			return;
		}

		GUI::MarkAsNeedUpdate(*m_control);
	}

	void DockAreaCaptionReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (!m_paneInfo->ShouldShowCloseButton())
		{
			return;
		}

		m_clickedCloseButton = m_mouseDownCloseButton && m_buttonRect.Contains(args.Position) && args.ButtonState.LeftButton;

		m_mouseDownCloseButton = false;

		GUI::MarkAsNeedUpdate(*m_control);
	}

	void DockAreaCaptionReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		auto window = m_control->Handle();
		auto buttonSize = window->ToScale(DOCK_AREA_CAPTION_BUTTON_SIZE);
		auto offsetY = (static_cast<int>(args.NewSize.Height) - buttonSize) >> 1;

		auto two = window->ToScale(2);
		m_buttonRect.X = static_cast<int>(args.NewSize.Width) - buttonSize - offsetY - two;
		m_buttonRect.Y = offsetY;
		m_buttonRect.Height = buttonSize;
		m_buttonRect.Width = buttonSize;
	}
	
	void DockArea::AddTab(const std::string& id, std::unique_ptr<ControlBase> control)
	{
		bool isFirstTab = m_tabBar->Count() == 0;
		m_tabBar->PushBack(id, control->Handle());
		m_tabBarPanels.emplace_back().ControlPtr = std::move(control);
		
		if (isFirstTab)
		{
			m_caption->SetCaption(id);
		}
	}

	void DockArea::Create(Window* parent, PaneInfo* paneInfo)
	{
		m_hostWindow = parent;
		Control::Create(parent, false, { 0,0,1,1 }, true);
		
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
				m_tabBar->SetArea({ 0, static_cast<int>(captionHeight), args.NewSize.Width, args.NewSize.Height - captionHeight });
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
			{
				return;
			}

			m_mouseInteraction.m_dragStarted = true;

			m_mouseInteraction.m_dragStartPos = GUI::GetScreenMousePosition();
			m_mouseInteraction.m_dragStartLocalPos = IsFloating() ? m_nativeContainer->GetPosition() : this->GetPosition();
			m_mouseInteraction.m_dragStartCaptionPos = args.Position;

			m_mouseInteraction.m_savedDPI = this->Handle()->DPI;
		});

		m_caption->GetEvents().MouseMove.Connect([this](const ArgMouse& args)
		{
			if (!m_mouseInteraction.m_dragStarted)
			{
				return;
			}

			auto dockAreaWindow = this->Handle();
			auto screenMousePos = GUI::GetScreenMousePosition();
			if (!IsFloating())
			{
				auto floatingThreshold = dockAreaWindow->ToScale(4);
				if (std::abs(m_mouseInteraction.m_dragStartPos.X - screenMousePos.X) > floatingThreshold ||
					std::abs(m_mouseInteraction.m_dragStartPos.Y - screenMousePos.Y) > floatingThreshold)
				{
					GUI::ReleaseCapture(*m_caption);

					auto pointInScreen = dockAreaWindow->Position;
					auto dockAreaSize = this->GetSize();

					Rectangle formRect{ pointInScreen.X, pointInScreen.Y, dockAreaSize.Width, dockAreaSize.Height };

					MakeFloating(formRect);

					auto nativeWindow = m_nativeContainer->Handle();
					m_mouseInteraction.m_dragStartLocalPos.X -= static_cast<int>(nativeWindow->BorderSize.Width / 2) - (screenMousePos.X - m_mouseInteraction.m_dragStartPos.X);
					m_mouseInteraction.m_dragStartLocalPos.Y -= static_cast<int>(nativeWindow->BorderSize.Height / 2) - (screenMousePos.Y - m_mouseInteraction.m_dragStartPos.Y);
					m_mouseInteraction.m_dragStartPos = GUI::GetScreenMousePosition();

					GUI::Capture(*m_caption);
					m_mouseInteraction.m_hasChanged = true;
				}
			}
			else
			{
				auto newPosition = screenMousePos - m_mouseInteraction.m_dragStartPos;
				newPosition += m_mouseInteraction.m_dragStartLocalPos;

				if (m_mouseInteraction.m_savedDPI != m_nativeContainer->Handle()->DPI)
				{
					float adjustScaleFactor = (float)m_nativeContainer->Handle()->DPI / m_mouseInteraction.m_savedDPI;
					m_mouseInteraction.m_dragStartCaptionPos.X = static_cast<int>(m_mouseInteraction.m_dragStartCaptionPos.X * adjustScaleFactor);
					m_mouseInteraction.m_dragStartCaptionPos.Y = static_cast<int>(m_mouseInteraction.m_dragStartCaptionPos.Y * adjustScaleFactor);

					auto upperLeftOffset = API::GetPointScreenToClient(m_nativeContainer->Handle()->RootHandle, screenMousePos);

					m_mouseInteraction.m_dragStartPos = screenMousePos;
					m_mouseInteraction.m_dragStartLocalPos = API::GetWindowPosition(m_nativeContainer->Handle()->RootHandle) + upperLeftOffset - m_mouseInteraction.m_dragStartCaptionPos;
					
					m_mouseInteraction.m_savedDPI = m_nativeContainer->Handle()->DPI;
				}
				
				m_mouseInteraction.m_hasChanged = true;
				GUI::MoveWindow(*m_nativeContainer, newPosition);

				m_ownerDockPane->NotifyMove();
			}
		});

		m_caption->GetEvents().MouseUp.Connect([this](const ArgMouse& args)
		{
			m_mouseInteraction.m_dragStarted = false;
			GUI::ReleaseCapture(*m_caption);

			if (m_caption->HaveClickedCloseButton())
			{
				auto selectedIndex = m_tabBar->GetSelectedIndex().value();
				m_tabBarPanels.erase(m_tabBarPanels.begin() + selectedIndex);
				
				m_ownerDockPane->RequestClose();
				return;
			}

			if (m_mouseInteraction.m_hasChanged)
			{
				m_ownerDockPane->NotifyMoveStopped();
			}
		});

		m_tabBar = std::make_unique<TabBar>(this->Handle(), Rectangle{0,0,1u,1u});
		m_tabBar->SetTabRowPosition(TabRowPosition::Bottom);
		m_tabBar->ShowCloseButton(false);
		
		m_tabBar->GetEvents().TabChanged.Connect([this](const ArgTabBar& args)
		{
			m_caption->SetCaption(std::string(args.Id));
		});

		if (!paneInfo->showCaption)
		{
			m_caption->Hide();
		}
		
		m_tabBar->GetEvents().TabMouseDown.Connect([this](const ArgTabMouse& args)
		{
			if (!args.Mouse.ButtonState.LeftButton)
			{
				return;
			}
			
			OnTabMouseDown(args.Index, args.Mouse.Position);
		});
		
		m_tabBar->GetEvents().TabMouseMove.Connect([this](const ArgTabMouse& args)
		{
			if (!args.Mouse.ButtonState.LeftButton)
			{
				return;
			}
			OnTabMouseMove(args.Mouse.Position);
		});
		
		m_tabBar->GetEvents().TabMouseUp.Connect([this](const ArgTabMouse& args)
		{
			if (!args.Mouse.ButtonState.LeftButton)
			{
				return;
			}
			OnTabMouseUp(args.Mouse.Position);
		});
	}

	void DockArea::Dock()
	{
		GUI::SetParentWindow(this->Handle(), m_hostWindow);
		m_nativeContainer.reset();
	}

	std::optional<size_t> DockArea::GetTabSelectedIndex() const
	{
		return m_tabBar->GetSelectedIndex();
	}

	void DockArea::MakeFloating(const Rectangle& rect)
	{
		if (IsFloating()) 
		{
			return; 
		}

		m_nativeContainer = std::make_unique<Form>(m_hostWindow, rect, FormStyle::Float());
		auto nativeWindow = m_nativeContainer->Handle();
    
#if BT_DEBUG
		nativeWindow->Name = "DockFloat-" + m_paneInfo->id;
#endif

		GUI::SetParentWindow(this->Handle(), nativeWindow);
		this->SetPosition({ 1, 1 });
		this->SetSize({ rect.Width - 1, rect.Height - 1 });
		m_nativeContainer->GetEvents().Resize.Connect([this](const ArgResize& args)
		{
			this->SetSize({ args.NewSize.Width - 1, args.NewSize.Height - 1 });
		});

		m_nativeContainer->Show();
		m_ownerDockPane->NotifyFloat();
	}

	void DockArea::OnTabMouseDown(size_t tabIndex, const Point& mouseScreenPos)
	{
		m_mouseInteraction.m_dragStarted = false;
		m_mouseInteraction.m_dragStartPos = mouseScreenPos;
		m_mouseInteraction.m_draggedTabIndex = tabIndex;
	}

	void DockArea::OnTabMouseMove(const Point& mouseScreenPos)
	{
		if (!m_mouseInteraction.m_dragStarted && m_mouseInteraction.m_draggedTabIndex.has_value() &&
			m_tabBar->Count() > 1)
		{
			int dx = mouseScreenPos.X - m_mouseInteraction.m_dragStartPos.X;
			int dy = mouseScreenPos.Y - m_mouseInteraction.m_dragStartPos.Y;

			constexpr int DRAG_THRESHOLD = 5;

			if (std::abs(dx) > DRAG_THRESHOLD || std::abs(dy) > DRAG_THRESHOLD)
			{
				m_mouseInteraction.m_dragStarted = true;
				size_t tabToFloat = *m_mouseInteraction.m_draggedTabIndex;
				m_mouseInteraction.m_draggedTabIndex = std::nullopt;
    
				if (m_ownerDockPane && tabToFloat < m_ownerDockPane->m_children.size())
				{
					auto tabNode = static_cast<DockPaneTabLayoutNode*>(m_ownerDockPane->m_children[tabToFloat].get());
					
					ArgFloatTab argFloatTab{tabNode, mouseScreenPos};
					m_ownerDockPane->Events.OnFloatTab.Emit(argFloatTab);
				}
			}
		}
	}

	void DockArea::OnTabMouseUp(const Point& mouseScreenPos)
	{
		if (!m_mouseInteraction.m_dragStarted)
			return;
		
		m_mouseInteraction.m_dragStarted = false;
		m_mouseInteraction.m_draggedTabIndex = std::nullopt;
    
		if (m_ownerDockPane)
		{
			m_ownerDockPane->NotifyMoveStopped();
		}
	}

	void DockAreaCaption::SetPaneInfo(PaneInfo* paneInfo)
	{
		GetReactor().m_paneInfo = paneInfo;
	}

	bool DockAreaCaption::WasPressedCloseButton() const
	{
		return GetReactor().m_mouseDownCloseButton;
	}

	bool DockAreaCaption::HaveClickedCloseButton() const
	{
		return GetReactor().m_clickedCloseButton;
	}

	DockPaneTabLayoutNode::DockPaneTabLayoutNode() :
		LayoutNode(LayoutNodeType::DockPaneTab)
	{
	}

	void DockPaneTabLayoutNode::CalculateAreas()
	{
	}
}