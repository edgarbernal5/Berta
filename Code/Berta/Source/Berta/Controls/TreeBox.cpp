/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "TreeBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/EnumTypes.h"

#include <numeric>
#include <stack>

namespace Berta
{
	void TreeBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.m_window = control.Handle();
		m_module.m_appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());

		m_module.Init();
		m_module.CalculateViewport(m_module.m_viewport);
	}

	void TreeBoxReactor::Update(Graphics& graphics)
	{
		//BT_CORE_TRACE << " -- TreeBox Update() " << std::endl;
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();

		graphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->BoxBackground, true);

		if (m_module.m_showNavigationLines)
		{
			m_module.DrawNavigationLines(graphics);
		}
		m_module.DrawTreeNodes(graphics);

		if (m_module.m_viewport.m_needHorizontalScroll && m_module.m_viewport.m_needVerticalScroll)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			graphics.DrawRectangle({ (int)(m_module.m_window->ClientSize.Width - scrollSize) - 1, (int)(m_module.m_window->ClientSize.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background, true);
		}
		graphics.DrawRectangle(window->ClientSize.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void TreeBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.CalculateViewport(m_module.m_viewport);

		m_module.UpdateScrollBars();
		m_module.CalculateVisibleNodes();

		if (m_module.m_scrollBarVert)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->ClientSize.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->ClientSize.Height - 2u };
			GUI::MoveWindow(m_module.m_scrollBarVert->Handle(), scrollRect);
		}
	}

	void TreeBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		bool needUpdate = m_module.m_mouseSelection.m_hoveredNode != nullptr;
		m_module.m_mouseSelection.m_hoveredNode = nullptr;

		m_module.m_hoveredArea = InteractionArea::None;
		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void TreeBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_pressedArea = m_module.m_hoveredArea;
		bool needUpdate = false;
		bool emitSelectionEvent = false;

		if (args.ButtonState.LeftButton)
		{
			if (m_module.m_hoveredArea == InteractionArea::Expander)
			{
				auto nodeHeight = m_module.m_window->ToScale(m_module.m_appearance->TreeItemHeight);
				auto nodeHeightInt = static_cast<int>(nodeHeight);

				auto positionY = args.Position.Y - m_module.m_viewport.m_backgroundRect.Y + m_module.m_scrollOffset.Y;
				int index = positionY / nodeHeightInt;
				index -= m_module.m_viewport.m_startingVisibleIndex;

				m_module.m_visibleNodes[index]->isExpanded = !m_module.m_visibleNodes[index]->isExpanded;
				m_module.CalculateViewport(m_module.m_viewport);

				if (m_module.UpdateScrollBars())
				{
					if (m_module.m_scrollBarVert)
						m_module.m_scrollBarVert->Handle()->Renderer.Update();

					if (m_module.m_scrollBarHoriz)
						m_module.m_scrollBarHoriz->Handle()->Renderer.Update();
				}
				m_module.CalculateVisibleNodes();

				needUpdate = true;
				m_module.EmitExpansionEvent(m_module.m_visibleNodes[index]);
			}
		}

		if (m_module.m_pressedArea == InteractionArea::Node)
		{
			if (m_module.m_mouseSelection.m_hoveredNode)
			{
				if (m_module.m_multiselection)
				{
					if (m_module.HandleMultiSelection(m_module.m_mouseSelection.m_hoveredNode))
					{
						needUpdate = true;
						emitSelectionEvent = true;
					}
				}
				else
				{
					if (m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_hoveredNode))
					{
						needUpdate = true;
						emitSelectionEvent = true;
					}
				}
			}
		}

		if (needUpdate && m_module.m_hoveredArea == InteractionArea::Node && m_module.m_viewport.m_needVerticalScroll)
		{
			auto nodeHeightInt = static_cast<int>(m_module.m_window->ToScale(m_module.m_appearance->TreeItemHeight));
			auto selectedIndex = m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_hoveredNode);
			auto positionY = selectedIndex * nodeHeightInt - m_module.m_scrollOffset.Y;
			auto newValue = m_module.m_scrollOffset.Y;
			if (positionY < 0)
			{
				newValue = positionY + m_module.m_scrollOffset.Y;
			}
			else if (positionY + nodeHeightInt > static_cast<int>(m_module.m_viewport.m_backgroundRect.Height))
			{
				newValue = positionY + m_module.m_scrollOffset.Y - static_cast<int>(m_module.m_viewport.m_backgroundRect.Height) + nodeHeightInt;
			}
			if (m_module.m_scrollOffset.Y != newValue)
			{
				m_module.m_scrollOffset.Y = newValue;
				m_module.m_scrollBarVert->SetValue(newValue);
			}
		}

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}

		if (emitSelectionEvent)
		{
			m_module.EmitSelectionEvent();
		}
	}

	void TreeBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto hoveredArea = m_module.DetermineHoverArea(args.Position);
		bool needUpdate = false;
		
		if (hoveredArea == InteractionArea::Node || hoveredArea == InteractionArea::Expander)
		{
			auto nodeHeight = m_module.m_window->ToScale(m_module.m_appearance->TreeItemHeight);
			auto nodeHeightInt = static_cast<int>(nodeHeight);

			auto positionY = args.Position.Y - m_module.m_viewport.m_backgroundRect.Y + m_module.m_scrollOffset.Y;
			int index = positionY / nodeHeightInt;
			index -= m_module.m_viewport.m_startingVisibleIndex;

			needUpdate = m_module.m_visibleNodes[index] != m_module.m_mouseSelection.m_hoveredNode;
			m_module.m_mouseSelection.m_hoveredNode = m_module.m_visibleNodes[index];
		}
		else if (hoveredArea == InteractionArea::Blank)
		{
			needUpdate = m_module.m_mouseSelection.m_hoveredNode != nullptr;
			m_module.m_mouseSelection.m_hoveredNode = nullptr;
		}

		m_module.m_hoveredArea = hoveredArea;

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void TreeBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
	}

	void TreeBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		if (!m_module.m_scrollBarVert && args.IsVertical || !m_module.m_scrollBarHoriz && !args.IsVertical)
		{
			return;
		}

		int direction = args.WheelDelta > 0 ? -1 : 1;
		direction *= args.IsVertical ? m_module.m_scrollBarVert->GetStepValue() : m_module.m_scrollBarHoriz->GetStepValue();
		auto min = args.IsVertical ? m_module.m_scrollBarVert->GetMin() : m_module.m_scrollBarHoriz->GetMin();
		auto max = args.IsVertical ? m_module.m_scrollBarVert->GetMax() : m_module.m_scrollBarHoriz->GetMax();
		int newOffset = std::clamp((args.IsVertical ? m_module.m_scrollOffset.Y : m_module.m_scrollOffset.X) + direction, (int)min, (int)max);

		if (args.IsVertical && newOffset != m_module.m_scrollOffset.Y ||
			!args.IsVertical && newOffset != m_module.m_scrollOffset.X)
		{
			if (args.IsVertical)
			{
				m_module.m_scrollOffset.Y = newOffset;
				m_module.CalculateVisibleNodes();
				m_module.m_scrollBarVert->SetValue(newOffset);

				GUI::MarkAsNeedUpdate(m_module.m_scrollBarVert->Handle());
			}
			else
			{
				m_module.m_scrollOffset.X = newOffset;
				m_module.m_scrollBarHoriz->SetValue(newOffset);

				GUI::MarkAsNeedUpdate(m_module.m_scrollBarHoriz->Handle());
			}

			GUI::MarkAsNeedUpdate(*m_control);
		}
	}

	void TreeBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;

		bool needUpdate = false;
		bool recalculateVisibleNodes = false;
		bool emitSelectionEvent = false;
		bool emitCollaspedEvent = false;
		auto nodeHeightInt = static_cast<int>(m_module.m_window->ToScale(m_module.m_appearance->TreeItemHeight));

		if (args.Key == KeyboardKey::ArrowLeft && m_module.m_mouseSelection.m_selectedNode)
		{
			if (m_module.m_mouseSelection.m_selectedNode->firstChild && m_module.m_mouseSelection.m_selectedNode->isExpanded)
			{
				m_module.m_mouseSelection.m_selectedNode->isExpanded = false;

				recalculateVisibleNodes = true;
				needUpdate = true;
				emitCollaspedEvent = true;
			}
			else if (m_module.m_mouseSelection.m_selectedNode->parent != &m_module.m_root)
			{
				if (m_module.m_multiselection)
				{
					m_module.ClearSelection();
					if (m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_selectedNode->parent))
					{
						needUpdate = true;
						emitSelectionEvent = true;
					}
					recalculateVisibleNodes = true;
				}
				else if (!m_module.m_multiselection && m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_selectedNode->parent))
				{
					needUpdate = true;
					emitSelectionEvent = true;
				}
				recalculateVisibleNodes = true;
			}
		}
		else if (args.Key == KeyboardKey::ArrowRight && m_module.m_mouseSelection.m_selectedNode)
		{
			if (!m_module.m_mouseSelection.m_selectedNode->isExpanded)
			{
				m_module.m_mouseSelection.m_selectedNode->isExpanded = true;

				recalculateVisibleNodes = true;
				needUpdate = true;
				emitCollaspedEvent = true;
			}
			else if (m_module.m_mouseSelection.m_selectedNode->firstChild)
			{
				if (m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_selectedNode->firstChild))
				{
					needUpdate = true;
					emitSelectionEvent = true;
					recalculateVisibleNodes = true;
				}
			}
		}
		else if (args.Key == KeyboardKey::Home || args.Key == KeyboardKey::End)
		{
			int newIndex = args.Key == KeyboardKey::Home ? 0 : ((int)m_module.m_viewport.m_treeSize - 1);

			auto newNode = m_module.LocateNodeIndexInTree(newIndex);
			if (newNode && m_module.UpdateSingleSelection(newNode))
			{
				m_module.m_mouseSelection.m_selectedNode = newNode;
				needUpdate = true;
				emitSelectionEvent = true;
				recalculateVisibleNodes = true;
			}
		}
		else if (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown ||
			args.Key == KeyboardKey::PageUp || args.Key == KeyboardKey::PageDown)
		{
			int pageAmount = static_cast<int>(m_module.m_viewport.m_backgroundRect.Height / nodeHeightInt);
			int direction = (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::PageUp) ? -1 : 1;
			int amount = direction * ((args.Key == KeyboardKey::PageDown || args.Key == KeyboardKey::PageUp) ? pageAmount : 1);
			
			auto selectedIndex = (m_module.m_mouseSelection.m_selectedNode == nullptr? (direction == -1 ? (int)m_module.m_viewport.m_treeSize : -1) : m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_selectedNode));
			auto newItemIndex = selectedIndex + amount;
			if (newItemIndex >= 0 && newItemIndex < static_cast<int>(m_module.m_viewport.m_treeSize))
			{
				if (!m_module.m_ctrlPressed)
				{
					m_module.ClearSelection();
				}

				if (m_module.m_multiselection && m_module.m_shiftPressed && m_module.m_mouseSelection.m_pivotNode != nullptr)
				{
					auto startIndex = m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_pivotNode);
					auto endIndex = newItemIndex;
					int minIndex = (std::min)(startIndex, endIndex);
					int maxIndex = (std::max)(startIndex, endIndex);

					std::vector<TreeNodeType*> rangeNodes;
					m_module.GetNodesInBetween(minIndex, maxIndex, rangeNodes);
					for (auto& node : rangeNodes)
					{
						if (!node->isSelected)
						{
							node->isSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(node);
						}
					}
					needUpdate = true;
					emitSelectionEvent = true;
					m_module.m_mouseSelection.m_selectedNode = endIndex == maxIndex ? rangeNodes.back() : rangeNodes.front();
				}
				else if (m_module.m_ctrlPressed)
				{
					auto newSelectedNode = m_module.LocateNodeIndexInTree(newItemIndex);
					needUpdate = emitSelectionEvent = m_module.m_mouseSelection.m_selectedNode != newSelectedNode;
					m_module.m_mouseSelection.m_selectedNode = newSelectedNode;
				}
				else
				{
					auto newSelectedNode = m_module.LocateNodeIndexInTree(newItemIndex);
					needUpdate = emitSelectionEvent = m_module.m_mouseSelection.m_selectedNode != newSelectedNode;
					newSelectedNode->isSelected = true;

					m_module.m_mouseSelection.m_selections.push_back(newSelectedNode);
					m_module.m_mouseSelection.m_selectedNode = newSelectedNode;
					m_module.m_mouseSelection.m_pivotNode = newSelectedNode;
				}
			}
			
			/*else
			{
				int selectedIndex;
				if (m_module.IsVisibleNode(m_module.m_mouseSelection.m_selectedNode, selectedIndex))
				{
					int newIndex = selectedIndex + amount;
					if (newIndex >= 0 && newIndex < m_module.m_visibleNodes.size())
					{
						newIndex = std::clamp(newIndex, 0, (int)m_module.m_visibleNodes.size() - 1);

						if (!m_module.m_multiselection && m_module.UpdateSingleSelection(m_module.m_visibleNodes[newIndex]))
						{
							m_module.m_mouseSelection.m_selectedNode = m_module.m_visibleNodes[newIndex];
							needUpdate = true;
							emitSelectionEvent = true;
						}
					}
					else
					{
						selectedIndex = m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_selectedNode);
						int newIndex = std::clamp(selectedIndex + amount, 0, (int)m_module.m_viewport.m_treeSize - 1);

						auto newNode = m_module.LocateNodeIndexInTree(newIndex);
						if (newNode && m_module.UpdateSingleSelection(newNode))
						{
							m_module.m_mouseSelection.m_selectedNode = newNode;
							needUpdate = true;
							emitSelectionEvent = true;
							recalculateVisibleNodes = true;
						}
					}
				}
				else
				{
					selectedIndex = m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_selectedNode);

					int newIndex = std::clamp(selectedIndex + amount, 0, (int)m_module.m_viewport.m_treeSize - 1);
					auto newNode = m_module.LocateNodeIndexInTree(newIndex);
					if (newNode && m_module.UpdateSingleSelection(newNode))
					{
						m_module.m_mouseSelection.m_selectedNode = newNode;
						needUpdate = true;
						emitSelectionEvent = true;
						recalculateVisibleNodes = true;
					}
				}
			}*/
			
		}
		else if (args.Key == KeyboardKey::Space && m_module.m_ctrlPressed)
		{
			if (m_module.m_mouseSelection.m_selectedNode)
			{
				if (!m_module.m_multiselection && !m_module.m_mouseSelection.m_selections.empty())
				{
					m_module.m_mouseSelection.m_selections[0]->isSelected = false;
					m_module.m_mouseSelection.m_selections.clear();
				}

				auto& isSelected = m_module.m_mouseSelection.m_selectedNode->isSelected;
				isSelected = !isSelected;
				if (isSelected)
				{
					m_module.m_mouseSelection.Select(m_module.m_mouseSelection.m_selectedNode);
				}
				else
				{
					m_module.m_mouseSelection.Deselect(m_module.m_mouseSelection.m_selectedNode);
				}

				if (m_module.m_multiselection)
				{
					m_module.m_mouseSelection.m_pivotNode = m_module.m_mouseSelection.m_selectedNode;
				}
				needUpdate = true;
			}
		}

		if (emitSelectionEvent && m_module.m_viewport.m_needVerticalScroll)
		{
			auto selectedIndex = m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_selectedNode);
			auto positionY = selectedIndex * nodeHeightInt - m_module.m_scrollOffset.Y;
			auto newValue = m_module.m_scrollOffset.Y;
			if (positionY < 0)
			{
				newValue = positionY + m_module.m_scrollOffset.Y;
				needUpdate = true;
			}
			else if (positionY + nodeHeightInt > static_cast<int>(m_module.m_viewport.m_backgroundRect.Height))
			{
				newValue = positionY + m_module.m_scrollOffset.Y - static_cast<int>(m_module.m_viewport.m_backgroundRect.Height) + nodeHeightInt;
				needUpdate = true;
			}
			if (m_module.m_scrollOffset.Y != newValue)
			{
				m_module.m_scrollOffset.Y = newValue;
				m_module.m_scrollBarVert->SetValue(newValue);
			}
		}

		//if (m_module.m_multiselection && emitSelectionEvent)
		//{
		//	for (auto& oldNode : m_module.m_mouseSelection.m_selections)
		//	{
		//		oldNode->isSelected = false;
		//	}
		//	m_module.m_mouseSelection.m_selections.clear();
		//}

		if (recalculateVisibleNodes)
		{
			m_module.CalculateViewport(m_module.m_viewport);
			m_module.CalculateVisibleNodes();
		}

		bool updateScrollBars = m_module.UpdateScrollBars();
		if (updateScrollBars)
		{
			if (m_module.m_scrollBarVert)
				m_module.m_scrollBarVert->Handle()->Renderer.Update();

			if (m_module.m_scrollBarHoriz)
				m_module.m_scrollBarHoriz->Handle()->Renderer.Update();
		}

		if (emitSelectionEvent)
		{
			m_module.EmitSelectionEvent();
		}

		if (emitCollaspedEvent)
		{
			m_module.EmitExpansionEvent(m_module.m_mouseSelection.m_selectedNode);
		}

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void TreeBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}

	bool TreeBoxReactor::MouseSelection::IsSelected(TreeNodeType* node) const
	{
		return std::find(m_selections.begin(), m_selections.end(), node) != m_selections.end();
	}

	void TreeBoxReactor::MouseSelection::Select(TreeNodeType* node)
	{
		m_selections.push_back(node);
	}

	void TreeBoxReactor::MouseSelection::Deselect(TreeNodeType* node)
	{
		auto it = std::find(m_selections.begin(), m_selections.end(), node);
		if (it != m_selections.end())
		{
			m_selections.erase(it);
		}
	}

	void TreeBoxReactor::Module::CalculateViewport(ViewportData& viewportData)
	{
		viewportData.m_backgroundRect = m_window->ClientSize.ToRectangle();
		viewportData.m_backgroundRect.X = viewportData.m_backgroundRect.Y = 1;
		viewportData.m_backgroundRect.Width -= 2u;
		viewportData.m_backgroundRect.Height -= 2u;

		auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
		viewportData.m_treeSize = CalculateTreeSize(&m_root) - 1;
		viewportData.m_contentSize.Height = nodeHeight * viewportData.m_treeSize;

		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		viewportData.m_needVerticalScroll = viewportData.m_contentSize.Height > viewportData.m_backgroundRect.Height;
		if (viewportData.m_needVerticalScroll)
		{
			viewportData.m_backgroundRect.Width -= scrollSize;
		}
		viewportData.m_contentSize.Width = viewportData.m_backgroundRect.Width;
	}

	void TreeBoxReactor::Module::CalculateVisibleNodes()
	{
		m_visibleNodes.clear();
		if (m_root.firstChild == nullptr)
		{
			m_viewport.m_startingVisibleIndex = m_viewport.m_endingVisibleIndex = 0;
			return;
		}

		auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
		auto visibleHeight = m_viewport.m_backgroundRect.Height;
		
		m_viewport.m_startingVisibleIndex = m_scrollOffset.Y / nodeHeight;
		m_viewport.m_endingVisibleIndex = (m_scrollOffset.Y + visibleHeight) / nodeHeight;

		GetNodesInBetween(m_viewport.m_startingVisibleIndex, m_viewport.m_endingVisibleIndex, m_visibleNodes);
	}

	void TreeBoxReactor::Module::GetNodesInBetween(int startIndex, int endIndex, std::vector< TreeNodeType*>& nodes) const
	{
		int index = 0;

		std::stack<TreeNodeType*> stack;
		stack.push(m_root.firstChild);

		while (!stack.empty())
		{
			TreeNodeType* current = stack.top();
			stack.pop();

			if (index >= startIndex && index <= endIndex)
			{
				nodes.emplace_back(current);
			}

			++index;

			if (index > endIndex)
				break;

			if (current->nextSibling)
			{
				stack.push(current->nextSibling);
			}

			if (current->isExpanded && current->firstChild)
			{
				stack.push(current->firstChild);
			}
		}
	}

	uint32_t TreeBoxReactor::Module::CalculateTreeSize(TreeNodeType* node)
	{
		if (node == nullptr)
		{
			return 0;
		}

		uint32_t size = 1;
		if (node->isExpanded)
		{
			for (TreeNodeType* child = node->firstChild; child; child = child->nextSibling)
			{
				size += CalculateTreeSize(child);
			}
		}

		return size;
	}

	uint32_t TreeBoxReactor::Module::CalculateNodeDepth(TreeNodeType* node)
	{
		uint32_t depth = 0;
		auto current = node->parent;
		while (current)
		{
			++depth;
			current = current->parent;
		}
		return depth;
	}

	void TreeBoxReactor::Module::Clear()
	{
		bool needUpdate = m_root.firstChild != nullptr;
		m_root.firstChild = nullptr;
		m_visibleNodes.clear();
		m_root.m_lookup.clear();

		CalculateViewport(m_viewport);
		UpdateScrollBars();

		if (needUpdate)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	Berta::TreeNodeType* TreeBoxReactor::Module::GetNextVisible(TreeNodeType* node)
	{
		if (node == nullptr)
			return nullptr;

		if (node->isExpanded)
		{
			if (node->firstChild == nullptr)
				return nullptr;

			return node->firstChild;
		}

		if (node->nextSibling != nullptr)
		{
			return node->nextSibling;
		}

		auto parent = node->parent;
		if (parent == &m_root)
			return nullptr;

		//GetNextVisible(parent);

		return nullptr;
	}

	int TreeBoxReactor::Module::LocateNodeIndexInTree(TreeNodeType* node) const
	{
		int index = 0;

		std::stack<TreeNodeType*> stack;
		stack.push(m_root.firstChild);

		while (!stack.empty())
		{
			TreeNodeType* current = stack.top();
			stack.pop();

			if (current == node)
				return index;

			++index;

			if (current->nextSibling)
			{
				stack.push(current->nextSibling);
			}

			if (current->isExpanded && current->firstChild)
			{
				stack.push(current->firstChild);
			}
		}

		return -1;
	}

	TreeNodeType* TreeBoxReactor::Module::LocateNodeIndexInTree(int nodeIndex) const
	{
		int index = 0;

		std::stack<TreeNodeType*> stack;
		stack.push(m_root.firstChild);

		while (!stack.empty())
		{
			TreeNodeType* current = stack.top();
			stack.pop();

			if (index == nodeIndex)
				return current;

			++index;

			if (current->nextSibling)
			{
				stack.push(current->nextSibling);
			}

			if (current->isExpanded && current->firstChild)
			{
				stack.push(current->firstChild);
			}
		}

		return nullptr;
	}

	TreeBoxReactor::InteractionArea TreeBoxReactor::Module::DetermineHoverArea(const Point& mousePosition)
	{
		if (!m_window->ClientSize.IsInside(mousePosition))
		{
			return InteractionArea::None;
		}

		auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
		auto nodeHeightInt = static_cast<int>(nodeHeight);

		auto positionY = mousePosition.Y - m_viewport.m_backgroundRect.Y + m_scrollOffset.Y;
		int index = positionY / nodeHeightInt;

		index -= m_viewport.m_startingVisibleIndex;
		if (index >= m_visibleNodes.size())
		{
			return InteractionArea::Blank;
		}

		auto expanderSize = m_window->ToScale(m_appearance->ExpanderButtonSize);
		auto depthWidthMultiplier = m_window->ToScale(m_appearance->DepthWidthMultiplier);
		auto nodeDepth = CalculateNodeDepth(m_visibleNodes[index]);
		int nodeOffset = (nodeDepth - 1) * depthWidthMultiplier;
		int expanderMarginX = static_cast<int>(depthWidthMultiplier - expanderSize) >> 1;
		
		Rectangle expanderRect
		{ 
			m_viewport.m_backgroundRect.X + m_scrollOffset.X + nodeOffset + expanderMarginX,
			(index + m_viewport.m_startingVisibleIndex) * nodeHeightInt + (int)(nodeHeightInt - expanderSize) / 2,
			expanderSize, expanderSize
		};
		
		Point absolutePosition = mousePosition;
		absolutePosition.Y += m_scrollOffset.Y;
		absolutePosition.X += m_scrollOffset.X;
		if (m_visibleNodes[index]->firstChild && expanderRect.IsInside(absolutePosition))
		{
			return InteractionArea::Expander;
		}

		if (absolutePosition.X >= expanderRect.X + static_cast<int>(expanderRect.Width))
		{
			return InteractionArea::Node;
		}

		return InteractionArea::Blank;
	}

	void TreeBoxReactor::Module::Update()
	{
		CalculateViewport(m_viewport);
		CalculateVisibleNodes();

		bool updateScrollBars = UpdateScrollBars();
		if (updateScrollBars)
		{
			if (m_scrollBarVert)
				m_scrollBarVert->Handle()->Renderer.Update();

			if (m_scrollBarHoriz)
				m_scrollBarHoriz->Handle()->Renderer.Update();
		}
	}

	void TreeBoxReactor::Module::Draw()
	{
		GUI::UpdateWindow(m_window);
	}

	void TreeBoxReactor::Module::DrawTreeNodes(Graphics& graphics)
	{
		bool enabled = true;
		auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
		auto nodeTextMargin = m_window->ToScale(4u);
		auto nodeHeightInt = static_cast<int>(nodeHeight);
		auto nodeHeightHalfInt = nodeHeightInt >> 1;
		Point offset{ m_viewport.m_backgroundRect.X - m_scrollOffset.X, m_viewport.m_backgroundRect.Y - m_scrollOffset.Y };

		auto iconSize = m_window->ToScale(m_window->Appearance->SmallIconSize);
		auto iconMargin = m_window->ToScale(2);
		auto expanderSize = m_window->ToScale(m_appearance->ExpanderButtonSize);
		auto depthWidthMultiplier = m_window->ToScale(m_appearance->DepthWidthMultiplier);

		int expanderMarginX = static_cast<int>(depthWidthMultiplier - expanderSize) >> 1;
		int expanderMarginY = (nodeHeightInt - static_cast<int>(expanderSize)) >> 1;

		int i = m_viewport.m_startingVisibleIndex;
		for (auto& node : m_visibleNodes)
		{
			auto depth = CalculateNodeDepth(node);
			int depthOffsetX = static_cast<int>((depth - 1) * depthWidthMultiplier);

			Rectangle expanderRect{ offset.X + depthOffsetX + expanderMarginX, offset.Y + nodeHeightInt * i + expanderMarginY, expanderSize, expanderSize };
			Rectangle nodeRect{ offset.X + depthOffsetX + (int)depthWidthMultiplier, offset.Y + nodeHeightInt * i, m_viewport.m_contentSize.Width, nodeHeight };
			nodeRect.Width -= nodeRect.X;

			bool isLastSelected = node == m_mouseSelection.m_selectedNode;
			bool isSelected = node->isSelected;
			bool isHovered = node == m_mouseSelection.m_hoveredNode;

			if (isSelected)
			{
				auto lineColor = enabled ? (isLastSelected ? m_appearance->Foreground2nd : (isSelected ? m_appearance->BoxBorderHighlightColor : m_appearance->BoxBorderColor)) : m_appearance->BoxBorderDisabledColor;

				graphics.DrawRectangle(nodeRect, m_window->Appearance->HighlightColor, true);
				graphics.DrawRectangle(nodeRect, lineColor, false);
			}
			else if (isHovered)
			{
				graphics.DrawRectangle(nodeRect, m_window->Appearance->ItemCollectionHightlightBackground, true);
			}
			else if (isLastSelected)
			{
				graphics.DrawRectangle(nodeRect, m_window->Appearance->Foreground2nd, false);
			}

			int contentOffsetX = 0;
			if (m_drawImages)
			{
				if (node->icon)
				{
					auto positionY = (nodeHeight - iconSize) >> 1;
					node->icon.Paste(graphics, { nodeRect.X + iconMargin, nodeRect.Y + (int)positionY, iconSize , iconSize });
				}
				contentOffsetX += iconSize + iconMargin * 2;
			}

			if (node->firstChild)
			{
				int arrowWidth = m_window->ToScale(4);
				int arrowLength = m_window->ToScale(2);
				graphics.DrawRoundRectBox(expanderRect, m_window->Appearance->Background, m_window->Appearance->BoxBorderColor, true);
			
				graphics.DrawArrow(expanderRect,
					arrowLength,
					arrowWidth,
					node->isExpanded ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right,
					m_window->Appearance->Foreground2nd,
					true,
					node->isExpanded ? m_window->Appearance->Foreground2nd : m_window->Appearance->BoxBackground
				);
			}

			graphics.DrawString({ nodeRect.X + contentOffsetX + (int)nodeTextMargin, nodeRect.Y + (int)(nodeHeight - graphics.GetTextExtent().Height) / 2 }, node->text, m_window->Appearance->Foreground);
			++i;
		}
	}

	void TreeBoxReactor::Module::DrawNavigationLines(Graphics& graphics)
	{
		auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
		auto nodeTextMargin = m_window->ToScale(8u);
		auto nodeHeightInt = static_cast<int>(nodeHeight);
		auto nodeHeightHalfInt = nodeHeightInt >> 1;
		Point offset{ m_viewport.m_backgroundRect.X - m_scrollOffset.X, m_viewport.m_backgroundRect.Y - m_scrollOffset.Y };

		auto iconSize = m_window->ToScale(m_window->Appearance->SmallIconSize);
		auto expanderSize = m_window->ToScale(m_appearance->ExpanderButtonSize);
		auto depthWidthMultiplier = m_window->ToScale(m_appearance->DepthWidthMultiplier);

		int expanderMarginX = static_cast<int>(depthWidthMultiplier - expanderSize) >> 1;
		int expanderMarginY = (nodeHeightInt - static_cast<int>(expanderSize)) >> 1;

		auto lineColor = m_window->Appearance->BoxBorderColor;
		Graphics::LineStyle lineStyle = Graphics::LineStyle::Dotted;
		int lineWidth = m_window->ToScale(1);

		uint32_t minDepth = (std::numeric_limits<uint32_t>::max)();
		int i = m_viewport.m_startingVisibleIndex;
		for (auto& node : m_visibleNodes)
		{
			auto depth = CalculateNodeDepth(node);
			int depthOffsetX = static_cast<int>((depth - 1) * depthWidthMultiplier);

			Rectangle expanderRect{ offset.X + depthOffsetX + expanderMarginX, offset.Y + nodeHeightInt * i + expanderMarginY, expanderSize, expanderSize };
			Rectangle nodeRect{ offset.X + depthOffsetX + (int)depthWidthMultiplier, offset.Y + nodeHeightInt * i, m_viewport.m_contentSize.Width, nodeHeight };
			nodeRect.Width -= nodeRect.X;

			auto expanderRectMidX = static_cast<int>((expanderRect.X * 2 + expanderRect.Width)) >> 1;
			Point startPointV{ expanderRectMidX, nodeRect.Y };
			Point endPointV{ startPointV.X, nodeRect.Y + nodeHeightInt };

			if (i == 0)
			{
				startPointV.Y += nodeHeightHalfInt;
			}

			if (node->prevSibling && !IsVisibleNode(node->prevSibling))
			{
				startPointV.Y = offset.Y + nodeHeightInt * m_viewport.m_startingVisibleIndex;
			}

			if (!node->nextSibling)
			{
				endPointV.Y -= nodeHeightHalfInt;
			}
			else
			{
				int nextSiblingIndex = -1;
				if (IsVisibleNode(node->nextSibling, nextSiblingIndex))
				{
					endPointV.Y = m_viewport.m_backgroundRect.Y + offset.Y + nodeHeightInt * (nextSiblingIndex + m_viewport.m_startingVisibleIndex);
				}
				else
				{
					endPointV.Y = offset.Y + nodeHeightInt * (m_viewport.m_endingVisibleIndex + 1);
				}
			}
			graphics.DrawLine(startPointV, endPointV, lineWidth, lineColor, lineStyle);
			
			Point startPointH{ expanderRectMidX + lineWidth * 2, nodeRect.Y + nodeHeightHalfInt };
			Point endPointH{ startPointH.X + (int)(depthWidthMultiplier / 2) - lineWidth * 2, startPointH.Y};
			graphics.DrawLine(startPointH, endPointH, lineWidth, lineColor, lineStyle);

			minDepth = (std::min)(minDepth, depth);
			++i;
		}

		if (minDepth > 1 && minDepth < (std::numeric_limits<uint32_t>::max)())
		{
			auto currentDepth = minDepth - 1;
			auto parentVisible = m_visibleNodes[0]->parent;
			auto depthParentVisible = CalculateNodeDepth(parentVisible);
			while (parentVisible && depthParentVisible > currentDepth)
			{
				parentVisible = parentVisible->parent;
				--depthParentVisible;
			}

			while (currentDepth > 0)
			{
				if (parentVisible && !parentVisible->nextSibling)
				{
					--currentDepth;
					parentVisible = parentVisible->parent;
					continue;
				}

				int nodeOffsetX = (currentDepth - 1) * depthWidthMultiplier;

				Point startPointV{ offset.X + nodeOffsetX + expanderMarginX + static_cast<int>(expanderSize) / 2, offset.Y + nodeHeightInt * m_viewport.m_startingVisibleIndex };
				Point endPointV{ startPointV.X, offset.Y + nodeHeightInt * (m_viewport.m_endingVisibleIndex + 1) };

				graphics.DrawLine(startPointV, endPointV, lineWidth, lineColor, lineStyle);

				--currentDepth;
				parentVisible = parentVisible->parent;
			}
		}
	}

	void TreeBoxReactor::Module::Init()
	{
		m_root.isExpanded = true;
	}

	TreeNodeHandle TreeBoxReactor::Module::CleanKey(const TreeNodeHandle& key)
	{
		if (key.empty() || key[key.size() - 1] != '/')
			return key;

		return key.substr(0, key.size() - 1);
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const TreeNodeHandle& key, const std::string& text)
	{
		auto cleanKey = CleanKey(key);
		if (cleanKey.empty())
			return {};

		auto parts = StringUtils::Split(key, '/');
		TreeNodeType* current = &m_root;
		for (const auto& part : parts)
		{
			current = current->Add(part, text, current);
		}

		return { current, this };
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const TreeNodeHandle& key, const std::string& text, const TreeNodeHandle& parentHandle)
	{
		return {};
	}

	TreeBoxItem TreeBoxReactor::Module::Find(const TreeNodeHandle& handle)
	{
		if (handle.empty())
		{
			return {};
		}

		auto parts = StringUtils::Split(handle, '/');
		auto current = &m_root;
		for (auto& part : parts)
		{
			current = current->Find(part);
			if (!current)
			{
				return {};
			}
		}

		return { current, this };
	}

	TreeNodeHandle TreeBoxReactor::Module::GenerateUniqueHandle(const std::string& key, TreeNodeType* parentNode)
	{
		if (parentNode)
		{
			return !parentNode->key.empty() && parentNode->key.back()=='/' ? 
				parentNode->key + key :
				parentNode->key + "/" + key;
		}
		return key;
	}

	void TreeBoxReactor::Module::Erase(const TreeNodeHandle& handle)
	{
		auto item = Find(handle);
		if (!item)
		{
			return;
		}

		Unlink(item.m_node);
		EraseNode(item.m_node);

		CalculateViewport(m_viewport);
		if (UpdateScrollBars())
		{
			if (m_scrollBarVert)
				m_scrollBarVert->Handle()->Renderer.Update();

			if (m_scrollBarHoriz)
				m_scrollBarHoriz->Handle()->Renderer.Update();
		}
		CalculateVisibleNodes();
		m_mouseSelection.Deselect(item.m_node);

		GUI::UpdateWindow(m_window);
	}

	void TreeBoxReactor::Module::Erase(TreeBoxItem item)
	{
		Unlink(item.m_node);
		EraseNode(item.m_node);

		CalculateViewport(m_viewport);
		if (UpdateScrollBars())
		{
			if (m_scrollBarVert)
				m_scrollBarVert->Handle()->Renderer.Update();

			if (m_scrollBarHoriz)
				m_scrollBarHoriz->Handle()->Renderer.Update();
		}
		CalculateVisibleNodes();
		m_mouseSelection.Deselect(item.m_node);

		GUI::UpdateWindow(m_window);
	}

	void TreeBoxReactor::Module::EraseNode(TreeNodeType* node)
	{
		if (node == nullptr)
			return;

		auto current = node->firstChild;
		while (current)
		{
			auto temp = current->nextSibling;
			EraseNode(current);

			current = temp;
		}
		node->m_lookup.clear();
		
		auto eraseResult = node->parent->m_lookup.erase(node->key);
	}

	void TreeBoxReactor::Module::Unlink(TreeNodeType* node)
	{
		auto current = node->parent->firstChild;
		while (current)
		{
			if (current == node)
			{
				auto prevSibling = current->prevSibling;
				if (current->nextSibling)
				{
					current->nextSibling->prevSibling = prevSibling;
				}
				if (current->prevSibling == nullptr)
				{
					node->parent->firstChild = current->nextSibling;
				}
				else
				{

					current->prevSibling->nextSibling = current->nextSibling;
				}
				break;
			}

			current = current->nextSibling;
		}
	}

	bool TreeBoxReactor::Module::UpdateScrollBars()
	{
		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		bool needUpdate = false;
		
		if (m_viewport.m_needVerticalScroll)
		{
			Rectangle scrollRect{ static_cast<int>(m_window->ClientSize.Width - scrollSize) - 1, 1, scrollSize, m_window->ClientSize.Height - 2u };
			if (m_viewport.m_needHorizontalScroll)
			{
				scrollRect.Height -= scrollSize;
			}

			if (!m_scrollBarVert)
			{
				m_scrollBarVert = std::make_unique<ScrollBar>(m_window, false, scrollRect);
				m_scrollBarVert->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_scrollOffset.Y = args.Value;
						CalculateVisibleNodes();

						GUI::UpdateWindow(m_window);
					});
			}
			else
			{
				GUI::MoveWindow(m_scrollBarVert->Handle(), scrollRect);
			}

			auto nodeItemHeight = m_window->ToScale(m_appearance->TreeItemHeight);
			m_scrollBarVert->SetMinMax(0, (int)(m_viewport.m_contentSize.Height - m_viewport.m_backgroundRect.Height));
			m_scrollBarVert->SetPageStepValue(m_viewport.m_backgroundRect.Height);
			m_scrollBarVert->SetStepValue(nodeItemHeight);

			m_scrollOffset.Y = m_scrollBarVert->GetValue();
			CalculateVisibleNodes();

			needUpdate = true;
		}
		else if (m_scrollBarVert)
		{
			m_scrollBarVert.reset();
			m_scrollOffset.Y = 0;
			CalculateVisibleNodes();

			needUpdate = true;
		}
		return needUpdate;
	}

	void TreeBoxReactor::Module::ClearSelection()
	{
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			m_mouseSelection.m_selections[i]->isSelected = false;
		}
		m_mouseSelection.m_selections.clear();
	}

	bool TreeBoxReactor::Module::ClearSingleSelection()
	{
		if (m_mouseSelection.m_selectedNode != nullptr)
		{
			m_mouseSelection.m_selectedNode->isSelected = false;
			m_mouseSelection.m_selections.clear();
			m_mouseSelection.m_selectedNode = nullptr;
			return true;
		}
		return false;
	}

	void TreeBoxReactor::Module::SelectItem(TreeNodeType* node)
	{
		node->isSelected = true;
		m_mouseSelection.m_selections.push_back(node);
		m_mouseSelection.m_selectedNode = node;
	}

	bool TreeBoxReactor::Module::HandleMultiSelection(TreeNodeType* node)
	{
		auto savedSelectedNode = m_mouseSelection.m_selectedNode;
		auto savedPivotNode = m_mouseSelection.m_pivotNode;
		if (!m_mouseSelection.m_pivotNode || (!m_ctrlPressed && !m_shiftPressed))
		{
			m_mouseSelection.m_pivotNode = node;
		}
		m_mouseSelection.m_selectedNode = node;

		if (m_shiftPressed)
		{
			for (auto& oldNode : m_mouseSelection.m_selections)
			{
				oldNode->isSelected = false;
			}
			m_mouseSelection.m_selections.clear();

			auto target1 = m_mouseSelection.m_pivotNode;
			auto target2 = node;

			std::stack<TreeNodeType*> stack;
			stack.push(m_root.firstChild);

			while (!stack.empty())
			{
				TreeNodeType* current = stack.top();
				stack.pop();

				if (target1 == current)
				{
					if (!current->isSelected)
					{
						current->isSelected = true;
						m_mouseSelection.Select(current);
					}
					target1 = nullptr;
				}
				else if (target2 == current)
				{
					if (!current->isSelected)
					{
						current->isSelected = true;
						m_mouseSelection.Select(current);
					}
					target2 = nullptr;
				}
				else if (target1 == nullptr || target2 == nullptr)
				{
					current->isSelected = true;
					m_mouseSelection.Select(current);
				}

				if (target1 == nullptr && target2 == nullptr)
					break;

				if (current->nextSibling)
				{
					stack.push(current->nextSibling);
				}

				if (current->isExpanded && current->firstChild)
				{
					stack.push(current->firstChild);
				}
			}

			for (auto& selNode : m_mouseSelection.m_selections)
			{
				BT_CORE_TRACE << "  - " << selNode->text << std::endl;
			}

			return savedPivotNode != m_mouseSelection.m_pivotNode || savedSelectedNode != m_mouseSelection.m_selectedNode;
		}

		if (m_ctrlPressed)
		{
			node->isSelected = !node->isSelected;
			if (node->isSelected)
			{
				m_mouseSelection.Select(node);
			}
			else
			{
				m_mouseSelection.Deselect(node);
			}

			return true;
		}

		auto savedSelectionCount = m_mouseSelection.m_selections.size();
		for (auto& selectedNode : m_mouseSelection.m_selections)
		{
			selectedNode->isSelected = false;
		}
		m_mouseSelection.m_selections.clear();
		node->isSelected = true;
		m_mouseSelection.Select(node);

		return savedSelectionCount != m_mouseSelection.m_selections.size() || savedSelectedNode != m_mouseSelection.m_selectedNode;
	}

	bool TreeBoxReactor::Module::UpdateSingleSelection(TreeNodeType* node)
	{
		bool needUpdate = m_mouseSelection.m_selectedNode != node || !node->isSelected;
		if (needUpdate)
		{
			ClearSingleSelection();
			SelectItem(node);
			m_mouseSelection.m_pivotNode = node;
		}
		return needUpdate;
	}

	bool TreeBoxReactor::Module::IsVisibleNode(TreeNodeType* node) const
	{
		return std::find(m_visibleNodes.begin(), m_visibleNodes.end(), node) != m_visibleNodes.end();
	}

	bool TreeBoxReactor::Module::IsVisibleNode(TreeNodeType* node, int& visibleIndex) const
	{
		auto it = std::find(m_visibleNodes.begin(), m_visibleNodes.end(), node);
		if (it != m_visibleNodes.end())
		{
			visibleIndex = static_cast<int>(it - m_visibleNodes.begin());
			return true;
		}
		return false;
	}

	bool TreeBoxReactor::Module::IsAnySiblingVisible(TreeNodeType* node) const
	{
		auto current = node;
		if (!current)
		{
			return false;
		}

		while (current)
		{
			if (IsVisibleNode(current))
				return true;

			current = current->nextSibling;
		}
		return false;
	}

	void TreeBoxReactor::Module::EmitSelectionEvent()
	{
		ArgTreeBoxSelection argTreeBox;
		argTreeBox.Items.resize(m_mouseSelection.m_selections.size());
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			argTreeBox.Items[i] = { m_mouseSelection.m_selections[i], this };
		}
		reinterpret_cast<TreeBoxEvents*>(m_window->Events.get())->Selected.Emit(argTreeBox);
	}

	void TreeBoxReactor::Module::EmitExpansionEvent(TreeNodeType* node)
	{
		auto item = TreeBoxItem{ node, this };
		ArgTreeBox argTreeBox(item, node->isExpanded);
		reinterpret_cast<TreeBoxEvents*>(m_window->Events.get())->Expanded.Emit(argTreeBox);
	}

	bool TreeBoxReactor::Module::Collapse(TreeBoxItem item)
	{
		if (!item || !item.m_node->firstChild)
		{
			return false;
		}
		bool needUpdate = item.m_node->isExpanded;
		item.m_node->isExpanded = false;

		return needUpdate;
	}

	bool TreeBoxReactor::Module::CollapseAll()
	{
		return CollapseAll({ &m_root, this });
	}

	bool TreeBoxReactor::Module::CollapseAll(TreeBoxItem item)
	{
		if (!item || !item.m_node->firstChild)
		{
			return false;
		}
		bool needUpdate = item.m_node != &m_root && item.m_node->isExpanded;
		if (item.m_node != &m_root)
			item.m_node->isExpanded = false;

		auto current = item.m_node->firstChild;

		while (current)
		{
			needUpdate |= CollapseAll({ current, this });
			current = current->nextSibling;
		}
		return needUpdate;
	}

	bool TreeBoxReactor::Module::ExpandAll()
	{
		return ExpandAll({ &m_root, this });
	}

	bool TreeBoxReactor::Module::ExpandAll(TreeBoxItem item)
	{
		if (!item || !item.m_node->firstChild)
		{
			return false;
		}
		bool needUpdate = item.m_node != &m_root && !item.m_node->isExpanded;
		if (item.m_node != &m_root) 
			item.m_node->isExpanded = true;
		
		auto current = item.m_node->firstChild;

		while (current)
		{
			needUpdate |= ExpandAll({ current, this });
			current = current->nextSibling;
		}
		return needUpdate;
	}

	bool TreeBoxReactor::Module::Expand(TreeBoxItem item)
	{
		if (!item || !item.m_node->firstChild)
		{
			return false;
		}
		bool needUpdate = !item.m_node->isExpanded;
		item.m_node->isExpanded = true;
		
		return needUpdate;
	}

	void TreeBoxReactor::Module::SetIcon(TreeNodeType* node, const Image& icon)
	{
		node->icon = icon;
		bool needUpdate = IsVisibleNode(node);
		m_drawImages = true;

		if (needUpdate)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	void TreeBoxReactor::Module::SetText(TreeNodeType* node, const std::string& newText) const
	{
		if (node->text == newText)
			return;

		node->text = newText;
		bool needUpdate = IsVisibleNode(node);

		if (needUpdate)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	void TreeBoxItem::Collapse()
	{
		if (m_module->Collapse(*this))
		{
			m_module->Update();
			m_module->Draw();
		}
	}

	void TreeBoxItem::Expand()
	{
		if (m_module->Expand(*this))
		{
			m_module->Update();
			m_module->Draw();
		}
	}

	void TreeBoxItem::Select()
	{
		bool needUpdate = false;
		bool emitSelectionEvent = false;
		if (m_module->m_multiselection)
		{
			if (m_module->HandleMultiSelection(m_node))
			{
				needUpdate = true;
				emitSelectionEvent = true;
			}
		}
		else
		{
			if (m_module->UpdateSingleSelection(m_node))
			{
				needUpdate = true;
				emitSelectionEvent = true;
			}
		}
		if (needUpdate)
		{
			m_module->Update();
			m_module->Draw();
		}
		if (emitSelectionEvent)
		{
			m_module->EmitSelectionEvent();
		}
	}

	TreeNodeType* TreeBoxReactor::Module::GetRoot()
	{
		return &m_root;
	}

	std::string TreeBoxReactor::Module::GetKeyPath(TreeBoxItem item, char separator)
	{
		std::string path;
		std::string temp;

		auto current = item.m_node;
		auto root = GetRoot();
		while (current->parent != root)
		{
			temp = separator;
			temp += current->key;
			path.insert(0, temp);

			current = current->parent;
		}
		path.insert(0, current->key);
		return path;
	}

	std::vector<TreeBoxItem> TreeBoxReactor::Module::GetSelected()
	{
		std::vector<TreeBoxItem> selections;
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			selections.emplace_back(TreeBoxItem{ m_mouseSelection.m_selections[i], this });
		}
		return selections;
	}

	bool TreeBoxReactor::Module::ShowNavigationLines(bool visible)
	{
		if (m_showNavigationLines == visible)
			return false;

		m_showNavigationLines = visible;
		return true;
	}

	TreeBox::TreeBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "TreeBox";
#endif
	}

	void TreeBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	void TreeBox::CollapseAll()
	{
		if (m_reactor.GetModule().CollapseAll())
		{
			m_reactor.GetModule().Update();
			m_reactor.GetModule().Draw();
		}
	}

	void TreeBox::CollapseAll(TreeBoxItem item)
	{
		if (m_reactor.GetModule().CollapseAll(item))
		{
			m_reactor.GetModule().Update();
			m_reactor.GetModule().Draw();
		}
	}

	void TreeBox::Erase(const TreeNodeHandle& key)
	{
		m_reactor.GetModule().Erase(key);
	}

	void TreeBox::Erase(TreeBoxItem item)
	{
		m_reactor.GetModule().Erase(item);
	}

	TreeBoxItem TreeBox::Find(const TreeNodeHandle& key)
	{
		return m_reactor.GetModule().Find(key);
	}

	TreeBoxItem TreeBox::Insert(const TreeNodeHandle& key, const std::string& text)
	{
		auto item = m_reactor.GetModule().Insert(key, text);

		if (item)
		{
			m_reactor.GetModule().Update();
			m_reactor.GetModule().Draw();
		}
		return item;
	}

	TreeBoxItem TreeBox::Insert(TreeBoxItem parent, const TreeNodeHandle& key, const std::string& text)
	{
		auto item = m_reactor.GetModule().Insert(key, text, parent.GetHandle());

		if (item)
		{
			m_reactor.GetModule().Update();
			m_reactor.GetModule().Draw();
		}
		return item;
	}

	void TreeBox::ExpandAll()
	{
		if (m_reactor.GetModule().ExpandAll())
		{
			m_reactor.GetModule().Update();
			m_reactor.GetModule().Draw();
		}
	}

	void TreeBox::ExpandAll(TreeBoxItem item)
	{
		if (m_reactor.GetModule().ExpandAll(item))
		{
			m_reactor.GetModule().Update();
			m_reactor.GetModule().Draw();
		}
	}

	std::string TreeBox::GetKeyPath(TreeBoxItem item, char separator)
	{
		return m_reactor.GetModule().GetKeyPath(item, separator);
	}

	std::vector<TreeBoxItem> TreeBox::GetSelected()
	{
		return m_reactor.GetModule().GetSelected();
	}

	void TreeBox::ShowNavigationLines(bool visible)
	{
		if (m_reactor.GetModule().ShowNavigationLines(visible))
		{
			m_reactor.GetModule().Update();
			m_reactor.GetModule().Draw();
		}
	}

	TreeNodeType* TreeNodeType::Add(const TreeNodeHandle& childKey, const std::string& text_, TreeNodeType* parent_)
	{
		auto it = m_lookup.find(childKey);
		if (it != m_lookup.end())
		{
			return it->second.get();
		}

		auto child = std::make_unique<TreeNodeType>(childKey, text_, parent_);
		TreeNodeType* childPtr = child.get();
		m_lookup[childKey] = std::move(child);

		if (firstChild)
		{
			auto lastNode = firstChild;
			while (lastNode->nextSibling != nullptr)
			{
				lastNode = lastNode->nextSibling;
			}
			lastNode->nextSibling = childPtr;
			childPtr->prevSibling = lastNode;
		}
		else
		{
			firstChild = childPtr;
		}

		return childPtr;
	}

	TreeNodeType* TreeNodeType::Find(const TreeNodeHandle& key)
	{
		auto it = m_lookup.find(key);
		return it != m_lookup.end() ? it->second.get() : nullptr;
	}
}
