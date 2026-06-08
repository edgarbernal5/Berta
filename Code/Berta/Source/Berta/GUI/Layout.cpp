/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Layout.h"

#include "Berta/GUI/Control.h"
#include "Berta/GUI/Window.h"
#include "Berta/GUI/Layouts/LayoutNodes.h"
#include "Berta/Controls/Form.h"
#include "Berta/Paint/DrawBatchActivator.h"
#include "Berta/GUI/Layouts/LayoutParser.h"
#include "Berta/GUI/Layouts/Lexer.h"

//#define BT_LAYOUT_PRINT_DEBUG

namespace Berta
{
	Layout::Layout()
	{
		InitPaneIndicators();
	}

	Layout::Layout(Window* owner)
	{
		InitPaneIndicators();
		Create(owner);
	}

	Layout::~Layout()
	{
		m_fields.clear();
	}

	void Layout::AddPane(std::string_view paneId)
	{
		if (!m_rootNode)
		{
			return;
		}
		
		auto paneNode = m_rootNode->Find(paneId);
		if (paneNode)
		{
			return;
		}
		
		auto dockRoot = m_rootNode->FindFirst(LayoutNodeType::Dock);
		if (!dockRoot || !dockRoot->m_children.empty())
		{
			return;
		}

		//create a new layout node (DockPaneLayoutNode)
		auto newPaneNode = std::make_unique<DockPaneLayoutNode>();

		std::string paneIdStr{ paneId };
		m_dockPaneFields[paneIdStr] = newPaneNode.get();
		auto& paneInfo = m_dockPaneInfoFields[paneIdStr];
		paneInfo.id = paneId;

		newPaneNode->m_dockArea = std::make_unique<DockArea>();
		newPaneNode->m_dockArea->Create(m_owner, &paneInfo);
		newPaneNode->m_dockArea->m_ownerDockPane = newPaneNode.get();

		WireDockPaneEvents(newPaneNode.get());
		
		newPaneNode->m_paneId = paneId;
		newPaneNode->SetParentNode(dockRoot);
		newPaneNode->SetOwnerWindow(dockRoot->GetOwnerWindow());

		dockRoot->m_children.emplace_back(std::move(newPaneNode));
	}

	void Layout::AddPaneTab(std::string_view paneId, std::string_view tabId, std::unique_ptr<ControlBase> control)
	{
		if (!m_rootNode)
		{
			return;
		}

		auto paneNode = GetPane(paneId);
		if (!paneNode)
		{
			return;
		}

		if (GetPaneTab(paneId, tabId))
		{
			return;
		}
		
		std::string paneIdStr{ paneId };
		std::string tabIdStr{ tabId };
		auto paneTabId = paneIdStr + "/" + tabIdStr;

		auto paneTabNode = std::make_unique<DockPaneTabLayoutNode>();
		paneTabNode->SetId(paneTabId);
		paneTabNode->m_tabId = paneTabId;
		
		paneTabNode->SetParentNode(paneNode);
		paneTabNode->SetOwnerWindow(paneNode->GetOwnerWindow());
		m_dockPaneTabFields[paneTabId] = paneTabNode.get();

		paneNode->AddTab(tabIdStr, std::move(control));
		paneNode->m_children.emplace_back(std::move(paneTabNode));

		Apply();
	}

	void Layout::AddPaneTab(std::string_view paneId, std::string_view tabId, std::unique_ptr<ControlBase> control, std::string_view relativePaneId, DockPosition dockPosition)
	{
		if (!m_rootNode)
		{
			return;
		}

		if (GetPane(paneId) || GetPaneTab(paneId, tabId))
		{
			return;
		}

		auto dockRoot = m_rootNode->FindFirst(LayoutNodeType::Dock);
		auto relativePaneNode = relativePaneId.empty() ? (!dockRoot || !dockRoot->m_children.empty() ? nullptr : dockRoot) : GetPane(relativePaneId);
		if (!relativePaneNode)
		{
			return;
		}

		std::string paneIdStr{ paneId };
		auto newPaneNode = std::make_unique<DockPaneLayoutNode>();
		newPaneNode->SetId(paneIdStr);

		auto newPaneNodePtr = newPaneNode.get();
		m_dockPaneFields[paneIdStr] = newPaneNodePtr;
		auto& paneInfo = m_dockPaneInfoFields[paneIdStr];
		paneInfo.id = paneId;

		newPaneNode->m_dockArea = std::make_unique<DockArea>();
		newPaneNode->m_dockArea->Create(m_owner, &paneInfo);
		newPaneNode->m_dockArea->m_ownerDockPane = newPaneNodePtr;

		WireDockPaneEvents(newPaneNode.get());
		
		newPaneNode->m_paneId = paneId;
		newPaneNode->SetOwnerWindow(m_owner);

		std::string tabIdStr{ tabId };
		auto paneTabId = paneIdStr + "/" + tabIdStr;

		auto paneTabNode = std::make_unique<DockPaneTabLayoutNode>();
		paneTabNode->SetId(paneTabId);
		paneTabNode->m_tabId = paneTabId;
		paneTabNode->SetParentNode(newPaneNodePtr);
		paneTabNode->SetOwnerWindow(newPaneNode->GetOwnerWindow());
		
		m_dockPaneTabFields[paneTabId] = paneTabNode.get();

		newPaneNode->AddTab(tabIdStr, std::move(control));
		newPaneNode->m_children.emplace_back(std::move(paneTabNode));

		m_floatingDockFields.emplace_back(std::move(newPaneNode));

		if (DoDock(newPaneNodePtr, relativePaneNode, dockPosition))
		{
			Apply();
		}
	}

