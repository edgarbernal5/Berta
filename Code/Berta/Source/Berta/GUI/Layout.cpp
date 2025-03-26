/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Layout.h"

#include "Berta/GUI/Control.h"
#include "Berta/GUI/Window.h"
#include "Berta/GUI/LayoutNodes.h"
#include "Berta/Controls/Form.h"

namespace Berta
{
	Layout::Layout() :
		m_parent(nullptr)
	{
		InitPaneIndicators();
	}

	Layout::Layout(Window* window)
	{
		InitPaneIndicators();
		Create(window);
	}

	Layout::~Layout()
	{
		m_fields.clear();
	}

	void Layout::AddPane(const std::string& paneId)
	{
		if (!m_rootNode)
			return;

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

		m_dockPaneFields[paneId] = newPaneNode.get();
		auto& paneInfo = m_dockPaneInfoFields[paneId];
		paneInfo.id = paneId;

		newPaneNode->m_dockArea = std::make_unique<DockArea>();
		newPaneNode->m_dockArea->Create(m_parent, &paneInfo);
		newPaneNode->m_dockArea->m_eventsNotifier = newPaneNode.get();

		newPaneNode->m_dockLayoutEvents = this;
		newPaneNode->m_paneId = paneId;
		newPaneNode->SetParentNode(dockRoot);
		newPaneNode->SetParentWindow(dockRoot->GetParentWindow());

		dockRoot->m_children.emplace_back(std::move(newPaneNode));
	}

	void Layout::AddPaneTab(const std::string& paneId, const std::string& tabId, ControlBase* control)
	{
		if (!m_rootNode)
			return;

		auto paneNode = GetPane(paneId);
		if (!paneNode)
		{
			return;
		}

		if (GetPaneTab(paneId, tabId))
		{
			return;
		}
		auto paneTabId = paneId + "/" + tabId;

		auto paneTabNode = std::make_unique<DockPaneTabLayoutNode>();
		paneTabNode->m_tabId = paneTabId;
		paneTabNode->SetParentNode(paneNode);
		paneTabNode->SetParentWindow(paneNode->GetParentWindow());
		m_dockPaneTabFields[paneTabId] = paneTabNode.get();

		paneNode->AddTab(tabId, control);
		paneNode->m_children.emplace_back(std::move(paneTabNode));
	}

	void Layout::AddPaneTab(const std::string& paneId, const std::string& tabId, ControlBase* control, const std::string& relativePaneId, DockPosition dockPosition)
	{
		if (!m_rootNode)
			return;

		if (GetPane(paneId) || GetPaneTab(paneId, tabId))
		{
			return;
		}

		auto dockRoot = m_rootNode->FindFirst(LayoutNodeType::Dock);
		auto relativePaneNode = relativePaneId.empty() ? (!dockRoot || !dockRoot->m_children.empty() ? nullptr : dockRoot) : GetPane(relativePaneId);
		if (!relativePaneNode)
			return;

		auto newPaneNode = std::make_unique<DockPaneLayoutNode>();

		auto newPaneNodePtr = newPaneNode.get();
		m_dockPaneFields[paneId] = newPaneNodePtr;
		auto& paneInfo = m_dockPaneInfoFields[paneId];
		paneInfo.id = paneId;

		newPaneNode->m_dockArea = std::make_unique<DockArea>();
		newPaneNode->m_dockArea->Create(m_parent, &paneInfo);
		newPaneNode->m_dockArea->m_eventsNotifier = newPaneNodePtr;

		newPaneNode->m_dockLayoutEvents = this;
		newPaneNode->m_paneId = paneId;
		newPaneNode->SetParentWindow(m_parent);

		auto paneTabId = paneId + "/" + tabId;

		auto paneTabNode = std::make_unique<DockPaneTabLayoutNode>();
		paneTabNode->m_tabId = paneTabId;
		paneTabNode->SetParentNode(newPaneNodePtr);
		paneTabNode->SetParentWindow(newPaneNode->GetParentWindow());
		
		m_dockPaneTabFields[paneTabId] = paneTabNode.get();

		newPaneNode->AddTab(tabId, control);
		newPaneNode->m_children.emplace_back(std::move(paneTabNode));

		m_floatingDockFields.emplace_back(std::move(newPaneNode));

		newPaneNodePtr->GetWindowsAreas().push_back({ newPaneNodePtr->m_dockArea->Handle() });
		if (DoDock(newPaneNodePtr, relativePaneNode, dockPosition))
		{

		}
	}

