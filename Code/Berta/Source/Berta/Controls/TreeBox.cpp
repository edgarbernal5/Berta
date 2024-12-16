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

		m_module.Init();
		m_module.CalculateViewport(m_module.m_viewport);
	}

	void TreeBoxReactor::Update(Graphics& graphics)
	{
		BT_CORE_TRACE << " -- TreeBox Update() " << std::endl;
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();

		auto nodeHeight = window->ToScale(window->Appearance->ComboBoxItemHeight);
		auto nodeTextMargin = window->ToScale(8u);
		auto nodeHeightInt = static_cast<int>(nodeHeight);
		auto nodeHeightHalfInt = nodeHeightInt >> 1;

		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->BoxBackground, true);

		Point offset{ m_module.m_viewport.m_backgroundRect.X - m_module.m_scrollOffset.X, m_module.m_viewport.m_backgroundRect.Y - m_module.m_scrollOffset.Y };

		auto iconSize = m_module.m_window->ToScale(m_module.m_window->Appearance->SmallIconSize);
		
		struct NodeAndDepth
		{
			TreeNodeType* node{ nullptr };
			uint32_t depth{ 0 };
		};
		std::stack<NodeAndDepth> navigationLines;

		TreeNodeType* currentParent{ nullptr };
		int i = m_module.m_viewport.m_startingVisibleIndex;
		for (auto& node : m_module.m_visibleNodes)
		{
			auto depth = m_module.CalculateNodeDepth(node);
			if (currentParent != node->parent)
			{
				navigationLines.push(NodeAndDepth{ node , depth});
			}
			Rectangle nodeRect{ offset.X, offset.Y + nodeHeightInt * i, m_module.m_viewport.m_contentSize.Width, nodeHeight };

			bool isSelected = node->isSelected;
			bool isHovered = node == m_module.m_mouseSelection.m_hoveredIndex;
			int nodeOffset = (depth - 1) * iconSize;
			if (isSelected)
			{
				//
				graphics.DrawRectangle(nodeRect, window->Appearance->HighlightColor, true);
				graphics.DrawRectangle(nodeRect, window->Appearance->BoxBorderHighlightColor, false);
			}
			else if (isHovered)
			{
				graphics.DrawRectangle(nodeRect, window->Appearance->ItemCollectionHightlightBackground, true);
			}

			Rectangle expanderRect{ nodeRect.X + nodeOffset, nodeRect.Y, iconSize, nodeHeight };
			
			nodeOffset += iconSize;
			if (m_module.m_drawImages)
			{
				if (node->icon)
				{
					auto& icon = node->icon;
					auto iconSourceSize = icon.GetSize();
					auto positionY = (nodeHeight - iconSize) >> 1;
					icon.Paste(graphics, { nodeRect.X + nodeOffset, (int)positionY, iconSize , iconSize });
				}

				nodeOffset += iconSize;
			}

			Point startLine{ static_cast<int>(expanderRect.X * 2 + expanderRect.Width) / 2, static_cast<int>(expanderRect.Y * 2 + expanderRect.Height) / 2 };
			if (node->firstChild && m_module.IsAnyChildrenVisible(node))
			{
				graphics.DrawLine(startLine, { startLine.X + (int)(expanderRect.Width / 2 + nodeTextMargin), startLine.Y}, m_module.m_window->Appearance->Foreground2nd, Graphics::LineStyle::Solid);
			}

			Point vStartPos{ expanderRect.X + (int)(expanderRect.Width / 2), nodeRect.Y};
			Point vEndPos{ vStartPos.X, vStartPos.Y + nodeHeightInt};

			if (i == 0)
			{
				vStartPos.Y += nodeHeightHalfInt;
			}

			if (node->nextSibling != nullptr && node->firstChild && node->isExpanded)
			{
				int nextSiblingIndex = -1;
				if (m_module.IsVisibleNode(node->nextSibling, nextSiblingIndex))
				{
					vEndPos.Y = nodeHeightInt * (nextSiblingIndex + m_module.m_viewport.m_startingVisibleIndex) + offset.Y;
				}
				else {
					vEndPos.Y = m_module.m_viewport.m_backgroundRect.Height;
				}
			}
			else if (node->nextSibling == nullptr && node->prevSibling)
			{
				vEndPos.Y = vStartPos.Y + nodeHeightHalfInt;
			}

			if (depth > 0)
			{
				auto parent = node->parent;
				while (parent && m_module.IsVisibleNode(parent))
				{
					parent = parent->parent;
				}
				if (parent && parent != &m_module.m_root)
				{
					auto parentDepth = m_module.CalculateNodeDepth(parent);
					while (parentDepth > 0)
					{
						int nodeOffset = (parentDepth - 1) * iconSize;
						int nextSiblingIndex = -1;
						if (m_module.IsVisibleNode(parent->nextSibling, nextSiblingIndex))
						{
							Point vStartPos{ offset.X + nodeOffset + (int)(expanderRect.Width / 2), 0 };
							Point vEndPos{ vStartPos.X, 1+offset.Y + nodeHeightInt * (nextSiblingIndex + m_module.m_viewport.m_startingVisibleIndex) };
							graphics.DrawLine(vStartPos, vEndPos, m_module.m_window->Appearance->Foreground2nd, Graphics::LineStyle::Solid);
						}
						else if (parent->nextSibling)
						{
							Point vStartPos{ offset.X + nodeOffset + (int)(expanderRect.Width / 2), 0 };
							Point vEndPos{ vStartPos.X, (int)m_module.m_viewport.m_backgroundRect.Height };
							graphics.DrawLine(vStartPos, vEndPos, m_module.m_window->Appearance->Foreground2nd, Graphics::LineStyle::Solid);
						}
						parent = parent->parent;
						--parentDepth;
					}
				}
			}

			graphics.DrawLine(vStartPos, vEndPos, m_module.m_window->Appearance->Foreground2nd, Graphics::LineStyle::Solid);
			{
				Point hStartPos{ vStartPos.X, nodeRect.Y + nodeHeightHalfInt };
				Point hEndPos{ hStartPos.X + (int)iconSize, hStartPos.Y};
				graphics.DrawLine(hStartPos, hEndPos, m_module.m_window->Appearance->Foreground2nd, Graphics::LineStyle::Solid);
			}

			if (node->firstChild)
			{
				int arrowWidth = window->ToScale(4);
				int arrowLength = window->ToScale(2);
				graphics.DrawArrow(expanderRect,
					arrowLength,
					arrowWidth,
					node->isExpanded ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right,
					window->Appearance->Foreground2nd,
					true,
					node->isExpanded ? window->Appearance->Foreground2nd : window->Appearance->BoxBackground
				);
			}

			graphics.DrawString({ nodeRect.X + nodeOffset + (int)nodeTextMargin, nodeRect.Y + (int)(nodeHeight - graphics.GetTextExtent().Height) / 2 }, node->text, m_module.m_window->Appearance->Foreground);
			++i;

			currentParent = node->parent;
		}

		while (!navigationLines.empty())
		{
			auto& top = navigationLines.top();
			navigationLines.pop();
		}

		if (m_module.m_viewport.m_needHorizontalScroll && m_module.m_viewport.m_needVerticalScroll)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			graphics.DrawRectangle({ (int)(m_module.m_window->Size.Width - scrollSize) - 1, (int)(m_module.m_window->Size.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background, true);
		}
		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void TreeBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.CalculateViewport(m_module.m_viewport);

		m_module.UpdateScrollBars();
		m_module.CalculateVisibleNodes();

		if (m_module.m_scrollBarVert)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			GUI::MoveWindow(m_module.m_scrollBarVert->Handle(), scrollRect);
		}
	}

	void TreeBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		bool needUpdate = m_module.m_mouseSelection.m_hoveredIndex != nullptr;
		m_module.m_mouseSelection.m_hoveredIndex = nullptr;

		m_module.m_hoveredArea = InteractionArea::None;
		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_window);
		}
	}

	void TreeBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_pressedArea = m_module.m_hoveredArea;
		bool needUpdate = false;
		bool emitEvent = false;

		if (args.ButtonState.LeftButton)
		{
			if (m_module.m_hoveredArea == InteractionArea::Expander)
			{
				auto nodeHeight = m_module.m_window->ToScale(m_module.m_window->Appearance->ComboBoxItemHeight);
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
			}
		}

		if (m_module.m_pressedArea == InteractionArea::Node)
		{
			m_module.m_mouseSelection.m_pressedIndex = m_module.m_mouseSelection.m_hoveredIndex;

			if (m_module.m_multiselection)
			{
				//needUpdate = m_module.HandleMultiSelection(m_module.m_mouseSelection.m_hoveredIndex, args);
			}
			else
			{
				if (m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_pressedIndex))
				{
					needUpdate = true;
					emitEvent = true;
				}
			}
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_window);
		}

		if (emitEvent)
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
			auto nodeHeight = m_module.m_window->ToScale(m_module.m_window->Appearance->ComboBoxItemHeight);
			auto nodeHeightInt = static_cast<int>(nodeHeight);

			auto positionY = args.Position.Y - m_module.m_viewport.m_backgroundRect.Y + m_module.m_scrollOffset.Y;
			int index = positionY / nodeHeightInt;
			index -= m_module.m_viewport.m_startingVisibleIndex;

			needUpdate = m_module.m_visibleNodes[index] != m_module.m_mouseSelection.m_hoveredIndex;
			m_module.m_mouseSelection.m_hoveredIndex = m_module.m_visibleNodes[index];
		}
		else if (hoveredArea == InteractionArea::Blank)
		{
			needUpdate = m_module.m_mouseSelection.m_hoveredIndex != nullptr;
			m_module.m_mouseSelection.m_hoveredIndex = nullptr;
		}

		m_module.m_hoveredArea = hoveredArea;

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_window);
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

				m_module.m_scrollBarVert->Handle()->Renderer.Update();
				GUI::MarkAsUpdated(m_module.m_scrollBarVert->Handle());
			}
			else
			{
				m_module.m_scrollOffset.X = newOffset;
				m_module.m_scrollBarHoriz->SetValue(newOffset);

				m_module.m_scrollBarHoriz->Handle()->Renderer.Update();
				GUI::MarkAsUpdated(m_module.m_scrollBarHoriz->Handle());
			}

			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
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
		viewportData.m_backgroundRect = m_window->Size.ToRectangle();
		viewportData.m_backgroundRect.X = viewportData.m_backgroundRect.Y = 1;
		viewportData.m_backgroundRect.Width -= 2u;
		viewportData.m_backgroundRect.Height -= 2u;

		auto nodeHeight = m_window->ToScale(m_window->Appearance->ComboBoxItemHeight);
		auto treeSize = CalculateTreeSize(&m_root) - 1;
		viewportData.m_contentSize.Height = nodeHeight * treeSize;

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

		auto nodeHeight = m_window->ToScale(m_window->Appearance->ComboBoxItemHeight);
		auto visibleHeight = m_viewport.m_backgroundRect.Height;
		
		m_viewport.m_startingVisibleIndex = m_scrollOffset.Y / nodeHeight;
		m_viewport.m_endingVisibleIndex = (m_scrollOffset.Y + visibleHeight) / nodeHeight;

		int index = 0;

		std::stack<TreeNodeType*> stack;
		stack.push(m_root.firstChild);

		while (!stack.empty())
		{
			TreeNodeType* node = stack.top();
			stack.pop();

			if (index >= m_viewport.m_startingVisibleIndex && index <= m_viewport.m_endingVisibleIndex)
			{
				m_visibleNodes.emplace_back(node);
			}

			if (index > m_viewport.m_endingVisibleIndex)
				break;

			++index;

			if (node->nextSibling)
			{
				stack.push(node->nextSibling);
			}

			if (node->isExpanded && node->firstChild)
			{
				stack.push(node->firstChild);
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
		m_nodeLookup.clear();

		CalculateViewport(m_viewport);
		UpdateScrollBars();

		if (needUpdate)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	Berta::TreeBoxReactor::TreeNodeType* TreeBoxReactor::Module::GetNextVisible(TreeNodeType* node)
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


	TreeBoxReactor::InteractionArea TreeBoxReactor::Module::DetermineHoverArea(const Point& mousePosition)
	{
		if (!m_window->Size.IsInside(mousePosition))
		{
			return InteractionArea::None;
		}

		auto nodeHeight = m_window->ToScale(m_window->Appearance->ComboBoxItemHeight);
		auto nodeHeightInt = static_cast<int>(nodeHeight);

		auto positionY = mousePosition.Y - m_viewport.m_backgroundRect.Y + m_scrollOffset.Y;
		int index = positionY / nodeHeightInt;

		index -= m_viewport.m_startingVisibleIndex;
		if (index >= m_visibleNodes.size())
		{
			return InteractionArea::Blank;
		}

		auto iconSize = m_window->ToScale(m_window->Appearance->SmallIconSize);
		auto nodeDepth = CalculateNodeDepth(m_visibleNodes[index]);
		int nodeOffset = (nodeDepth - 1) * iconSize;
		
		Rectangle expanderRect{ m_viewport.m_backgroundRect.X + m_scrollOffset.X + nodeOffset, index * nodeHeightInt, iconSize, nodeHeight };
		if (m_visibleNodes[index]->firstChild && expanderRect.IsInside(mousePosition))
		{
			return InteractionArea::Expander;
		}

		return InteractionArea::Node;
	}

	void TreeBoxReactor::Module::Init()
	{
		m_root.isExpanded = true;
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text)
	{
		auto hasParentIndex = key.find_last_of('/');
		if (hasParentIndex != std::string::npos)
		{
			return Insert(key.substr(hasParentIndex + 1, key.size()), text, key.substr(0, hasParentIndex));
		}

		auto node = std::make_unique<TreeNodeType>(key, text, &m_root);
		TreeNodeType* nodePtr = node.get();

		if (m_root.firstChild)
		{
			auto lastNode = m_root.firstChild;
			while (lastNode->nextSibling != nullptr)
			{
				lastNode = lastNode->nextSibling;
			}
			lastNode->nextSibling = nodePtr;
			nodePtr->prevSibling = lastNode;
		}
		else
		{
			m_root.firstChild = nodePtr;
		}
		
		m_nodeLookup[key] = std::move(node);
		return { nodePtr, this };
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text, const TreeNodeHandle& parentHandle)
	{
		TreeNodeType* parentNode{ nullptr };
		if (!parentHandle.empty())
		{
			auto it = m_nodeLookup.find(parentHandle);
			if (it == m_nodeLookup.end())
			{
				return {};
			}
			parentNode = it->second.get();
		}

		std::string handle = GenerateUniqueHandle(key, parentNode);
		auto node = std::make_unique<TreeNodeType>(handle, text, parentNode);
		node->parent = parentNode;
		TreeNodeType* nodePtr = node.get();

		if (parentNode)
		{
			if (parentNode->firstChild == nullptr)
			{
				parentNode->firstChild = nodePtr;
			}
			else
			{
				auto lastNode = parentNode->firstChild;
				while (lastNode->nextSibling != nullptr)
				{
					lastNode = lastNode->nextSibling;
				}
				lastNode->nextSibling = nodePtr;
				nodePtr->prevSibling = lastNode;
			}
		}
		else
		{
			if (m_root.firstChild == nullptr)
			{
				m_root.firstChild = nodePtr;
			}
			else
			{
				auto lastNode = m_root.firstChild;
				while (lastNode->nextSibling != nullptr)
				{
					lastNode = lastNode->nextSibling;
				}
				m_root.nextSibling = nodePtr;
				nodePtr->prevSibling = lastNode;
			}
		}

		m_nodeLookup[handle] = std::move(node);
		return { nodePtr, this };
	}

	TreeBoxItem TreeBoxReactor::Module::Find(const TreeNodeHandle& handle)
	{
		if (handle.empty())
		{
			return {};
		}

		auto it = m_nodeLookup.find(handle);
		if (it == m_nodeLookup.end())
		{
			return {};
		}

		return { it->second.get(), this };
	}

	TreeNodeHandle TreeBoxReactor::Module::GenerateUniqueHandle(const std::string& key, TreeNodeType* parentNode)
	{
		if (parentNode)
		{
			return parentNode->key + "/" + key;
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

		auto rr=m_nodeLookup.erase(node->key);
	}

	void TreeBoxReactor::Module::Unlink(TreeNodeType* node)
	{
		auto current = node->parent->firstChild;
		while (current)
		{
			if (current == node)
			{
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
			Rectangle scrollRect{ static_cast<int>(m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_window->Size.Height - 2u };
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

			auto nodeItemHeight = m_window->ToScale(m_window->Appearance->ComboBoxItemHeight);
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

	bool TreeBoxReactor::Module::ClearSingleSelection()
	{
		if (m_mouseSelection.m_selectedIndex != nullptr)
		{
			m_mouseSelection.m_selectedIndex->isSelected = false;
			m_mouseSelection.m_selections.clear();
			m_mouseSelection.m_selectedIndex = nullptr;
			return true;
		}
		return false;
	}

	void TreeBoxReactor::Module::SelectItem(TreeNodeType* node)
	{
		node->isSelected = true;
		m_mouseSelection.m_selections.push_back(node);
		m_mouseSelection.m_selectedIndex = node;
	}

	bool TreeBoxReactor::Module::UpdateSingleSelection(TreeNodeType* node)
	{
		bool needUpdate = m_mouseSelection.m_selectedIndex != node;
		if (needUpdate)
		{
			ClearSingleSelection();
			SelectItem(node);
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
			visibleIndex = it - m_visibleNodes.begin();
			return true;
		}
		return false;
	}

	bool TreeBoxReactor::Module::IsAnyChildrenVisible(TreeNodeType* parentNode) const
	{
		auto current = parentNode->firstChild;
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
		ArgTreeBox argTreeBox;
		argTreeBox.Items.resize(m_mouseSelection.m_selections.size());
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			argTreeBox.Items[i] = { m_mouseSelection.m_selections[i], this };
		}
		reinterpret_cast<TreeBoxEvents*>(m_window->Events.get())->Selected.Emit(argTreeBox);
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

	void TreeBox::Erase(const std::string& key)
	{
		m_reactor.GetModule().Erase(key);
	}

	void TreeBox::Erase(TreeBoxItem item)
	{
		m_reactor.GetModule().Erase(item);
	}

	TreeBoxItem TreeBox::Insert(const std::string& key, const std::string& text)
	{
		return m_reactor.GetModule().Insert(key, text);
	}

	std::vector<TreeBoxItem> TreeBox::GetSelected()
	{
		return m_reactor.GetModule().GetSelected();
	}


}