	void Layout::Apply()
	{
		if (!m_rootNode || !m_owner)
		{
			return;
		}

		auto area = GUI::SizeWindow(m_owner);
		if (area.IsEmpty())
		{
			return;
		}
		//TODO:
		DrawBatchActivator drawBatch(m_owner->RootWindow);

		m_rootNode->SetArea(area.ToRectangle());
		m_rootNode->CalculateAreas();

		if (auto windowToUpdate = m_owner->FindFirstNonPanelAncestor())
		{
			GUI::UpdateWindow(windowToUpdate);
		}
	}

	Layout& Layout::Attach(std::string_view fieldId, Window* window)
	{
		std::string searchId{ fieldId };
		auto it = m_fields.find(searchId);
		
		if (it != m_fields.end())
		{
			it->second->AddWindow(window);
		}
		else
		{
			auto newNode = m_rootNode->Find(searchId);
			if (newNode)
			{
				m_fields[searchId] = newNode; 
				newNode->AddWindow(window);
			}
			else
			{
			}
		}
		return *this;
	}

	void Layout::Create(Window* owner)
	{
		if (!owner)
		{
			return;
		}
		if (m_owner)
		{
			m_owner->Events->Resize.Disconnect(m_resizeEventId);
		}
		m_owner = owner;

		m_resizeEventId = m_owner->Events->Resize.Connect([this](const ArgResize& args)
			{
				//TODO: add this same logic to visibility event?!
				if (m_rootNode)
				{
					m_rootNode->SetArea({ 0, 0, args.NewSize.Width, args.NewSize.Height });
					m_rootNode->CalculateAreas();
				}

				Print();
			});
	}

	void Layout::Parse(const std::string& source)
	{
		Lexer lexer(source);
		std::vector<Token> tokens = lexer.Tokenize();

		LayoutParser parser(tokens);
		auto rootNode = parser.Parse();
		if (!rootNode)
		{
			return;
		}
		BT_CORE_TRACE << "Parse completed." << std::endl;

		m_rootNode = std::move(rootNode);
		m_rootNode->SetOwnerWindow(m_owner);
	}

	bool Layout::RemoveDockPane(DockPaneLayoutNode* node)
	{
		DoFloat(node);

		size_t nodeIndex;
		if (IsAlreadyDocked(node, nodeIndex))
		{
			return false;
		}

		m_floatingDockFields.erase(m_floatingDockFields.begin() + nodeIndex);
		return true;
	}

	void Layout::WireDockPaneEvents(DockPaneLayoutNode* node)
	{
		if (!node)
		{
			return;
		}
		
		node->Events.OnFloat.Connect([this](DockPaneLayoutNode* const& n)
		{
			HandleFloat(n); 
		});

		node->Events.OnMove.Connect([this](DockPaneLayoutNode* const& n)
		{
			HandleMove(n); 
		});

		node->Events.OnMoveStarted.Connect([this](DockPaneLayoutNode* const& n)
		{
			HandleMoveStarted(n); 
		});

		node->Events.OnMoveStopped.Connect([this](DockPaneLayoutNode* const& n)
		{
			HandleMoveStopped(n); 
		});

		node->Events.OnRequestClose.Connect([this](DockPaneLayoutNode* const& n)
		{
			HandleRequestClose(n); 
		});
		
		node->Events.OnFloatTab.Connect([this](const ArgFloatTab& args)
		{
		   HandleFloatTab(args.tabNode, args.mouseScreenPos); 
		});
	}

	void Layout::HandleFloat(DockPaneLayoutNode* const& node)
	{
		BT_CORE_TRACE << "HandleFloat id=" << node->GetId() << std::endl;
		node->SetOwnerWindow(node->m_dockArea->m_nativeContainer->Handle());

		if (DoFloat(node))
		{
			Apply();
			Print();
		}
	}