	void Layout::Apply()
	{
		if (!m_rootNode || !m_parent)
			return;

		m_rootNode->SetArea(GUI::AreaWindow(m_parent));
		m_rootNode->CalculateAreas();
		m_rootNode->Apply();
	}

	void Layout::Attach(const std::string& fieldId, Window* window)
	{
		auto& pair = m_fields[fieldId];

		if (pair == nullptr)
		{
			auto newNode = m_rootNode->Find(fieldId);
			pair = newNode;
		}
		pair->AddWindow(window);
	}

	void Layout::Create(Window* window)
	{
		if (!window)
		{
			return;
		}
		if (m_parent)
		{

		}
		m_parent = window;

		m_parent->Events->Resize.Connect([this](const ArgResize& args)
		{
			//TODO: add this same logic to visibility event?!
			if (m_rootNode)
			{
				m_rootNode->SetArea({ 0, 0, args.NewSize.Width, args.NewSize.Height });
				m_rootNode->CalculateAreas();
				m_rootNode->Apply();
			}

			//Print();
		});

		m_parent->Events->Destroy.Connect([](const ArgDestroy& args)
		{
			//
		});
	}

	void Layout::Parse(const std::string& source)
	{
		Layout::Parser parser(source);

		auto rootNode = parser.Parse();
		if (!rootNode)
		{
			return;
		}
		BT_CORE_TRACE << "Parse completed." << std::endl;

		m_rootNode = std::move(rootNode);
		m_rootNode->SetParentWindow(m_parent);
	}

	void Layout::NotifyFloat(DockPaneLayoutNode* node)
	{
		auto oldParentWindow = node->GetParentWindow();
		node->SetParentWindow(node->m_dockArea->m_nativeContainer->Handle());

		if (DoFloat(node))
		{
			Apply();
			GUI::UpdateTree(oldParentWindow);
			Print();
		}
	}

	void Layout::NotifyMove(LayoutNode* node)
	{
		if (!IsMouseInsideWindow())
		{
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

		auto paneNode = reinterpret_cast<DockPaneLayoutNode*>(node);
		DockPosition dockPosition = DockPosition::Tab;
		if (IsMouseInsideDockIndicator(&dockPosition))
		{
			/*auto newArea = paneNode->GetArea();
			auto newSize = paneNode->m_dockArea->GetSize();
			newArea.Width = newSize.Width;
			newArea.Height = newSize.Height;

			paneNode->SetArea(newArea);*/

			if (DoDock(paneNode, paneOrDock, dockPosition))
			{
				BT_CORE_TRACE << " - DoDock." << std::endl;
				//paneOrDock->Apply();
				Print();
			}
		}
		else
		{
			if (DoFloat(paneNode))
			{
				BT_CORE_TRACE << " - DoFloat." << std::endl;
				paneNode->CalculateAreas();
				paneNode->Apply();
				Print();
			}
		}
	}

	void Layout::NotifyMoveStopped(LayoutNode* node)
	{
		auto paneNode = reinterpret_cast<DockPaneLayoutNode*>(node);
		if (IsMouseInsideDockIndicator())
		{
			node->SetParentWindow(paneNode->m_dockArea->m_hostWindow);
			paneNode->m_dockArea->Dock();
			Apply();
			GUI::UpdateWindow(*paneNode->m_dockArea);
		}
		HidePaneDockIndicators();
	}

	void Layout::RequestClose(LayoutNode* node)
	{
		auto paneNode = reinterpret_cast<DockPaneLayoutNode*>(node);
		auto index = paneNode->m_dockArea->GetTabSelectedIndex();
		auto childNode = reinterpret_cast<DockPaneTabLayoutNode*>(paneNode->m_children[index].get());
		m_dockPaneTabFields.erase(childNode->m_tabId);

		paneNode->m_dockArea->m_tabBar->Erase(index);
		paneNode->m_children.erase(paneNode->m_children.begin() + index);

		bool needUpdate = false;
		m_dockPaneFields.erase(paneNode->m_paneId);
		if (paneNode->m_children.empty())
		{
			auto parent = paneNode->GetParentNode();
			if (parent == nullptr)
			{
				for (size_t i = 0; i < m_floatingDockFields.size(); i++)
				{
					if (m_floatingDockFields[i].get() == paneNode)
					{
						m_floatingDockFields.erase(m_floatingDockFields.begin() + i);
						break;
					}
				}
				//std::unique_ptr<DockPaneLayoutNode> remove(paneNode);
			}
			else
			{
				for (size_t i = 0; i < parent->m_children.size(); i++)
				{
					if (parent->m_children[i].get() == node)
					{
						parent->m_children.erase(parent->m_children.begin() + i);
						break;
					}
				}
				needUpdate = true;
			}
		}
		if (needUpdate)
		{
			GUI::UpdateWindow(m_parent);
			Apply();
		}
	}

	DockPaneLayoutNode* Layout::GetPane(const std::string& paneId)
	{
		auto it = m_dockPaneFields.find(paneId);
		if (it != m_dockPaneFields.end())
		{
			return it->second;
		}

		return nullptr;
	}

	DockPaneTabLayoutNode* Layout::GetPaneTab(const std::string& paneId, const std::string& tabId)
	{
		auto paneTabId = paneId + "/" + tabId;
		auto it = m_dockPaneTabFields.find(paneTabId);
		if (it != m_dockPaneTabFields.end())
		{
			return it->second;
		}

		return nullptr;
	}

	void Layout::InitPaneIndicators()
	{
		m_paneIndicators.emplace_back(new DockIndicator{ DockPosition::Up });
		m_paneIndicators.emplace_back(new DockIndicator{ DockPosition::Down });
		m_paneIndicators.emplace_back(new DockIndicator{ DockPosition::Left });
		m_paneIndicators.emplace_back(new DockIndicator{ DockPosition::Right });
		m_paneIndicators.emplace_back(new DockIndicator{ DockPosition::Tab });
	}

	void Layout::HidePaneDockIndicators()
	{
		for (auto& indicator : m_paneIndicators)
		{
			indicator->Docker.reset();
		}
	}

	void Layout::ShowPaneDockIndicators(LayoutNode* node)
	{
		auto indicatorSize = m_parent->ToScale(32);
		auto indicatorSizeHalf = indicatorSize >> 1;
		auto indicatorSizeOffset = indicatorSizeHalf >> 1;

		for (auto& indicator : m_paneIndicators)
		{
			/*if (node->GetType() == LayoutNodeType::Dock && indicator->Position != DockPosition::Tab)
			{
				if (indicator->Docker)
				{
					indicator->Docker.reset();
				}
				continue;
			}*/
			auto nodeArea = node->GetArea();
			auto x = nodeArea.X + static_cast<int>(nodeArea.Width) / 2;
			auto y = nodeArea.Y + static_cast<int>(nodeArea.Height) / 2;

			if (!indicator->Docker)
			{
				if (indicator->Position == DockPosition::Tab)
				{
					Point position{ x - indicatorSizeHalf, y - indicatorSizeHalf };
					indicator->Docker = std::make_unique<Form>(m_parent, Rectangle{ position.X, position.Y, (uint32_t)indicatorSize, (uint32_t)indicatorSize }, FormStyle::Flat());
					indicator->Docker->GetAppearance().Background = Colors::Light_ButtonBackground;
				}
				else if (indicator->Position == DockPosition::Up)
				{
					Point position{ x - indicatorSizeHalf, y - indicatorSize - indicatorSizeHalf - indicatorSizeOffset };
					indicator->Docker = std::make_unique<Form>(m_parent, Rectangle{ position.X, position.Y, (uint32_t)indicatorSize, (uint32_t)indicatorSize }, FormStyle::Flat());
					indicator->Docker->GetAppearance().Background = Colors::Light_ButtonBackground;
				}
				else if (indicator->Position == DockPosition::Down)
				{
					Point position{ x - indicatorSizeHalf, y + indicatorSizeHalf + indicatorSizeOffset };
					indicator->Docker = std::make_unique<Form>(m_parent, Rectangle{ position.X, position.Y, (uint32_t)indicatorSize, (uint32_t)indicatorSize }, FormStyle::Flat());
					indicator->Docker->GetAppearance().Background = Colors::Light_ButtonBackground;
				}
				else if (indicator->Position == DockPosition::Left)
				{
					Point position{ x - indicatorSizeHalf - indicatorSize - indicatorSizeOffset, y - indicatorSizeHalf };
					indicator->Docker = std::make_unique<Form>(m_parent, Rectangle{ position.X, position.Y, (uint32_t)indicatorSize, (uint32_t)indicatorSize }, FormStyle::Flat());
					indicator->Docker->GetAppearance().Background = Colors::Light_ButtonBackground;
				}
				else if (indicator->Position == DockPosition::Right)
				{
					Point position{ x + indicatorSizeHalf + indicatorSizeOffset, y - indicatorSizeHalf };
					indicator->Docker = std::make_unique<Form>(m_parent, Rectangle{ position.X, position.Y, (uint32_t)indicatorSize, (uint32_t)indicatorSize }, FormStyle::Flat());
					indicator->Docker->GetAppearance().Background = Colors::Light_ButtonBackground;
				}
				//GUI::MakeWindowActive(*indicator->Docker, false, nullptr);
				
				if (indicator->Docker)
				{
					std::ostringstream builder;
					builder << "Indicator-" << (int)indicator->Position;
					indicator->Docker->SetDebugName(builder.str());
					indicator->Docker->Show();
				}
			}
		}
	}

	bool Layout::IsMouseInsideWindow() const
	{
		if (!m_parent)
			return false;

		auto mousePosition = GUI::GetScreenMousePosition();
		auto windowPosition = GUI::GetPointClientToScreen(m_parent, GUI::GetAbsoluteRootPosition(m_parent));
		Rectangle rect{ windowPosition.X, windowPosition.Y, m_parent->Size.Width, m_parent->Size.Height };

		return rect.IsInside(mousePosition);
	}

	bool Layout::IsMouseInsideDockIndicator(DockPosition* outDockPosition) const
	{
		for (auto& indicator : m_paneIndicators)
		{
			if (!indicator->Docker)
			{
				continue;
			}

			auto mousePosition = GUI::GetScreenMousePosition();
			auto dockerHandle = indicator->Docker->Handle();
			auto windowPosition = GUI::GetPointClientToScreen(dockerHandle, GUI::GetAbsoluteRootPosition(dockerHandle));
			Rectangle rect{ windowPosition.X, windowPosition.Y, dockerHandle->Size.Width, dockerHandle->Size.Height };

			if (rect.IsInside(mousePosition))
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
		if (!m_parent || !m_rootNode)
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
				auto windowPosition = GUI::GetPointClientToScreen(m_parent, GUI::GetAbsoluteRootPosition(m_parent));
				auto nodeArea = node->GetArea();
				Rectangle rect
				{
					windowPosition.X + nodeArea.X, windowPosition.Y + nodeArea.Y,
					nodeArea.Width, nodeArea.Height
				};

				if (rect.IsInside(mousePosition))
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

				if (i == parent->m_children.size() - 1)
				{
					parent->m_children.pop_back();
					if (parent->m_children.size())
					{
						parent->m_children.pop_back();
					}
				}
				else
				{
					parent->m_children.erase(parent->m_children.begin() + i);
					if (parent->m_children.size())
					{
						parent->m_children.erase(parent->m_children.begin() + i);
						if (i > 0 && i < parent->m_children.size())
						{
							parent->m_children[i - 1]->SetNext(parent->m_children[i].get());
						}
					}
				}

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
								child->SetNext(parentParent->m_children[j]->GetNext());
								parentParent->m_children[j].reset(child);
								if (j > 0)
									parentParent->m_children[j - 1]->SetNext(child);

								//parentParent->weight.reset();
								//child->weight.reset();
								child->SetParentNode(parentParent);
								break;
							}
						}
					}
				}
				break;
			}
		}

		return true;
	}

	bool Layout::DoDock(DockPaneLayoutNode* paneNode, LayoutNode* target, DockPosition dockPosition)
	{
		size_t nodeIndex;
		if (IsAlreadyDocked(paneNode, nodeIndex))
			return false;

		if (target && target->GetType() == LayoutNodeType::Dock && target->m_children.empty())
		{
			paneNode->SetParentNode(target);
			target->m_children.emplace_back(std::move(m_floatingDockFields[nodeIndex]));
			m_floatingDockFields.erase(m_floatingDockFields.begin() + nodeIndex);

			return true;
		}

		/*if (target == nullptr)
		{
			paneNode->SetParentNode(target);
			target->m_children.emplace_back(std::move(m_floatingDockFields[nodeIndex]));
			m_floatingDockFields.erase(m_floatingDockFields.begin() + nodeIndex);
			return true;
		}*/

		LayoutNode* dockRootNode = target;
		while (dockRootNode)
		{
			if (dockRootNode->GetType() == LayoutNodeType::Dock)
			{
				break;
			}

			dockRootNode = dockRootNode->GetParentNode();
		}

		if (dockRootNode == nullptr)
		{
			return false;
		}

		auto targetParent = reinterpret_cast<ContainerLayoutNode*>(target ? target->GetParentNode() : dockRootNode);
		bool addNewOrientation = false;
		if (dockRootNode->m_children[0]->GetType() == LayoutNodeType::DockPane)
		{
			addNewOrientation = true;
		}
		else
		{
			if (targetParent->GetOrientation() != (dockPosition == DockPosition::Up || dockPosition == DockPosition::Down))
			{
				addNewOrientation = true;
			}
		}

		auto targetIndex = target->GetIndex();
		if (addNewOrientation)
		{
			std::unique_ptr<LayoutNode> targetPtr = std::move(target->GetParentNode()->m_children[targetIndex]);

			auto containerPtr = new ContainerLayoutNode(dockPosition == DockPosition::Up || dockPosition == DockPosition::Down);
			containerPtr->SetParentNode(target->GetParentNode());
			containerPtr->SetParentWindow(target->GetParentWindow());

			auto splitterPtr = new SplitterLayoutNode(dockPosition == DockPosition::Up || dockPosition == DockPosition::Down);
			splitterPtr->SetParentNode(containerPtr->GetParentNode());
			splitterPtr->SetParentWindow(target->GetParentWindow());

			paneNode->SetParentNode(containerPtr);
			targetPtr->SetParentNode(containerPtr);
			containerPtr->SetNext(target->GetNext());

			if (dockPosition == DockPosition::Up || dockPosition == DockPosition::Left)
			{
				paneNode->SetNext(splitterPtr);
				splitterPtr->SetNext(target);

				containerPtr->m_children.emplace_back(std::move(m_floatingDockFields[nodeIndex]));
				containerPtr->m_children.emplace_back(splitterPtr);
				containerPtr->m_children.emplace_back(targetPtr.release());

				containerPtr->m_children[1]->SetPrev(containerPtr->m_children[0].get());
				containerPtr->m_children[1]->SetNext(containerPtr->m_children[2].get());
			}
			else
			{
				targetPtr->SetNext(splitterPtr);
				splitterPtr->SetNext(paneNode);

				containerPtr->m_children.emplace_back(targetPtr.release());
				containerPtr->m_children.emplace_back(splitterPtr);
				containerPtr->m_children.emplace_back(std::move(m_floatingDockFields[nodeIndex]));

				containerPtr->m_children[1]->SetPrev(containerPtr->m_children[0].get());
				containerPtr->m_children[1]->SetNext(containerPtr->m_children[2].get());
			}

			if (targetIndex == std::string::npos)
			{
				return false;
			}
			else
			{
				targetParent->m_children[targetIndex].reset(containerPtr);
				if (targetIndex > 0)
				{
					targetParent->m_children[targetIndex - 1]->SetNext(containerPtr);
				}
			}
		}
		else
		{
			auto splitterPtr = new SplitterLayoutNode(dockPosition == DockPosition::Up || dockPosition == DockPosition::Down);
			splitterPtr->SetParentNode(targetParent);
			splitterPtr->SetParentWindow(target->GetParentWindow());

			paneNode->SetParentNode(targetParent);

			if (dockPosition == DockPosition::Up || dockPosition == DockPosition::Left)
			{
				if (targetIndex == std::string::npos)
					targetIndex = 0;

				targetParent->m_children.emplace(targetParent->m_children.begin() + targetIndex, std::move(m_floatingDockFields[nodeIndex]));
				targetParent->m_children.emplace(targetParent->m_children.begin() + targetIndex + 1, splitterPtr);
			}
			else
			{
				if (targetIndex == std::string::npos)
					targetIndex = targetParent->m_children.size() - 1;


				targetParent->m_children.emplace(targetParent->m_children.begin() + targetIndex + 1, splitterPtr);
				targetParent->m_children.emplace(targetParent->m_children.begin() + targetIndex + 2, std::move(m_floatingDockFields[nodeIndex]));
			}

			for (size_t i = 0; i < targetParent->m_children.size(); i++)
			{
				if (i == targetParent->m_children.size() - 1)
					targetParent->m_children[i]->SetNext(nullptr);
				else
					targetParent->m_children[i]->SetNext(targetParent->m_children[i + 1].get());
			}
		}
		m_floatingDockFields.erase(m_floatingDockFields.begin() + nodeIndex);

		return true;
	}

	void Layout::Print()
	{
		std::cout << "Print()" << std::endl;
		if (!m_rootNode)
		{
			std::cout << " - empty." << std::endl;
			return;
		}

		Print(m_rootNode.get(), 0);
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

	Tokenizer::Tokenizer(const std::string& source) : 
		m_buffer(source.c_str()),
		m_bufferEnd(source.c_str() + source.size())
	{
	}

	void Tokenizer::Next()
	{
		while (SkipWhitespace()) {}

		if (m_error)
		{
			m_token = Token::Type::EndOfStream;
			return;
		}

		if (m_buffer >= m_bufferEnd || *m_buffer == '\0')
		{
			m_token = Token::Type::EndOfStream;
			return;
		}

		if (m_buffer[0] == '{')
		{
			m_token = Token::Type::OpenBrace;
			++m_buffer;
			return;
		}
		if (m_buffer[0] == '}')
		{
			m_token = Token::Type::CloseBrace;
			++m_buffer;
			return;
		}
		if (m_buffer[0] == '=')
		{
			m_token = Token::Type::Equal;
			++m_buffer;
			return;
		}
		if (m_buffer[0] == '%')
		{
			m_token = Token::Type::Percentage;
			++m_buffer;
			return;
		}
		if (m_buffer[0] == '|')
		{
			m_token = Token::Type::Splitter;
			++m_buffer;
			return;
		}

		if (ScanNumber())
		{
			return;
		}

		const char* start = m_buffer;
		while (m_buffer < m_bufferEnd && m_buffer[0] != 0 && !IsSymbol(m_buffer[0]) && !std::isspace(m_buffer[0]))
		{
			++m_buffer;
		}

		size_t length = m_buffer - start;
		memcpy(m_identifier, start, length);
		m_identifier[length] = 0;

		const int numReservedWords = sizeof(g_reservedWords) / sizeof(const char*);
		for (int i = 0; i < numReservedWords; ++i)
		{
			if (strcmp(g_reservedWords[i], m_identifier) == 0)
			{
				m_token = (Token::Type)(i + 256);
				return;
			}
		}

		m_token = Token::Type::Identifier;
	}

	void Tokenizer::GetTokenName(Token::Type token, char buffer[g_maxIdentifierLength])
	{
	}

	bool Tokenizer::SkipWhitespace()
	{
		bool result = false;
		while (m_buffer < m_bufferEnd && std::isspace(m_buffer[0]))
		{
			result = true;
			if (m_buffer[0] == '\n')
			{
				++m_lineNumber;
			}
			++m_buffer;
		}
		return result;
	}

	bool Tokenizer::IsSymbol(char ch)
	{
		switch (ch)
		{
		case '{':
		case '}':
		case '-':
		case '=':
		case '%':
		case '|':
			return true;
		}
		return false;
	}

	bool Tokenizer::IsNumberSeparator(char ch)
	{
		return ch == 0 || std::isspace(ch) || IsSymbol(ch);
	}

	bool Tokenizer::ScanNumber()
	{
		char* fEnd = nullptr;
		double dValue = std::strtod(m_buffer, &fEnd);

		if (fEnd == m_buffer)
		{
			return false;
		}

		char* iEnd = nullptr;
		long lValue = std::strtol(m_buffer, &iEnd, 10);

		if (fEnd > iEnd && IsNumberSeparator(fEnd[0]))
		{
			m_buffer = fEnd;
			m_token = Token::Type::NumberDouble; // it is a double.
			m_dValue = dValue;
			return true;
		}
		else if (iEnd > m_buffer && IsNumberSeparator(iEnd[0]))
		{
			m_buffer = iEnd;
			m_token = Token::Type::NumberInt; // it is a integer.
			m_iValue = static_cast<int>(lValue);
			return true;
		}
		return false;
	}

	Layout::Parser::Parser(const std::string& source) :
		m_source(source),
		m_tokenizer(source)
	{
	}

	std::unique_ptr<LayoutNode> Layout::Parser::Parse()
	{
		std::string identifier;
		std::vector<std::unique_ptr<LayoutNode>> children;
		std::unique_ptr<LayoutNode> node;
		bool isVertical = false;
		Token::Type dockType = Token::Type::None;
		std::unordered_map<std::string, LayoutNode::PropertyValue> properties;

		m_tokenizer.Next();
		Token::Type currentToken = m_tokenizer.GetToken();
		while (currentToken != Token::Type::EndOfStream && currentToken != Token::Type::CloseBrace)
		{
			bool moveToNextToken = true;
			switch (currentToken)
			{
			case Berta::Token::Type::Identifier:
				identifier = m_tokenizer.GetIdentifier();
				break;
			case Berta::Token::Type::OpenBrace:
			{
				auto child = Parse();

				children.emplace_back(std::move(child));
				break;
			}
			case Berta::Token::Type::Splitter:
			{
				auto splitterNode = std::make_unique<SplitterLayoutNode>(false);
				splitterNode->SetProperty("Dimension", Number{ SplitterLayoutNode::Size });

				children.emplace_back(std::move(splitterNode));
				break;
			}
			case Berta::Token::Type::VerticalLayout:
				isVertical = true;
				break;
			case Berta::Token::Type::HorizontalLayout:
				isVertical = false;
				break;
			case Berta::Token::Type::Dock:
				dockType = Berta::Token::Type::Dock;
				break;
			case Berta::Token::Type::DockPane:
				dockType = Berta::Token::Type::DockPane;
				break;
			case Berta::Token::Type::Width:
			case Berta::Token::Type::Height:
			{
				auto propertyName = currentToken == Berta::Token::Type::Width ? "Width" : "Height";
				
				m_tokenizer.Next();
				currentToken = m_tokenizer.GetToken();
				if (!Expect(Token::Type::Equal))
				{
					return nullptr;
				}
				bool isNumberInt = false;
				bool isNumberDouble = false;
				if ((isNumberInt = Accept(Token::Type::NumberInt)) || (isNumberDouble = Accept(Token::Type::NumberDouble)))
				{
					Number number;
					if (isNumberInt)
					{
						number.SetValue(m_tokenizer.GetInt());
					}
					else
					{
						number.SetValue(m_tokenizer.GetDouble());
					}

					if (Accept(Token::Type::Percentage))
					{
						number.isPercentage = true;
					}
					
					properties[propertyName] = number;
					moveToNextToken = !IsEqualTo(Token::Type::CloseBrace);
				}
				break;
			}
			case Berta::Token::Type::MinHeight:
			case Berta::Token::Type::MaxHeight:
			case Berta::Token::Type::MinWidth:
			case Berta::Token::Type::MaxWidth:
			{
				m_tokenizer.Next();
				currentToken = m_tokenizer.GetToken();
				if (!Expect(Token::Type::Equal))
				{
					return nullptr;
				}

				if (IsEqualTo(Token::Type::NumberInt))
				{
					Number number;
					number.SetValue(m_tokenizer.GetInt());

					auto propertyName = currentToken == Berta::Token::Type::MinHeight ? "MinHeight" : 
						(currentToken == Berta::Token::Type::MaxHeight ? "MaxHeight" : 
							(currentToken == Berta::Token::Type::MinWidth ? "MinWidth" : "MaxWidth"));

					properties[propertyName] = number;
				}
				else
				{
					return nullptr;
				}
				break;
			}
			}
			if (moveToNextToken)
			{
				m_tokenizer.Next();
			}
			currentToken = m_tokenizer.GetToken();
		}

		switch (currentToken)
		{
		case Token::Type::CloseBrace:
		case Token::Type::EndOfStream:
			if (dockType == Berta::Token::Type::Dock)
			{
				node = std::make_unique<DockLayoutNode>();
			}
			else if (dockType == Berta::Token::Type::DockPane)
			{
				node = std::make_unique<DockPaneLayoutNode>();
			}
			else if (children.empty())
			{
				node = std::make_unique<LeafLayoutNode>();
			}
			else
			{
				node = std::make_unique<ContainerLayoutNode>(isVertical);
			}
			break;
		}

		for (size_t i = 0; i < children.size(); i++)
		{
			auto child = children[i].get();
			child->SetParentNode(node.get());
			if (i < children.size() - 1)
			{
				child->SetNext(children[i + 1].get());
			}
			if (i > 0)
			{
				child->SetPrev(children[i - 1].get());
			}

			if (child->GetType() == LayoutNodeType::Splitter)
			{
				auto splitterNode = reinterpret_cast<SplitterLayoutNode*>(child);
				splitterNode->SetOrientation(isVertical);

				auto dimension = child->GetProperty<Number>("Dimension");
				child->SetProperty(isVertical ? "Height" : "Width", dimension);
				child->RemoveProperty("Dimension");
			}
		}

		node->SetId(identifier);
		node->m_properties.swap(properties);
		node->m_children.swap(children);

		return node;
	}

	bool Layout::Parser::Accept(Token::Type tokenId)
	{
		if (m_tokenizer.GetToken() == tokenId)
		{
			m_tokenizer.Next();
			return true;
		}
		return false;
	}

	bool Layout::Parser::AcceptIdentifier(std::string& identifier)
	{
		if (m_tokenizer.GetToken() == Token::Type::Identifier)
		{
			identifier = m_tokenizer.GetIdentifier();
			m_tokenizer.Next();
			return true;
		}
		return false;
	}

	bool Layout::Parser::Expect(Token::Type tokenId)
	{
		if (!Accept(tokenId))
		{
			/*
			char want[HLSLTokenizer::s_maxIdentifier];
			m_tokenizer.GetTokenName(token, want);
			*/
			BT_CORE_ERROR << "error. expected token= " << (int)tokenId << std::endl;
			return false;
		}
		return true;
	}

	bool Layout::Parser::IsEqualTo(Token::Type tokenId)
	{
		return m_tokenizer.GetToken() == tokenId;
	}

	void LayoutControlContainer::AddWindow(Window* window)
	{
		m_windows.push_back({ window });
	}
}