	void Layout::HandleMove(DockPaneLayoutNode* const& node)
	{
		//BT_CORE_TRACE << "HandleMove id=" << node->GetId() << std::endl;
		if (!IsMouseInsideWindow())
		{
			m_dragDropCtx.lastTargetNode = nullptr;
			HidePaneDockIndicators();
			return;
		}

		auto paneOrDock = GetPaneOrDockOnMousePosition();
		if (paneOrDock)
		{
			ShowPaneDockIndicators(paneOrDock);
		}
		else
		{
			HidePaneDockIndicators();
		}

		DockPosition dockPosition = DockPosition::Tab;
		if (IsMouseInsideDockIndicator(&dockPosition))
		{
			if (DoDock(node, paneOrDock, dockPosition))
			{
				BT_CORE_TRACE << " - DoDock." << std::endl;
				m_dragDropCtx.lockPaneIndicators = true;
				m_dragDropCtx.lastTargetNode = paneOrDock;
				Apply();

				auto dockPanelTargetArea = node->GetArea();

				m_dragDropCtx.dockPanelTarget.reset(new DockPanel(m_owner, false, dockPanelTargetArea));
				m_dragDropCtx.dockPanelTarget->Show();

				Print();
			}
		}
		else
		{
			m_dragDropCtx.lockPaneIndicators = false;
			if (DoFloat(node))
			{
				m_dragDropCtx.lastTargetNode = nullptr;
				BT_CORE_TRACE << " - DoFloat." << std::endl;
				m_dragDropCtx.dockPanelTarget.reset();

				Apply();
				Print();
			}
		}
	}

	void Layout::HandleMoveStarted(DockPaneLayoutNode* const& node)
	{
		node->m_dockArea->EnterSizeMove();
	}

	void Layout::HandleMoveStopped(DockPaneLayoutNode* const& node)
	{
		BT_CORE_TRACE << "HandleStop id=" << node->GetId() << std::endl;
		
		node->m_dockArea->ExitSizeMove();
		
		m_dragDropCtx.lockPaneIndicators = false;
		auto shouldDock = IsMouseInsideDockIndicator();
		HidePaneDockIndicators();

		m_dragDropCtx.dockPanelTarget.reset();
		if (shouldDock)
		{
			if (m_tabDockField)
			{
				auto targetPane = static_cast<DockPaneLayoutNode*>(m_dragDropCtx.lastTargetNode);
				targetPane->AppendPane(node);

				for (size_t i = 0; i < node->m_children.size(); i++)
				{
					std::string oldTabId = node->m_children[i]->GetId();
					std::string newTabId = targetPane->m_paneId + oldTabId.substr(oldTabId.find_last_of('/'));
					m_dockPaneTabFields.erase(oldTabId);
					
					auto tabNodePtr = static_cast<DockPaneTabLayoutNode*>(node->m_children[i].get());
					
					tabNodePtr->m_tabId = newTabId;
					node->m_children[i]->SetParentNode(targetPane);
					m_dockPaneTabFields[newTabId] = tabNodePtr;
					
					targetPane->m_children.emplace_back(std::move(node->m_children[i]));
				}
				node->m_children.clear();
				node->m_dockArea->m_nativeContainer.reset();

				m_tabDockField.reset();
			}
			else
			{
				node->SetOwnerWindow(node->m_dockArea->m_hostWindow);
				node->m_dockArea->Dock();
			}
			
			Apply();
		}

		m_dragDropCtx.lastTargetNode = nullptr;
	}

	void Layout::HandleRequestClose(DockPaneLayoutNode* const& node)
	{
		auto index = node->m_dockArea->GetTabSelectedIndex().value();
		auto childNode = static_cast<DockPaneTabLayoutNode*>(node->m_children[index].get());
		m_dockPaneTabFields.erase(childNode->m_tabId);

		node->m_dockArea->m_tabBar->Erase(index);
		node->m_children.erase(node->m_children.begin() + index);

		m_dockPaneFields.erase(node->m_paneId);

		bool needUpdate = false;
		if (node->m_children.empty())
		{
			needUpdate = RemoveDockPane(node);
		}

		if (needUpdate)
		{
			Apply();
		}
	}

	void Layout::HandleFloatTab(DockPaneTabLayoutNode* tabNode, const Point& mouseScreenPos)
	{
		if (!tabNode || !m_rootNode)
		{
			return;
		}
		
		auto sourcePaneNode = static_cast<DockPaneLayoutNode*>(tabNode->GetParentNode());
		if (!sourcePaneNode || !sourcePaneNode->m_dockArea)
		{
			return;
		}
		
		size_t tabIndex = 0;
		for (; tabIndex < sourcePaneNode->m_children.size(); ++tabIndex)
		{
			if (sourcePaneNode->m_children[tabIndex].get() == tabNode)
			{
				break;
			}
		}
		if (tabIndex >= sourcePaneNode->m_children.size())
		{
			return;
		}

		auto detachedTabNode = std::move(sourcePaneNode->m_children[tabIndex]);
		sourcePaneNode->m_children.erase(sourcePaneNode->m_children.begin() + tabIndex);

		auto control = std::move(sourcePaneNode->m_dockArea->m_tabBarPanels[tabIndex].ControlPtr);
		sourcePaneNode->m_dockArea->m_tabBarPanels.erase(sourcePaneNode->m_dockArea->m_tabBarPanels.begin() + tabIndex);
		sourcePaneNode->m_dockArea->m_tabBar->Detach(tabIndex);

		std::string newPaneIdStr = sourcePaneNode->m_paneId; 
		std::string newTabIdStr = tabNode->m_tabId;
		std::string rawTabIdStr = newTabIdStr.substr(newTabIdStr.find_last_of('/') + 1);

		auto newPaneNode = std::make_unique<DockPaneLayoutNode>();
		newPaneNode->SetId(newPaneIdStr);
		newPaneNode->m_paneId = newPaneIdStr;
		newPaneNode->SetOwnerWindow(m_owner);

		auto newPaneNodePtr = newPaneNode.get();
		m_dockPaneFields[newPaneIdStr] = newPaneNodePtr;

		auto& paneInfo = m_dockPaneInfoFields[newPaneIdStr];
		paneInfo.id = newPaneIdStr;

		newPaneNode->m_dockArea = std::make_unique<DockArea>();
		newPaneNode->m_dockArea->Create(m_owner, &paneInfo);
		newPaneNode->m_dockArea->m_ownerDockPane = newPaneNodePtr;
		
		WireDockPaneEvents(newPaneNodePtr);

		tabNode->SetId(newTabIdStr);
		tabNode->m_tabId = newTabIdStr;
		tabNode->SetParentNode(newPaneNodePtr);

		newPaneNode->AddTab(rawTabIdStr, std::move(control));
		newPaneNode->m_children.emplace_back(std::move(detachedTabNode));

		m_floatingDockFields.emplace_back(std::move(newPaneNode));
    
		auto sourceSize = sourcePaneNode->m_dockArea->GetSize();
		
		Point windowTopLeft = sourcePaneNode->m_dockArea->GetPosition();
		Rectangle startRect{ windowTopLeft.X, windowTopLeft.Y, sourceSize.Width, sourceSize.Height };
		
		newPaneNodePtr->m_dockArea->MakeFloating(startRect);
		
		auto& interaction = newPaneNodePtr->m_dockArea->m_mouseInteraction;
		interaction.m_dragStarted = true;
		interaction.m_dragStartPos = GUI::GetScreenMousePosition();
		interaction.m_dragStartCaptionPos = windowTopLeft;

		interaction.m_savedDPI = newPaneNodePtr->m_dockArea->Handle()->DPI;
		
		interaction.m_dragStartLocalPos = sourcePaneNode->m_dockArea->GetPosition(); 

		GUI::Capture(*newPaneNodePtr->m_dockArea->m_caption);
		newPaneNodePtr->NotifyMoveStarted();
		
		Apply();
	}

	DockPaneLayoutNode* Layout::GetPane(std::string_view paneId)
	{
		std::string paneIdStr { paneId };
		auto it = m_dockPaneFields.find(paneIdStr);
		if (it != m_dockPaneFields.end())
		{
			return it->second;
		}

		return nullptr;
	}

	DockPaneTabLayoutNode* Layout::GetPaneTab(std::string_view paneId, std::string_view tabId)
	{
		auto paneTabId = std::string(paneId) + "/" + std::string(tabId);
		auto it = m_dockPaneTabFields.find(paneTabId);
		if (it != m_dockPaneTabFields.end())
		{
			return it->second;
		}

		return nullptr;
	}

	void Layout::InitPaneIndicators()
	{
		m_dragDropCtx.paneIndicators.emplace_back(new DockIndicator{ DockPosition::Up });
		m_dragDropCtx.paneIndicators.emplace_back(new DockIndicator{ DockPosition::Down });
		m_dragDropCtx.paneIndicators.emplace_back(new DockIndicator{ DockPosition::Left });
		m_dragDropCtx.paneIndicators.emplace_back(new DockIndicator{ DockPosition::Right });
		m_dragDropCtx.paneIndicators.emplace_back(new DockIndicator{ DockPosition::Tab });
	}

	void Layout::HidePaneDockIndicators()
	{
		if (m_dragDropCtx.lockPaneIndicators)
			return;

		for (auto& indicator : m_dragDropCtx.paneIndicators)
		{
			indicator->Docker.reset();
		}
	}

	void Layout::ShowPaneDockIndicators(LayoutNode* node)
	{
		if (m_dragDropCtx.lockPaneIndicators)
			return;

		auto indicatorSize = m_owner->ToScale(32);
		auto indicatorSizeHalf = indicatorSize >> 1;
		auto indicatorSizeOffset = indicatorSizeHalf >> 1;

		for (auto& indicator : m_dragDropCtx.paneIndicators)
		{
			if (node->GetType() == LayoutNodeType::Dock && indicator->Position != DockPosition::Tab)
			{
				if (indicator->Docker)
				{
					indicator->Docker.reset();
				}
				continue;
			}
			auto nodeArea = node->GetArea();
			auto x = nodeArea.X + static_cast<int>(nodeArea.Width) / 2;
			auto y = nodeArea.Y + static_cast<int>(nodeArea.Height) / 2;

			Point position{};
			if (indicator->Position == DockPosition::Tab)
			{
				position = { x - indicatorSizeHalf, y - indicatorSizeHalf };
			}
			else if (indicator->Position == DockPosition::Up)
			{
				position = { x - indicatorSizeHalf, y - indicatorSize - indicatorSizeHalf - indicatorSizeOffset };
			}
			else if (indicator->Position == DockPosition::Down)
			{
				position = { x - indicatorSizeHalf, y + indicatorSizeHalf + indicatorSizeOffset };
			}
			else if (indicator->Position == DockPosition::Left)
			{
				position = { x - indicatorSizeHalf - indicatorSize - indicatorSizeOffset, y - indicatorSizeHalf };
			}
			else if (indicator->Position == DockPosition::Right)
			{
				position = { x + indicatorSizeHalf + indicatorSizeOffset, y - indicatorSizeHalf };
			}

			if (!indicator->Docker)
			{
				indicator->Docker = std::make_unique<DockIndicatorForm>(m_owner, Rectangle{ position.X, position.Y, (uint32_t)indicatorSize, (uint32_t)indicatorSize });
				indicator->Docker->SetDockPosition(indicator->Position);
				
				if (indicator->Docker)
				{
					GUI::MakeWindowActive(*indicator->Docker, false, m_owner);
#if BT_DEBUG
					std::ostringstream builder;
					builder << "Indicator-" << (int)indicator->Position;
					indicator->Docker->SetDebugName(builder.str());
#endif
					indicator->Docker->Show();
				}
			}
			else
			{
				auto oldPosition = API::GetWindowPosition(indicator->Docker->Handle()->RootHandle);
				if (oldPosition != position)
				{
					indicator->Docker->SetPosition(position);
				}
			}
		}
	}

	bool Layout::IsMouseInsideWindow() const
	{
		if (!m_owner)
			return false;

		auto mousePosition = GUI::GetScreenMousePosition();
		auto windowPosition = GUI::GetPointClientToScreen(m_owner, GUI::GetWindowRootPosition(m_owner));
		Rectangle rect{ windowPosition.X, windowPosition.Y, m_owner->ClientSize.Width, m_owner->ClientSize.Height };

		return rect.Contains(mousePosition);
	}

	bool Layout::IsMouseInsideDockIndicator(DockPosition* outDockPosition) const
	{
		for (auto& indicator : m_dragDropCtx.paneIndicators)
		{
			if (!indicator->Docker)
			{
				continue;
			}

			auto mousePosition = GUI::GetScreenMousePosition();
			auto dockerHandle = indicator->Docker->Handle();
			auto windowPosition = GUI::GetPointClientToScreen(dockerHandle, GUI::GetWindowRootPosition(dockerHandle));
			Rectangle rect{ windowPosition.X, windowPosition.Y, dockerHandle->ClientSize.Width, dockerHandle->ClientSize.Height };

			if (rect.Contains(mousePosition))
			{
				if (outDockPosition)
				{
					*outDockPosition = indicator->Position;
				}

				return true;
			}
		}
		return false;
	}

	LayoutNode* Layout::GetPaneOrDockOnMousePosition() const
	{
		if (!m_owner || !m_rootNode)
			return nullptr;

		auto getPane = GetPaneOrDockOnMousePositionInternal(m_rootNode.get(), LayoutNodeType::DockPane);
		if (getPane)
		{
			return getPane;
		}
		return GetPaneOrDockOnMousePositionInternal(m_rootNode.get(), LayoutNodeType::Dock);
	}

	LayoutNode* Layout::GetPaneOrDockOnMousePositionInternal(LayoutNode* node, LayoutNodeType nodeType) const
	{
		if (node->GetType() == nodeType)
		{
			if (nodeType == LayoutNodeType::Dock && node->m_children.empty() ||
				nodeType == LayoutNodeType::DockPane)
			{
				auto mousePosition = GUI::GetScreenMousePosition();
				auto windowPosition = GUI::GetPointClientToScreen(m_owner, GUI::GetWindowRootPosition(m_owner));
				auto nodeArea = node->GetArea();
				Rectangle rect
				{
					windowPosition.X + nodeArea.X, windowPosition.Y + nodeArea.Y,
					nodeArea.Width, nodeArea.Height
				};

				if (rect.Contains(mousePosition))
				{
					return node;
				}
			}
		}

		for (size_t i = 0; i < node->m_children.size(); i++)
		{
			auto child = GetPaneOrDockOnMousePositionInternal(node->m_children[i].get(), nodeType);
			if (child)
			{
				return child;
			}
		}
		return nullptr;
	}

	bool Layout::IsAlreadyDocked(LayoutNode* node, size_t& nodeIndex) const
	{
		nodeIndex = 0;
		for (nodeIndex = 0; nodeIndex < m_floatingDockFields.size(); ++nodeIndex)
		{
			if (m_floatingDockFields[nodeIndex].get() == node)
			{
				break;
			}
		}

		return nodeIndex == m_floatingDockFields.size(); 
	}

	bool Layout::DoFloat(DockPaneLayoutNode* paneNode)
	{
		if (m_tabDockField)
		{
			m_floatingDockFields.emplace_back(std::move(m_tabDockField));
			auto& floatingDockField = m_floatingDockFields.back();
      
			floatingDockField->SetParentNode(nullptr);
			floatingDockField->SetPrev(nullptr);
			floatingDockField->SetNext(nullptr);
      
			// Limpieza en el nodo que pasa a flotar
			floatingDockField->RemoveProperty("LayoutWeight");

			m_tabDockField.reset();
			return true;
		}

		auto parent = paneNode->GetParentNode();
		if (!parent)
		{
			return false;
		}
		
		for (size_t i = 0; i < parent->m_children.size(); ++i)
		{
			if (parent->m_children[i].get() == paneNode)
			{
				m_floatingDockFields.emplace_back(parent->m_children[i].release());
				auto& floatingDockField = m_floatingDockFields.back();
         
				floatingDockField->SetParentNode(nullptr);
				floatingDockField->SetPrev(nullptr);
				floatingDockField->SetNext(nullptr);
         
				floatingDockField->RemoveProperty("LayoutWeight");

				// 1. Extirpación del nodo y su Splitter adyacente
				if (i == parent->m_children.size() - 1)
				{
					parent->m_children.pop_back(); // Elimina el panel
					if (!parent->m_children.empty()) parent->m_children.pop_back(); // Elimina el splitter previo
				}
				else
				{
					parent->m_children.erase(parent->m_children.begin() + i); // Elimina el panel
					if (!parent->m_children.empty())
					{
						parent->m_children.erase(parent->m_children.begin() + i); // Elimina el splitter posterior
					}
				}

				// 2. Comprobar si el padre colapsa (se quedó con 1 solo hijo y es un contenedor)
				if (parent->m_children.size() == 1 && parent->GetType() == LayoutNodeType::Container)
				{
					auto child = parent->m_children[0].release();
					auto parentParent = parent->GetParentNode();
            
					if (parentParent)
					{
						for (size_t j = 0; j < parentParent->m_children.size(); j++)
						{
							if (parentParent->m_children[j].get() == parent)
							{
								child->SetParentNode(parentParent);
                      
								parentParent->m_children[j].reset(child);

								for (size_t k = 0; k < parentParent->m_children.size(); k++)
								{
									auto* current = parentParent->m_children[k].get();
									current->SetNext(k == parentParent->m_children.size() - 1 ? nullptr : parentParent->m_children[k + 1].get());
									current->SetPrev(k == 0 ? nullptr : parentParent->m_children[k - 1].get());
                        
									current->RemoveProperty("LayoutWeight");
									current->RemoveProperty("Width");
									current->RemoveProperty("Height");
								}
								break;
							}
						}
					}
				}
				else
				{
					for (size_t k = 0; k < parent->m_children.size(); k++)
					{
						auto* current = parent->m_children[k].get();
						current->SetNext(k == parent->m_children.size() - 1 ? nullptr : parent->m_children[k + 1].get());
						current->SetPrev(k == 0 ? nullptr : parent->m_children[k - 1].get());
                
						current->RemoveProperty("LayoutWeight");
						current->RemoveProperty("Width");
						current->RemoveProperty("Height");
					}
				}
				break;
			}
		}

		return true;
	}

	bool Layout::DoDock(DockPaneLayoutNode* node, LayoutNode* target, DockPosition dockPosition)
	{
		if (!target)
		{
			return false;
		}
		
		size_t nodeIndex;
		if (IsAlreadyDocked(node, nodeIndex)) return false;

		// Caso A: Anclar a una raíz de Dock vacía
		if (target->GetType() == LayoutNodeType::Dock && target->m_children.empty())
		{
			node->SetParentNode(target);
       
			// 💎 RECONSTRUCCIÓN: Al ser el único hijo, sus referencias de vecindad son nulas.
			node->SetPrev(nullptr);
			node->SetNext(nullptr);
       
			target->m_children.emplace_back(std::move(m_floatingDockFields[nodeIndex]));
			m_floatingDockFields.erase(m_floatingDockFields.begin() + nodeIndex);
			return true;
		}

		// Caso B: Docking por pestañas
		if (dockPosition == DockPosition::Tab)
		{
			node->SetArea(target->GetArea());
			m_tabDockField = std::move(m_floatingDockFields[nodeIndex]);
			m_floatingDockFields.erase(m_floatingDockFields.begin() + nodeIndex);
			return true;
		}

		LayoutNode* dockRootNode = target;
		while (dockRootNode && dockRootNode->GetType() != LayoutNodeType::Dock)
		{
			dockRootNode = dockRootNode->GetParentNode();
		}
		if (!dockRootNode) return false;

		auto targetParent = target->GetParentNode() ? target->GetParentNode() : dockRootNode;
		bool isVerticalOrientation = (dockPosition == DockPosition::Up || dockPosition == DockPosition::Down);
		bool addNewOrientation = false;

		if (dockRootNode->m_children[0]->GetType() == LayoutNodeType::DockPane)
		{
			addNewOrientation = true;
		}
		else
		{
			auto targetParentContainer = static_cast<ContainerLayoutNode*>(targetParent);
			if (targetParentContainer->GetOrientation() != isVerticalOrientation)
			{
				addNewOrientation = true;
			}
		}

		auto targetIndex = target->GetIndex();
   
		// --- RAMA 1: NUEVA ORIENTACIÓN (Sub-contenedor) ---
		if (addNewOrientation)
		{
			if (targetIndex == std::string::npos) return false;

			std::unique_ptr<LayoutNode> targetPtr = std::move(target->GetParentNode()->m_children[targetIndex]);

			auto containerPtr = std::make_unique<ContainerLayoutNode>(isVerticalOrientation);
			containerPtr->SetParentNode(target->GetParentNode());
			containerPtr->SetOwnerWindow(target->GetOwnerWindow());

			auto splitterPtr = std::make_unique<SplitterLayoutNode>(isVerticalOrientation);
			splitterPtr->SetParentNode(containerPtr.get());
			splitterPtr->SetOwnerWindow(target->GetOwnerWindow());

			// 💎 HERENCIA DE VECINDAD: El nuevo contenedor hereda explícitamente el Prev y el Next del target original
			containerPtr->SetPrev(targetPtr->GetPrev());
			containerPtr->SetNext(targetPtr->GetNext());

			node->SetParentNode(containerPtr.get());
			targetPtr->SetParentNode(containerPtr.get());

			targetPtr->RemoveProperty("LayoutWeight");
			targetPtr->RemoveProperty("Width");
			targetPtr->RemoveProperty("Height");
      
			node->RemoveProperty("LayoutWeight");
			node->RemoveProperty("Width");
			node->RemoveProperty("Height");

			if (dockPosition == DockPosition::Up || dockPosition == DockPosition::Left)
			{
				containerPtr->m_children.emplace_back(std::move(m_floatingDockFields[nodeIndex]));
				containerPtr->m_children.emplace_back(std::move(splitterPtr));
				containerPtr->m_children.emplace_back(std::move(targetPtr));
			}
			else
			{
				containerPtr->m_children.emplace_back(std::move(targetPtr));
				containerPtr->m_children.emplace_back(std::move(splitterPtr));
				containerPtr->m_children.emplace_back(std::move(m_floatingDockFields[nodeIndex]));
			}

			auto* child0 = containerPtr->m_children[0].get();
			auto* child1 = containerPtr->m_children[1].get();
			auto* child2 = containerPtr->m_children[2].get();

			child0->SetPrev(nullptr); child0->SetNext(child1);
			child1->SetPrev(child0);  child1->SetNext(child2);
			child2->SetPrev(child1);  child2->SetNext(nullptr);

			targetParent->m_children[targetIndex] = std::move(containerPtr);
			auto* newContainerRaw = targetParent->m_children[targetIndex].get();
       
			// 💎 SINCRONIZACIÓN EXTERNA: Actualizamos las referencias de los hermanos adyacentes para que apunten al nuevo contenedor
			if (targetIndex > 0)
			{
				targetParent->m_children[targetIndex - 1]->SetNext(newContainerRaw);
			}
			if (targetIndex < targetParent->m_children.size() - 1)
			{
				targetParent->m_children[targetIndex + 1]->SetPrev(newContainerRaw);
			}
		}
		// --- RAMA 2: MISMA ORIENTACIÓN (Inserción co-lineal) ---
		else
		{
			auto splitterPtr = std::make_unique<SplitterLayoutNode>(isVerticalOrientation);
			splitterPtr->SetParentNode(targetParent);
			splitterPtr->SetOwnerWindow(target->GetOwnerWindow());

			node->SetParentNode(targetParent);

			if (dockPosition == DockPosition::Up || dockPosition == DockPosition::Left)
			{
				if (targetIndex == std::string::npos) targetIndex = 0;
				targetParent->m_children.emplace(targetParent->m_children.begin() + targetIndex, std::move(m_floatingDockFields[nodeIndex]));
				targetParent->m_children.emplace(targetParent->m_children.begin() + targetIndex + 1, std::move(splitterPtr));
			}
			else
			{
				if (targetIndex == std::string::npos) targetIndex = targetParent->m_children.size() - 1;
				targetParent->m_children.emplace(targetParent->m_children.begin() + targetIndex + 1, std::move(splitterPtr));
				targetParent->m_children.emplace(targetParent->m_children.begin() + targetIndex + 2, std::move(m_floatingDockFields[nodeIndex]));
			}

			for (size_t i = 0; i < targetParent->m_children.size(); i++)
			{
				auto* currentChild = targetParent->m_children[i].get();
         
				currentChild->SetNext(i == targetParent->m_children.size() - 1 ? nullptr : targetParent->m_children[i + 1].get());
				currentChild->SetPrev(i == 0 ? nullptr : targetParent->m_children[i - 1].get());

				currentChild->RemoveProperty("LayoutWeight");
				currentChild->RemoveProperty("Width");
				currentChild->RemoveProperty("Height");
			}
		}

		m_floatingDockFields.erase(m_floatingDockFields.begin() + nodeIndex);
		return true;
	}

	void Layout::Print()
	{
#ifdef BT_LAYOUT_PRINT_DEBUG
		std::cout << "Print()" << std::endl;
		if (!m_rootNode)
		{
			std::cout << " - empty." << std::endl;
			return;
		}

		Print(m_rootNode.get(), 0);
#endif
	}

	void Layout::Print(LayoutNode* node, uint32_t level)
	{
		for (size_t i = 0; i < level; i++)
		{
			std::cout << " ";
		}

		if (node->GetType() == LayoutNodeType::Container)
		{
			std::cout << "{Container}";
		}
		else if (node->GetType() == LayoutNodeType::DockPane)
		{
			std::cout << "{DockPane}";
		}
		else if (node->GetType() == LayoutNodeType::Dock)
		{
			std::cout << "{Dock}";
		}
		else if (node->GetType() == LayoutNodeType::DockPaneTab)
		{
			std::cout << "{DockPaneTab}";
		}
		else if (node->GetType() == LayoutNodeType::Leaf)
		{
			std::cout << "{Leaf}";
		}
		else if (node->GetType() == LayoutNodeType::Splitter)
		{
			std::cout << "{Splitter}";
		}
		else
		{
			std::cout << "{UNKNOWN}";
		}

		std::cout << " id = " << node->GetId() << ". children = " << node->m_children.size() << std::endl;
		for (size_t i = 0; i < node->m_children.size(); i++)
		{
			Print(node->m_children[i].get(), level + 1);
		}
		for (size_t i = 0; i < level; i++)
		{
			std::cout << " ";
		}

		if (node->GetType() == LayoutNodeType::Container)
		{
			std::cout << "{/Container}";
		}
		else if (node->GetType() == LayoutNodeType::DockPane)
		{
			std::cout << "{/DockPane}";
		}
		else if (node->GetType() == LayoutNodeType::Dock)
		{
			std::cout << "{/Dock}";
		}
		else if (node->GetType() == LayoutNodeType::DockPaneTab)
		{
			std::cout << "{/DockPaneTab}";
		}
		else if (node->GetType() == LayoutNodeType::Leaf)
		{
			std::cout << "{/Leaf}";
		}
		else if (node->GetType() == LayoutNodeType::Splitter)
		{
			std::cout << "{/Splitter}";
		}
		else
		{
			std::cout << "{/UNKNOWN}";
		}
		std::cout << std::endl;
	}

	void LayoutControlContainer::AddWindow(Window* window)
	{
		m_windows.push_back({ window });
	}
}
